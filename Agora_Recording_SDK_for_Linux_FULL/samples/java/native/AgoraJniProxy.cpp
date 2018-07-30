#include "IAgoraLinuxSdkCommon.h"
#include "./jni/AgoraJavaRecording.h"
#include "AgoraJniProxy.h"
#include <pthread.h>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include "helper.h"

using std::string;
using std::cout;
using std::cerr;
using std::endl;

using agora::base::log;
using namespace jniproxy;

atomic_bool_t g_bSignalStop;

void signal_handler(int signo) {
  (void)signo;
  cerr << "Signal " << signo<<endl;
  g_bSignalStop = true;
}
jmethodID AgoraJniProxySdk::mJavaRecvAudioMtd = NULL;
jmethodID AgoraJniProxySdk::mJavaVideoFrameInitMtd = NULL;
jmethodID AgoraJniProxySdk::mJavaVideoYuvFrameInitMtd = NULL;
jmethodID AgoraJniProxySdk::mJavaVideoH264FrameInitMtd = NULL;
jmethodID AgoraJniProxySdk::mJavaVideoJpgFrameInitMtd = NULL;

jmethodID AgoraJniProxySdk::m_CBObjectMethodIDs[MID_CBOBJECT_NUM];
jfieldID AgoraJniProxySdk::m_VideoYuvFrameFieldIDs[FID_YUVNUM];
jfieldID AgoraJniProxySdk::m_AudioFrameFieldIDs[FID_AF_NUM];

jfieldID AgoraJniProxySdk::m_VideoH264FrameFieldIDs[FID_H264NUM];
jfieldID AgoraJniProxySdk::m_VideoJpgFrameFieldIDs[FID_JPGNUM];  
jfieldID AgoraJniProxySdk::m_AudioPcmFrameFieldIDs[FID_PCMNUM];
jfieldID AgoraJniProxySdk::m_AudioAacFrameFieldIDs[FID_AACNUM];
  


template<typename T1, typename T2>
int AgoraJniProxySdk::staticInitCommonFrameFid(JNIEnv* env, jclass clazz, GETID_TYPE type, T1& src, T2& dest){
  for (int i = 0; i < sizeof(src) / sizeof(src[0]); i++){
    const JavaObjectMethod& m = src[i];
    jfieldID id = safeGetFieldID(env, clazz, m.name, m.signature);
    if(!id){
      cout<<"staticInitCommonFrameFid failed,name:"<<m.name<<",m.signature:"<<m.signature<<endl;
      continue;
    }
    dest[m.id] = id;
  }
  return 0;
}

#ifdef __cplusplus
extern "C" {
#endif

JavaVM* g_jvm = NULL;

class AttachThreadScoped
{
public:
  explicit AttachThreadScoped(JavaVM* jvm)
    : attached_(false), jvm_(jvm), env_(NULL) {
    jint ret_val = jvm->GetEnv(reinterpret_cast<void**>(&env_),JNI_VERSION_1_4);
    if (ret_val == JNI_EDETACHED) {
      // Attach the thread to the Java VM.
      ret_val = jvm_->AttachCurrentThread((void**)&env_, NULL);
      attached_ = ret_val >= 0;
      assert(attached_);
    }
  }
  /*~AttachThreadScoped() {
    if (attached_ && (jvm_->DetachCurrentThread() < 0)) {
      assert(false);
    }
  }*/
  void detach(){
    if (!attached_ && jvm_->DetachCurrentThread() < 0) {
      assert(false);
    }
  }
  JNIEnv* env() { return env_; }
private:
  bool attached_;
  JavaVM* jvm_;
  JNIEnv* env_;
};

AgoraJniProxySdk::AgoraJniProxySdk():AgoraSdk(){
  LOG_DIR(m_logdir.c_str(), INFO,"AgoraJniProxySdk constructor");
  mJavaAgoraJavaRecordingClass = NULL;
  mJavaAgoraJavaRecordingObject = NULL;
  mJavaVideoFrameClass = NULL;
  mJavaVideoFrameObject = NULL;
  mJavaVideoYuvFrameClass = NULL;
  mJavaVideoYuvFrameObject = NULL;
  mJavaVideoJpgFrameClass = NULL;
  mJavaVideoJpgFrameObject = NULL;
  mJavaVideoH264FrameClass = NULL;
  mJavaVideoH264FrameObject = NULL;
  //audio
  mJavaAudioFrameClass = NULL;
  mJavaAudioFrameObject = NULL;
  mJavaAudioAacFrameClass = NULL;
  mJavaAudioAacFrameObject = NULL;
  mJavaAudioPcmFrameClass =NULL;
  mJavaAudioPcmFrameObject =NULL;

  mJavaAudioFrameInitMtd = NULL;
  mJavaAudioAacFrameInitMtd = NULL;
  mJavaAudioPcmFrameInitMtd = NULL;

  mJavaVideoFrameTypeClass = NULL;
  mJavaVideoFrameTypeObject = NULL;
  mJavaAudioFrameInitMtd = NULL;

}
AgoraJniProxySdk::~AgoraJniProxySdk(){
  AttachThreadScoped ats(g_jvm);
  JNIEnv* env = ats.env();
  if(!env) return;
  cout<<"AgoraJniProxySdk destructor begin"<<endl;
  initJavaObjects(env, false);
  cout<<"AgoraJniProxySdk destructor end"<<endl;
}
extern "C++" {
void AgoraJniProxySdk::initialize(){
  AttachThreadScoped ats(g_jvm);
  JNIEnv* env = ats.env();
  if(!env) return;
  initJavaObjects(env, true);
  cacheJavaCBFuncMethodIDs4Video(env, CN_VIDEO_FRAME);
  cacheJavaVideoFrameInitMethodIDs(env, CN_VIDEO_YUV_FRAME);
  cacheJavaVideoFrameInitMethodIDs(env, CN_VIDEO_H264_FRAME);
  cacheJavaVideoFrameInitMethodIDs(env, CN_VIDEO_JPG_FRAME);
  //cacheAudioPcmFrame(env);
  cacheJavaObject(env);
  staticInitCBFuncMid(env, mJavaAgoraJavaRecordingClass);
  //staticInitVideoYuvFrameFid(env,mJavaVideoYuvFrameClass);
  //video
  staticInitCommonFrameFid(env, mJavaVideoYuvFrameClass, FIDID,jVideoYuvFrameFields, m_VideoYuvFrameFieldIDs);
  staticInitCommonFrameFid(env, mJavaVideoH264FrameClass, FIDID,jVideoH264FrameFields, m_VideoH264FrameFieldIDs);
  staticInitCommonFrameFid(env, mJavaVideoJpgFrameClass, FIDID,jVideoJpgFrameFields, m_VideoJpgFrameFieldIDs);
  //audio
  staticInitCommonFrameFid(env, mJavaAudioFrameClass, FIDID,jAudioFrameFields, m_AudioFrameFieldIDs);
  staticInitCommonFrameFid(env, mJavaAudioPcmFrameClass, FIDID,jAudioPcmFrameFields, m_AudioPcmFrameFieldIDs);
  staticInitCommonFrameFid(env, mJavaAudioAacFrameClass, FIDID, jAudioAacFrameFields, m_AudioAacFrameFieldIDs);
  }
}
int AgoraJniProxySdk::staticInitVideoYuvFrameFid(JNIEnv* env, jclass clazz){
  for (int i = 0; i < sizeof(jVideoYuvFrameFields) / sizeof(jVideoYuvFrameFields[0]); i++){
    const JavaObjectMethod& m = jVideoYuvFrameFields[i];
    jfieldID fid = safeGetFieldID(env, clazz, m.name, m.signature);
    if (!fid){
      cout<<"AgoraJniProxySdk::staticInitCBFuncMid failed get methid:"<<m.name<<endl;
      return -1;
    }
    m_VideoYuvFrameFieldIDs[m.id] = fid;
  }
  return 0;
}

int AgoraJniProxySdk::staticInitCBFuncMid(JNIEnv* env, jclass clazz){
  for (int i = 0; i < sizeof(jCBObjectMethods) / sizeof(jCBObjectMethods[0]); i++){
    const JavaObjectMethod& m = jCBObjectMethods[i];
    jmethodID mid = safeGetMethodID(env, clazz, m.name, m.signature);
    if (!mid){
      cout<<"AgoraJniProxySdk::staticInitCBFuncMid failed get methid:"<<m.name<<endl;
    }
    m_CBObjectMethodIDs[m.id] = mid;
  }
}
void AgoraJniProxySdk::cacheJavaObject(JNIEnv* env){
}
jobject AgoraJniProxySdk::newJObject(JNIEnv* env, jclass jcls, jmethodID jmtd) const{
  if(!jmtd || !jcls){
    LOG_DIR(m_logdir.c_str(), ERROR,"newJObject but jcls or jmethodID not inited!");
    return NULL;
  }
  jobject job = env->NewObject(jcls, jmtd);
  if(!job) {
    LOG_DIR(m_logdir.c_str(), ERROR,"cannot get videoinit methodid");
    return NULL;
  }
  return job;
}

jclass AgoraJniProxySdk::newGlobalJClass(JNIEnv* env, const char* className){
  jclass localRef = env->FindClass(className);
  if(!localRef) {
    LOG_DIR(m_logdir.c_str(), ERROR,"newGlobalJClass cannot find class:%s",className);
    return NULL;
  }
  jclass globalJc = static_cast<jclass>(env->NewGlobalRef(localRef));
  if(!globalJc){
    LOG_DIR(m_logdir.c_str(), ERROR,"newGlobalJClass cound not create global reference!",className);
    return NULL;
  }
  env->DeleteLocalRef(localRef);
  return globalJc;
}
jobject AgoraJniProxySdk::newGlobalJObject2(JNIEnv* env, jclass jc, jmethodID initMid) const{
  if(!jc || !initMid) {
    LOG_DIR(m_logdir.c_str(), ERROR,"newGlobalJObject but jc or initMid is NULL");
    return NULL;
  }
  jobject globalJob = env->NewGlobalRef(env->NewObject(mJavaVideoFrameClass, initMid));
  if(!globalJob){
    LOG_DIR(m_logdir.c_str(), ERROR,"newGlobalJObject new global reference failed ");
    return NULL;
  }
  return globalJob;
}

jobject AgoraJniProxySdk::newGlobalJObject(JNIEnv* env, jclass jc, const char* signature){
  jmethodID initMid = env->GetMethodID(jc, SG_MTD_INIT, signature);
  if(!initMid) {
    LOG_DIR(m_logdir.c_str(), ERROR,"newGlobalJObject cannot get init method for this signature:%s", signature);
    return NULL;
  }
  jobject globalJob = env->NewGlobalRef(env->NewObject(mJavaVideoFrameClass, initMid));
  if(!globalJob){
    LOG_DIR(m_logdir.c_str(), ERROR,"newGlobalJObject new global reference failed ");
    return NULL;
  }
  return globalJob;
}
jmethodID AgoraJniProxySdk::safeGetMethodID(JNIEnv* env, jclass clazz, const char* name, const char* sig) const {
  CPN(clazz);
  jmethodID mid = env->GetMethodID(clazz, name, sig);
  return mid;
}
jfieldID AgoraJniProxySdk::safeGetFieldID(JNIEnv* env, jclass clazz, const char* name, const char* sig) const {
  CPN(clazz);
  jfieldID fid = env->GetFieldID(clazz, name, sig);
  return fid;
}
jfieldID AgoraJniProxySdk::safeGetFieldID2(JNIEnv* env, jclass clazz, const char* name, const char* sig) const {
  jclass jc = env->FindClass("Lio/agora/recording/common/Common$AUDIO_FRAME_TYPE;");
  jfieldID fid = env->GetFieldID(jc, "type", "I");
  return fid;
}
void AgoraJniProxySdk::cacheAudioPcmFrame(JNIEnv* env){
}
void AgoraJniProxySdk::cacheJavaVideoFrameInitMethodIDs(JNIEnv* env, const char* className){
  if(className && !strcmp(className,CN_VIDEO_H264_FRAME)){
    CP(mJavaVideoYuvFrameClass);
    mJavaVideoYuvFrameInitMtd = safeGetMethodID(env, mJavaVideoYuvFrameClass, SG_MTD_INIT, SN_MTD_VIDEO_YUV_FRAME_INIT);
    CP(mJavaVideoYuvFrameInitMtd);
    if(!mJavaVideoYuvFrameInitMtd) {
      LOG_DIR(m_logdir.c_str(), ERROR,"cannot get video yuv init methodid");
      return;
    }
  }
  if(className && !strcmp(className,CN_VIDEO_H264_FRAME)){
    CP(mJavaVideoH264FrameClass);
    mJavaVideoH264FrameInitMtd = safeGetMethodID(env, mJavaVideoH264FrameClass, SG_MTD_INIT, SN_MTD_VIDEO_H264_FRAME_INIT);
    CP(mJavaVideoH264FrameInitMtd);
    if(!mJavaVideoH264FrameInitMtd) {
      LOG_DIR(m_logdir.c_str(), ERROR,"cannot get video h264 init methodid");
      return;
    }
  }
  if(className && !strcmp(className,CN_VIDEO_JPG_FRAME)){
    CP(mJavaVideoJpgFrameClass);
    mJavaVideoJpgFrameInitMtd = safeGetMethodID(env, mJavaVideoJpgFrameClass, SG_MTD_INIT, SN_MTD_VIDEO_JPG_FRAME_INIT);
    CP(mJavaVideoJpgFrameInitMtd);
    if(!mJavaVideoJpgFrameInitMtd) {
      LOG_DIR(m_logdir.c_str(), ERROR,"cannot get video Jpg init methodid");
      return;
    }
  }
}
void AgoraJniProxySdk::cacheJavaCBFuncMethodIDs4Video(JNIEnv* env, const char* className){
  if (!env) return;
  //AV class
  mJavaVideoFrameInitMtd = safeGetMethodID(env, mJavaVideoFrameClass, SG_MTD_INIT,"(Lio/agora/recording/common/Common;)V");
  if(!mJavaVideoFrameInitMtd) {
    LOG_DIR(m_logdir.c_str(), ERROR,"cannot get videoinit methodid");
    return;
  }
  mJavaVideoFrameYuvFid = env->GetFieldID(mJavaVideoFrameClass, FID_VIDEO_FRAME_YUV, VIDEOFRAME_YUV_SIGNATURE);
  if(!mJavaVideoFrameYuvFid){
    cout<<"mJavaVideoFrameYuvFid is null"<<endl;
    return;
  }
  mJavaVideoFrameJpgFid = env->GetFieldID(mJavaVideoFrameClass, FID_VIDEO_FRAME_JPG, VIDEOFRAME_JPG_SIGNATURE);
  CP(mJavaVideoFrameJpgFid);
  mJavaVideoFrameH264Fid = env->GetFieldID(mJavaVideoFrameClass, FID_VIDEO_FRAME_H264, VIDEOFRAME_H264_SIGNATURE);
  CP(mJavaVideoFrameH264Fid);

 }
void AgoraJniProxySdk::initJavaObjects(JNIEnv* env, bool init){
  if(!init) return;
  mJavaVideoFrameClass = newGlobalJClass(env, CN_VIDEO_FRAME);
  CP(mJavaVideoFrameClass);
  mJavaVideoFrameObject = newGlobalJObject(env, mJavaVideoFrameClass, SN_MTD_COMMON_INIT);
  CP(mJavaVideoFrameObject);
  mJavaVideoYuvFrameClass = newGlobalJClass(env, CN_VIDEO_YUV_FRAME);
  CP(mJavaVideoYuvFrameClass);
  mJavaVideoYuvFrameObject = newGlobalJObject(env, mJavaVideoYuvFrameClass, SN_MTD_VIDEO_YUV_FRAME_INIT); 
  CP(mJavaVideoYuvFrameObject);
  mJavaVideoJpgFrameClass = newGlobalJClass(env, CN_VIDEO_JPG_FRAME);
  CP(mJavaVideoJpgFrameClass);
  mJavaVideoJpgFrameObject = newGlobalJObject(env, mJavaVideoJpgFrameClass, SN_MTD_COMMON_INIT);
  CP(mJavaVideoJpgFrameObject);
  mJavaVideoH264FrameClass = newGlobalJClass(env, CN_VIDEO_H264_FRAME);
  CP(mJavaVideoH264FrameClass);
  mJavaVideoH264FrameObject = newGlobalJObject(env, mJavaVideoH264FrameClass, SN_MTD_COMMON_INIT);
  CP(mJavaVideoH264FrameObject);
  mJavaVideoFrameTypeClass = newGlobalJClass(env, CN_VIDEO_FRAME_TYPE);
  CP(mJavaVideoFrameTypeClass);
  mJavaVideoFrameTypeInitMtd = safeGetMethodID(env, mJavaVideoFrameTypeClass, SG_MTD_INIT, SN_MTD_COMMON_INIT); 
  CP(mJavaVideoFrameTypeInitMtd);
  mJavaVideoFrameTypeObject = newGlobalJObject2(env, mJavaVideoFrameTypeClass, mJavaVideoFrameTypeInitMtd);
  CP(mJavaVideoFrameTypeObject);
  mJavaVideoFrameTypeTypeFid = safeGetFieldID(env, mJavaVideoFrameTypeClass, MTD_TYPE, SG_INT);
  CP(mJavaVideoFrameTypeTypeFid);
  //audio
  mJavaAudioFrameClass = newGlobalJClass(env, CN_AUDIO_FRAME);
  CP(mJavaAudioFrameClass);
  mJavaAudioFrameInitMtd = safeGetMethodID(env, mJavaAudioFrameClass, SG_MTD_INIT, SN_MTD_COMMON_INIT);
  CP(mJavaAudioFrameInitMtd);
  mJavaAudioFrameObject = newGlobalJObject2(env, mJavaAudioFrameClass,  mJavaAudioFrameInitMtd);
  CP(mJavaAudioFrameObject);
  //java audio receive func
  mJavaRecvAudioMtd = safeGetMethodID(env, mJavaAgoraJavaRecordingClass, CB_FUNC_RECEIVE_AUDIOFRAME, SN_CB_FUNC_RECEIVE_AUDIOFRAME);
  CP(mJavaRecvAudioMtd);
  //audio frame type
  mJavaAudioFrameTypeClass = newGlobalJClass(env, CN_AUDIO_FRAME_TYPE);
  CP(mJavaAudioFrameTypeClass);
  mJavaAudioFrameTypeInitMtd = safeGetMethodID(env, mJavaAudioFrameTypeClass, SG_MTD_INIT, SN_MTD_COMMON_INIT);
  CP(mJavaAudioFrameTypeInitMtd);
  mJavaAudioFrameTypeObject = newGlobalJObject2(env, mJavaAudioFrameTypeClass,  mJavaAudioFrameTypeInitMtd);
  CP(mJavaAudioFrameTypeObject);
  //audioFrameType class type fid
  mJavaAudioFrameTypeTypeFid = safeGetFieldID(env, mJavaAudioFrameTypeClass, MTD_TYPE, SG_INT);
  CP(mJavaAudioFrameTypeTypeFid);
  //audio frame type
  mJavaAudioFrameTypeFid = safeGetFieldID(env, mJavaAudioFrameClass, MTD_TYPE, SN_AUDIO_FRAME_TYPE);
  CP(mJavaAudioFrameTypeFid);
  //pcm
  mJavaAudioPcmFrameClass = newGlobalJClass(env, CN_AUDIO_PCM_FRAME);
  CP(mJavaAudioPcmFrameClass);
  mJavaAudioPcmFrameInitMtd = safeGetMethodID(env, mJavaAudioPcmFrameClass, SG_MTD_INIT, SN_INIT_MTD_AUDIO_FRAME);
  CP(mJavaAudioPcmFrameInitMtd);
  mJavaAudioPcmFrameObject = newGlobalJObject2(env, mJavaAudioPcmFrameClass,  mJavaAudioPcmFrameInitMtd);
  CP(mJavaAudioPcmFrameObject);
  //aac
  mJavaAudioAacFrameClass = newGlobalJClass(env, CN_AUDIO_PCM_FRAME);
  CP(mJavaAudioAacFrameClass);
  mJavaAudioAacFrameInitMtd = safeGetMethodID(env, mJavaAudioAacFrameClass, SG_MTD_INIT, SN_INIT_MTD_AUDIO_FRAME);
  CP(mJavaAudioAacFrameInitMtd);
  mJavaAudioAacFrameObject = newGlobalJObject2(env, mJavaAudioAacFrameClass,  mJavaAudioAacFrameInitMtd);
  CP(mJavaAudioAacFrameObject);
}

bool AgoraJniProxySdk::fillAacAllFields(JNIEnv* env, jobject& job, jclass& jc, const agora::linuxsdk::AudioFrame*& frame) const {
  CHECK_PTR_RETURN_BOOL(mJavaAgoraJavaRecordingClass);
  if(frame->type != agora::linuxsdk::AUDIO_FRAME_AAC) return false;
  agora::linuxsdk::AudioAacFrame *f = frame->frame.aac;
  //frame_ms_
  long frame_ms_ = f->frame_ms_;
  env->SetLongField(job, m_AudioPcmFrameFieldIDs[FID_AAC_FRAMEMS], jlong(frame_ms_));
  //aacBuf_
  long aacBufSize_ = f->aacBufSize_;
  jobject buf = env->NewDirectByteBuffer((void*)f->aacBuf_, aacBufSize_);
  env->SetObjectField(job, m_AudioAacFrameFieldIDs[FID_AAC_BUF], buf);
  //aacBufSize_
  env->SetLongField(job, m_AudioAacFrameFieldIDs[FID_AAC_BUFSIZE], jlong(aacBufSize_));
  env->DeleteLocalRef(buf);
  return true;
}

bool AgoraJniProxySdk::fillPcmAllFields(JNIEnv* env, jobject& job, jclass& jc, const agora::linuxsdk::AudioFrame*& frame) const {
  CHECK_PTR_RETURN_BOOL(mJavaAgoraJavaRecordingClass);
  if(frame->type != agora::linuxsdk::AUDIO_FRAME_RAW_PCM) return false;
  agora::linuxsdk::AudioPcmFrame *f = frame->frame.pcm;
  //frame_ms_
  long frame_ms_ = f->frame_ms_;
  env->SetLongField(job, m_AudioPcmFrameFieldIDs[FID_PCM_FRAMEMS], jlong(frame_ms_));
  //channels_
  long channels_ = f->channels_;
  env->SetLongField(job, m_AudioPcmFrameFieldIDs[FID_PCM_CHANNELS], jlong(channels_));
  long sample_bits_ = f->sample_bits_;
  env->SetLongField(job, m_AudioPcmFrameFieldIDs[FID_PCM_SAMPLEBITS], jlong(sample_bits_));
  //sample_rates_
  long sample_rates_ = f->sample_rates_;
  env->SetLongField(job, m_AudioPcmFrameFieldIDs[FID_PCM_SAMPLERATES], jlong(sample_rates_));
  //sampel
  long samples__ = f->samples_;
  env->SetLongField(job, m_AudioPcmFrameFieldIDs[FID_PCM_SAMPLES], jlong(samples__));
  //pcmBuf_
  long pcmBufSize_ = f->pcmBufSize_;
  jobject buf = env->NewDirectByteBuffer((void*)f->pcmBuf_, pcmBufSize_);
  env->SetObjectField(job, m_AudioPcmFrameFieldIDs[FID_PCM_BUF], buf);
  //pcmBufSize_
  env->SetLongField(job, m_AudioPcmFrameFieldIDs[FID_PCM_BUFSIZE], jlong(pcmBufSize_));
  env->DeleteLocalRef(buf);
  return true;
}

bool AgoraJniProxySdk::fillJAudioFrameByFields(JNIEnv* env, const agora::linuxsdk::AudioFrame*& frame, jclass jcAudioFrame, jobject& jobAudioFrame) const {
  //1.find main class
  if (frame->type == agora::linuxsdk::AUDIO_FRAME_RAW_PCM) {
    //call one function
    if(!fillAudioPcmFrame(env, frame, jcAudioFrame,jobAudioFrame)){
      LOG_DIR(m_logdir.c_str(), INFO,"Warning: fillAudioPcmFrame failed!!!!!");
      return false;
    }
  }else if (frame->type == agora::linuxsdk::AUDIO_FRAME_AAC) {
    //do things here
    if(!fillAudioAacFrame(env, frame, jcAudioFrame,jobAudioFrame)){
      LOG_DIR(m_logdir.c_str(), INFO,"Warning: fillAudioAacFrame failed!!!!!");
      return false;
    }
  }
  return true;
}

bool AgoraJniProxySdk::fillAudioAacFrame(JNIEnv* env, const agora::linuxsdk::AudioFrame*& frame, \
            jclass& jcAudioFrame, jobject& jobAudioFrame) const  {
  CHECK_PTR_RETURN_BOOL(mJavaAgoraJavaRecordingClass);
  if(frame->type != agora::linuxsdk::AUDIO_FRAME_AAC) return false;
  jobject job = newJObject(env, mJavaAudioAacFrameClass, mJavaAudioAacFrameInitMtd);
  CPB(job);
  if(!job){
    LOG_DIR(m_logdir.c_str(), ERROR,"new AudioAacFrame failed! no memory?");
    return false;
  }
  if(!fillAacAllFields(env, job, jcAudioFrame, frame)){
    LOG_DIR(m_logdir.c_str(), INFO,"fillAacAllFields failed!");
    return false;
  }
  //Fill in the jobAdudioFrame
  env->SetObjectField(jobAudioFrame, m_AudioFrameFieldIDs[FID_AF_AAC], job);
  env->DeleteLocalRef(job);
  return true;
}
bool AgoraJniProxySdk::fillAudioPcmFrame(JNIEnv* env, const agora::linuxsdk::AudioFrame*& frame, \
            jclass& jcAudioFrame, jobject& jobAudioFrame) const  {
  CHECK_PTR_RETURN_BOOL(mJavaAgoraJavaRecordingClass);
  if(frame->type != agora::linuxsdk::AUDIO_FRAME_RAW_PCM) return false;
  jobject job = newJObject(env, mJavaAudioPcmFrameClass, mJavaAudioPcmFrameInitMtd);
  CPB(job);
  if(!job){
    LOG_DIR(m_logdir.c_str(), ERROR,"new AudioPcmFrame failed! no memory?");
    return false;
  }
  //fill all fields of AudioPcmFrame jobject
  if(!fillPcmAllFields(env, job, jcAudioFrame, frame)){
    LOG_DIR(m_logdir.c_str(), INFO,"fillPcmAllFields failed!");
    return false;
  }
  //Fill in the jobAdudioFrame
  env->SetObjectField(jobAudioFrame, m_AudioFrameFieldIDs[FID_AF_PCM], job);
  env->DeleteLocalRef(job);
  return true;
}
bool AgoraJniProxySdk::fillVideoOfYUV(JNIEnv* env, const agora::linuxsdk::VideoFrame*& frame, jclass& jcVideoFrame, jobject& jobVideoFrame) const {
  CHECK_PTR_RETURN_BOOL(mJavaAgoraJavaRecordingClass);
  if(frame->type != agora::linuxsdk::VIDEO_FRAME_RAW_YUV) return false;
  if(!env || !frame) return false;
  agora::linuxsdk::VideoYuvFrame *f = frame->frame.yuv;
  if(!f) {
    LOG_DIR(m_logdir.c_str(), ERROR,"yuv frame is nullptr");
    return false;
  }
  jobject job = newJObject(env, mJavaVideoYuvFrameClass, mJavaVideoYuvFrameInitMtd);
  if(!job){
    cout<<"new yuv frame return null"<<endl;
    return false;
  }
  long frame_ms_ = f->frame_ms_;
  env->SetLongField(job, m_VideoYuvFrameFieldIDs[FID_FRAMEMS], jlong(frame_ms_));
  long bufSize_ = f->bufSize_;
  jobject buf  = env->NewDirectByteBuffer((void*)(f->buf_), bufSize_);
  env->SetObjectField(job, m_VideoYuvFrameFieldIDs[FID_YUVBUF], buf);
  env->SetLongField(job, m_VideoYuvFrameFieldIDs[FID_YUVBUFSIZE], jlong(bufSize_));
  env->SetObjectField(jobVideoFrame, mJavaVideoFrameYuvFid, job);
  env->DeleteLocalRef(buf);
  env->DeleteLocalRef(job);
  return  true;
}
bool AgoraJniProxySdk::fillVideoOfJPG(JNIEnv* env, const agora::linuxsdk::VideoFrame*& frame, jclass& jcVideoFrame, jobject& jobVideoFrame) const{
  CHECK_PTR_RETURN_BOOL(mJavaAgoraJavaRecordingClass);
  LOG_DIR(m_logdir.c_str(), INFO,"AgoraJniProxySdk::fillVideoOfJPG enter" );
  if(frame->type != agora::linuxsdk::VIDEO_FRAME_JPG) return false;
  if(!env || !frame) return false;
  jobject job = NULL;

  //3.new VideoXXXXXFrame object
  job = env->NewObject(mJavaVideoJpgFrameClass, mJavaVideoJpgFrameInitMtd);
  if(!job){
    LOG_DIR(m_logdir.c_str(), ERROR,"new subclass  failed! no memory?");
    return false;
  }
  agora::linuxsdk::VideoJpgFrame *f = frame->frame.jpg;
  if(!f) return false;
  //4.fill all fields
  //4.1 get & set of this subclass object
  //frame_ms_
  long frame_ms_ = f->frame_ms_;
  env->SetLongField(job, m_VideoJpgFrameFieldIDs[FID_JPG_FRAMEMS], jlong(frame_ms_));
  const unsigned char* buf_ = f->buf_;
  long bufSize_ = f->bufSize_;

  jobject jbArr = NULL;
  jbArr = env->NewDirectByteBuffer((void*)f->buf_, bufSize_);

  env->SetObjectField(job, m_VideoJpgFrameFieldIDs[FID_JPG_BUF], jbArr);
  //bufSize_
  env->SetLongField(job, m_VideoJpgFrameFieldIDs[FID_JPG_BUFSIZE], jlong(bufSize_));
  //6.fill jobVideFrame
  env->SetObjectField(jobVideoFrame, mJavaVideoFrameJpgFid, job);
  env->DeleteLocalRef(job);
  env->DeleteLocalRef(jbArr);
  return  true;
}
bool AgoraJniProxySdk::fillVideoOfH264(JNIEnv* env, const agora::linuxsdk::VideoFrame*& frame, jclass& jcVideoFrame, jobject& jobVideoFrame) const {
  CHECK_PTR_RETURN_BOOL(mJavaAgoraJavaRecordingClass);
  if(frame->type == agora::linuxsdk::VIDEO_FRAME_JPG || frame->type == agora::linuxsdk::VIDEO_FRAME_RAW_YUV) return false;
  if(!env || !frame) return false;
  jclass jc = NULL;
  jmethodID initMid = NULL;
  jobject job = NULL;
  jfieldID fid = NULL;
  int fieldId = 0; //TODO
  //3.new VideoH264Frame object
  CPB(mJavaVideoH264FrameClass);
  CPB(mJavaVideoH264FrameInitMtd);
  job = env->NewObject(mJavaVideoH264FrameClass, mJavaVideoH264FrameInitMtd);
  if(!job){
    LOG_DIR(m_logdir.c_str(), ERROR,"new subclass  failed! no memory?");
    return false;
  }
  agora::linuxsdk::VideoH264Frame *f = frame->frame.h264;
  if(!f) return false;
  //frame_ms_
  long frame_ms_ = f->frame_ms_;
  env->SetLongField(job, m_VideoH264FrameFieldIDs[FID_H264_FRAMEMS], jlong(frame_ms_));
  //frame_num_
  long frame_num_ = f->frame_num_;
  env->SetLongField(job, m_VideoH264FrameFieldIDs[FID_H264_FRAMENUM], jlong(frame_num_));
  //buf_
  const unsigned char* buf_ = f->buf_;
  long bufSize_ = f->bufSize_;
  jobject buf = env->NewDirectByteBuffer((void*)f->buf_, bufSize_);
  env->SetObjectField(job, m_VideoH264FrameFieldIDs[FID_H264_BUF], buf);
  //bufSize_
  env->SetLongField(job, m_VideoH264FrameFieldIDs[FID_H264_BUF_SIZE], jlong(bufSize_));
  //5.get subclass field
  env->SetObjectField(jobVideoFrame, mJavaVideoFrameH264Fid, job);
  env->DeleteLocalRef(job);
  env->DeleteLocalRef(buf);
  return  true;
}
bool AgoraJniProxySdk::fillVideoFrameByFields(JNIEnv* env, const agora::linuxsdk::VideoFrame*& frame, jclass jcVideoFrame, jobject jobVideoFrame) const{
  CHECK_PTR_RETURN_BOOL(mJavaAgoraJavaRecordingClass);
  bool ret = false;
  if(!env || !frame || !jcVideoFrame || !jobVideoFrame){
    LOG_DIR(m_logdir.c_str(), ERROR,"AgoraJniProxySdk::fillVideoFrameByFields para error!");
    return ret;
  }
  if (frame->type == agora::linuxsdk::VIDEO_FRAME_RAW_YUV) {
    if(!fillVideoOfYUV(env, frame, jcVideoFrame, jobVideoFrame)){
      LOG_DIR(m_logdir.c_str(), INFO,"fill subclass falied!");
      return false;
    }
  }else if(frame->type == agora::linuxsdk::VIDEO_FRAME_JPG){
    if(!fillVideoOfJPG(env, frame, jcVideoFrame, jobVideoFrame)) {
      LOG_DIR(m_logdir.c_str(), INFO,"fill subclass falied!");
      return false;
    }
  }else{
    if(!fillVideoOfH264(env, frame, jcVideoFrame, jobVideoFrame))
    {
      LOG_DIR(m_logdir.c_str(), INFO,"fillVideoOfH264 failed!");
      return false;
    }
  }
  return true;
}
void AgoraJniProxySdk::videoFrameReceived(unsigned int uid, const agora::linuxsdk::VideoFrame *frame) const {
  CHECK_PTR_RETURN(mJavaAgoraJavaRecordingClass);
  AttachThreadScoped ats(g_jvm);
  JNIEnv* env = ats.env();
  if (!env) return;
  jobject job = newJObject(env, mJavaVideoFrameClass,mJavaVideoFrameInitMtd);
  if(!fillVideoFrameByFields(env, frame, mJavaVideoFrameClass, job))
  {
    LOG_DIR(m_logdir.c_str(), ERROR,"jni fillVideoFrameByFields failed!" );
    return;
  }
  env->CallVoidMethod(mJavaAgoraJavaRecordingObject, m_CBObjectMethodIDs[MID_ON_VIDEOFRAME_RECEIVED], jlong(long(uid)),frame->type, job, frame->rotation_);
  env->DeleteLocalRef(job);
  return;
}
//TODO  use the same parameter
void AgoraJniProxySdk::audioFrameReceived(unsigned int uid, const agora::linuxsdk::AudioFrame *frame) const {
  CHECK_PTR_RETURN(mJavaAgoraJavaRecordingClass);
  AttachThreadScoped ats(g_jvm);
  JNIEnv* env = ats.env();
  if (!env) return;
  jobject jobAudioFrame = newJObject(env, mJavaAudioFrameClass, mJavaAudioFrameInitMtd);
  if(!jobAudioFrame){
    LOG_DIR(m_logdir.c_str(), ERROR,"new audio frame object failed!");
    return;
  }
  if(!fillJAudioFrameByFields(env, frame, mJavaAudioFrameClass, jobAudioFrame)) {
    LOG_DIR(m_logdir.c_str(), ERROR,"fillJAudioFrameByFields failed!" );
    return;
  }
  int type = static_cast<int>(frame->type);
  env->CallVoidMethod(mJavaAgoraJavaRecordingObject, m_CBObjectMethodIDs[MID_ON_AUDIOFRAME_RECEIVED], jlong(long(uid)), jint(type), jobAudioFrame);
  env->DeleteLocalRef(jobAudioFrame);
  return;
}

void AgoraJniProxySdk::onUserJoined(agora::linuxsdk::uid_t uid, agora::linuxsdk::UserJoinInfos &infos) {
  LOG_DIR(m_logdir.c_str(), INFO,"AgoraJniProxySdk User:%u" ,uid , " joined, RecordingDir:%s" , (infos.storageDir? infos.storageDir:"NULL") );
  CHECK_PTR_RETURN(mJavaAgoraJavaRecordingClass);
  std::string store_dir = std::string(infos.storageDir);
  m_logdir = store_dir;
  
  AttachThreadScoped ats(g_jvm);
  JNIEnv* env = ats.env();
  if (!env) return;
  if(!m_CBObjectMethodIDs[MID_ON_USERJOINED]){
    LOG_DIR(m_logdir.c_str(), ERROR,"MID_ON_USERJOINED not inited");
    return;
  }
  jstring jstrRecordingDir = env->NewStringUTF(store_dir.c_str());
  env->CallVoidMethod(mJavaAgoraJavaRecordingObject, m_CBObjectMethodIDs[MID_ON_USERJOINED], jlong((long)(uid)),jstrRecordingDir);
  return;
}
void AgoraJniProxySdk::onUserOffline(agora::linuxsdk::uid_t uid, agora::linuxsdk::USER_OFFLINE_REASON_TYPE reason) {
  LOG_DIR(m_logdir.c_str(), INFO,"AgoraJniProxySdk onUserOffline User:%u",uid, ",reason:%d",static_cast<int>(reason));
  CHECK_PTR_RETURN(mJavaAgoraJavaRecordingClass);
  AttachThreadScoped ats(g_jvm);
  JNIEnv* env = ats.env();
  if (!env) return;
  if(!m_CBObjectMethodIDs[MID_ON_USEROFFLINE]){
    LOG_DIR(m_logdir.c_str(), ERROR,"MID_ON_USEROFFLINE not inited" );
    return;
  }
  env->CallVoidMethod(mJavaAgoraJavaRecordingObject, m_CBObjectMethodIDs[MID_ON_USEROFFLINE], jlong((long)(uid)),jint(int(reason)));

  return;
}
void AgoraJniProxySdk::onLeaveChannel(agora::linuxsdk::LEAVE_PATH_CODE code) {
  LOG_DIR(m_logdir.c_str(), INFO,"AgoraJniProxySdk onLeaveChannel");
  CHECK_PTR_RETURN(mJavaAgoraJavaRecordingClass);

  AttachThreadScoped ats(g_jvm);
  JNIEnv* env = ats.env();
  if (!env) return;
  if(!m_CBObjectMethodIDs[MID_ON_LEAVECHANNEL]){
    LOG_DIR(m_logdir.c_str(), ERROR,"MID_ON_LEAVECHANNEL not inited" );
    return;
  }
  env->CallVoidMethod(mJavaAgoraJavaRecordingObject, m_CBObjectMethodIDs[MID_ON_LEAVECHANNEL], jint((int)(code)));
  return;
}
void AgoraJniProxySdk::onWarning(int warn) {
  CHECK_PTR_RETURN(mJavaAgoraJavaRecordingClass);
  
  AttachThreadScoped ats(g_jvm);
  JNIEnv* env = ats.env();
  if (!env) return;
  if(!m_CBObjectMethodIDs[MID_ON_WARNING]){
    LOG_DIR(m_logdir.c_str(), ERROR,"MID_ON_WARNING not inited" );
    return;
  }
  env->CallVoidMethod(mJavaAgoraJavaRecordingObject, m_CBObjectMethodIDs[MID_ON_WARNING], warn);
  return;
}

void AgoraJniProxySdk::onError(int error, agora::linuxsdk::STAT_CODE_TYPE stat_code) {
  LOG_DIR(m_logdir.c_str(), INFO,"AgoraJniProxySdk onError");
  CHECK_PTR_RETURN(mJavaAgoraJavaRecordingClass);
  AttachThreadScoped ats(g_jvm);
  JNIEnv* env = ats.env();
  if (!env) return;

  if(!m_CBObjectMethodIDs[MID_ON_ERROR]) {
    LOG_DIR(m_logdir.c_str(), INFO,"MID_ON_ERROR not inited!");
    return;
  }
  env->CallVoidMethod(mJavaAgoraJavaRecordingObject, m_CBObjectMethodIDs[MID_ON_ERROR], error, jint((int)(stat_code)));
  leaveChannel();
  ats.detach();
  return;
}
JNIEXPORT jint JNI_OnLoad(JavaVM* jvm, void* reserved) {
  g_jvm = jvm;
  return JNI_VERSION_1_4;
}

/*
 * Class:     AgoraJavaRecording
 * Method:    leaveChannel
 * Signature: ()Z
 */
JNIEXPORT jboolean JNICALL Java_AgoraJavaRecording_leaveChannel
  (JNIEnv *, jobject, jlong nativeObjectRef) {
  cout<<"java call leaveChannel"<<endl;
  jniproxy::AgoraJniProxySdk* nativeHandle = reinterpret_cast<jniproxy::AgoraJniProxySdk*>(nativeObjectRef);
  /*if(!nativeHandle){
    return JNI_FALSE;
  }
  nativeHandle->leaveChannel();*/
  g_bSignalStop = true;
  return JNI_TRUE;
}


/*
 * Class:     AgoraJavaRecording
 * Method:    setVideoMixingLayout
 * Signature: (JLio/agora/recording/common/Common/VideoMixingLayout;)I
 */
JNIEXPORT jint JNICALL Java_AgoraJavaRecording_setVideoMixingLayout
  (JNIEnv * env, jobject job, jlong nativeObjectRef, jobject jVideoMixLayout)
{
  jniproxy::AgoraJniProxySdk* nativeHandle = reinterpret_cast<jniproxy::AgoraJniProxySdk*>(nativeObjectRef);
  if(!nativeHandle){
    cout<<"setVideoMixingLayout nativeHandle is null"<<endl;
    return JNI_FALSE;
  }
  //convert jVideoMixLayout to c++ VideoMixLayout
	jclass jcVideoMixingLayout  = env->GetObjectClass(jVideoMixLayout); 
  if(!jcVideoMixingLayout){
    cout<<"jcVideoMixingLayout is NULL";
    return JNI_FALSE;
	}
  jfieldID jCanvasWidthID = env->GetFieldID(jcVideoMixingLayout, "canvasWidth", INT_SIGNATURE);
  jfieldID jCanvasHeightID = env->GetFieldID(jcVideoMixingLayout, "canvasHeight", INT_SIGNATURE);
  jfieldID jBackgroundColorID = env->GetFieldID(jcVideoMixingLayout, "backgroundColor", STRING_SIGNATURE);
  jfieldID jRegionCountID = env->GetFieldID(jcVideoMixingLayout, "regionCount", INT_SIGNATURE);
  jfieldID jRegionsID = env->GetFieldID(jcVideoMixingLayout, "regions", VIDEOMIXLAYOUT_SIGNATURE);
  jfieldID jAppDataID = env->GetFieldID(jcVideoMixingLayout, "appData", STRING_SIGNATURE);
  jfieldID jAppDataLengthID = env->GetFieldID(jcVideoMixingLayout, "appDataLength", INT_SIGNATURE);
  
  if(!jCanvasWidthID || !jCanvasHeightID || !jBackgroundColorID || !jRegionCountID || !jRegionsID){
    cout<<"Java_AgoraJavaRecording_setVideoMixingLayout get fields failed!"<<endl;
    return JNI_FALSE;
  }
  //convert into cpp value
  jint canvasWidth = env->GetIntField(jVideoMixLayout,jCanvasWidthID);
  jint canvasHeight = env->GetIntField(jVideoMixLayout,jCanvasWidthID);
  
  jstring jstrBackgroundColor = (jstring)env->GetObjectField(jVideoMixLayout, jBackgroundColorID);
  const char* c_backgroundColor = env->GetStringUTFChars(jstrBackgroundColor, JNI_FALSE);
  
  jint regionCount = env->GetIntField(jVideoMixLayout,jRegionCountID);
  
  //jstring jstrAppData = (jstring)env->GetObjectField(jVideoMixLayout,jAppDataID);
  //const char* c_jstrAppData = env->GetStringUTFChars(jstrAppData, JNI_FALSE);
  //jint appDataLength = env->GetIntField(jVideoMixLayout,jAppDataLengthID);

  agora::linuxsdk::VideoMixingLayout layout;
 
  layout.canvasWidth = int(canvasWidth);
  layout.canvasHeight = int(canvasHeight);
  layout.backgroundColor = c_backgroundColor;
  layout.regionCount = int(regionCount);

  //layout.appData = c_jstrAppData;
  //layout.appDataLength = int(appDataLength);
  //regions begin
  
  if(0<regionCount)
  {
   jobjectArray jobRegions =  (jobjectArray)env->GetObjectField(jVideoMixLayout, jRegionsID);
   if(!jobRegions) {
    cout<<"[ERROR]regionCount is bigger than zero,but cannot find Regions in jVideoMixLayout!"<<endl;
    return JNI_FALSE;
   }
   jint arrLen = env->GetArrayLength(jobRegions);
   if(arrLen != regionCount){
    cout<<"regionCount is not equal with arrLen"<<endl;
    return JNI_FALSE;//return ??
   }  
    agora::linuxsdk::VideoMixingLayout::Region* regionList = new agora::linuxsdk::VideoMixingLayout::Region[arrLen];
    for(int i=0; i<arrLen;++i){
      jobject region = env->GetObjectArrayElement(jobRegions, i);
      jclass jcRegion =env->GetObjectClass(region);
      if(!jcRegion){
        cout<<"cannot get jclass Region!"<<endl;
        break;
      }
      jfieldID jfidUid = env->GetFieldID(jcRegion, "uid", LONG_SIGNATURE);
      if(!jfidUid){
        cout<<"connot get region uid"<<endl;
        continue;
      }
      //uid
      jlong uidValue = env->GetLongField(region,jfidUid);
      //C++ uid is uint32
      regionList[i].uid = static_cast<uint32_t>(uidValue);
      //x
      jfieldID jxID = env->GetFieldID(jcRegion, "x", DOUBLE_SIGNATURE);
      if(!jxID){
        cout<<"connot get region x,uid:"<<uint32_t(uidValue)<<endl;
        continue;
      }
      jdouble jx = env->GetDoubleField(region,jxID);
      regionList[i].x = static_cast<float>(jx);

      //y
      jfieldID jyID = env->GetFieldID(jcRegion, "y", DOUBLE_SIGNATURE);
      if(!jyID){
        cout<<"connot get region y,uid:"<<int(uidValue)<<endl;
        continue;
      }
      jdouble jy = env->GetDoubleField(region,jyID);
      regionList[i].y = static_cast<float>(jy);

      //width
      jfieldID jwidthID = env->GetFieldID(jcRegion, "width", DOUBLE_SIGNATURE);
      if(!jwidthID){
        cout<<"connot get region width, uid:"<<int(uidValue)<<endl;
        continue;
      }
      jdouble jwidth = env->GetDoubleField(region,jwidthID);
      regionList[i].width = static_cast<float>(jwidth);

      //height
      jfieldID jheightID = env->GetFieldID(jcRegion, "height", DOUBLE_SIGNATURE);
      if(!jheightID){
        cout<<"connot get region height, uid:"<<int(uidValue)<<endl;
        continue;
      }
      jdouble jheight = env->GetDoubleField(region,jheightID);
      regionList[i].height = static_cast<float>(jheight);
      cout<<"user id:"<<static_cast<uint32_t>(uidValue)<<",x:"<<static_cast<float>(jx)<<",y:"<<static_cast<float>(jy)<<",width:"<<static_cast<float>(jwidth)<<",height:"<<static_cast<float>(jheight)<<",alpha:"<<static_cast<double>(i + 1)<<endl;

      //zOrder
      regionList[i].zOrder = 0;
      //alpha
      regionList[i].alpha = static_cast<double>(i + 1);
      //renderMode
      regionList[i].renderMode = 0;
    }  
    layout.regions = regionList;
  }
  else
    layout.regions = NULL;
  //regions end
  nativeHandle->setVideoMixingLayout(layout);
  return jint(0);
}

/*                                                                                                                                                                           * Class:     AgoraJavaRecording
 * Method:    getProperties
 * Signature: (J)LRecordingEngineProperties;
 */
JNIEXPORT jobject JNICALL Java_AgoraJavaRecording_getProperties(JNIEnv * env, jobject, jlong nativeObjectRef){
  jniproxy::AgoraJniProxySdk* nativeHandle = reinterpret_cast<jniproxy::AgoraJniProxySdk*>(nativeObjectRef);
  if(!nativeHandle){
    return JNI_FALSE;
  }
  jclass jc = env->FindClass(CN_REP);
  if(!jc){
    cout<<"cannot get jclass RecordingEngineProperties!"<<endl;    
    return JNI_FALSE;
  }
  jmethodID initMid = env->GetMethodID(jc,SG_MTD_INIT,"(Lio/agora/recording/common/Common;)V");
  if(!initMid){
    cout<<"cannot get RecordingEngineProperties init!"<<endl;
    return JNI_FALSE;
  }
  jobject job = NULL;
  job = env->NewObject(jc, initMid);
  if(!job){
    cout<<"new object failed!"<<endl;
    return JNI_FALSE;
  }
  jfieldID storageFid = env->GetFieldID(jc, "storageDir", "Ljava/lang/String;");
  if(!storageFid){
    cout<<"storageDir fid not found!"<<endl;
    return JNI_FALSE;
  }
  char* storageDir = nativeHandle->getRecorderProperties()->storageDir;
  jstring jstrStorageDir = env->NewStringUTF(storageDir);
  env->SetObjectField(job, storageFid, jstrStorageDir);
  env->DeleteLocalRef(jc);
  return job;
}
/*
 * Class:     AgoraJavaRecording
 * Method:    startService
 * Signature: (J)V
 */
 JNIEXPORT jint JNICALL Java_AgoraJavaRecording_startService(JNIEnv * env, jobject job, jlong nativeObjectRef){
   jniproxy::AgoraJniProxySdk* nativeHandle = reinterpret_cast<jniproxy::AgoraJniProxySdk*>(nativeObjectRef);
   if(nativeHandle){
     return nativeHandle->startService();
   }
   return -1;
 }
/*
 * Class:     AgoraJavaRecording
 * Method:    stopService
 * Signature: (J)V
 */
JNIEXPORT jint JNICALL Java_AgoraJavaRecording_stopService(JNIEnv * env, jobject job, jlong nativeObjectRef){
  jniproxy::AgoraJniProxySdk* nativeHandle = reinterpret_cast<jniproxy::AgoraJniProxySdk*>(nativeObjectRef);
  if(nativeHandle){
    return nativeHandle->stopService();
  }
  return -1;
}


void AgoraJniProxySdk::stopJavaProc(JNIEnv* env) {
  LOG_DIR(m_logdir.c_str(), WARN,"AgoraJniProxySdk stopJavaProc");
  CHECK_PTR_RETURN(mJavaAgoraJavaRecordingClass);
  jmethodID jStopCB =  env->GetMethodID(mJavaAgoraJavaRecordingClass,"stopCallBack","()V");
  assert(jStopCB);
  env->CallVoidMethod(mJavaAgoraJavaRecordingObject, jStopCB);
}

void AgoraJniProxySdk::setJavaRecordingPath(JNIEnv* env, std::string& storeDir){
  CHECK_PTR_RETURN(mJavaAgoraJavaRecordingClass);
  
  jmethodID jRecordingPathCB =  env->GetMethodID(mJavaAgoraJavaRecordingClass,"recordingPathCallBack","(Ljava/lang/String;)V");
  jstring jstrRecordingDir = env->NewStringUTF(storeDir.c_str());

  env->CallVoidMethod(mJavaAgoraJavaRecordingObject, jRecordingPathCB, jstrRecordingDir);
}
/*void AgoraJniProxySdk::setJavaObjects(bool init, jobject job){
    
}*/

JNIEXPORT jboolean JNICALL Java_AgoraJavaRecording_createChannel(JNIEnv * env, jobject thisObj, jstring jni_appid, jstring jni_channelKey, 
      jstring jni_channelName, jint jni_uid, jobject jni_recordingConfig)
{
  uint32_t uid = 0;
  string appId;
  string channelKey;
  string channelName;
  uint32_t channelProfile = 0;

  string decryptionMode;
  string secret;
  string mixResolution("360,640,15,500");

  int idleLimitSec=5*60;//300s

  string applitePath;
  string appliteLogPath;
  string recordFileRootDir = "";
  string cfgFilePath = "";
  string proxyServer;

  int lowUdpPort = 0;//40000;
  int highUdpPort = 0;//40004;

  bool isAudioOnly=0;
  bool isVideoOnly=0;
  bool isMixingEnabled=0;
  bool mixedVideoAudio=0;

  uint32_t getAudioFrame = agora::linuxsdk::AUDIO_FORMAT_DEFAULT_TYPE;
  uint32_t getVideoFrame = agora::linuxsdk::VIDEO_FORMAT_DEFAULT_TYPE;
  uint32_t streamType = agora::linuxsdk::REMOTE_VIDEO_STREAM_HIGH;
  int captureInterval = 5;
  int triggerMode = 0;
  int lang = 1;
  
  g_bSignalStop = false;

  signal(SIGQUIT, signal_handler);
  signal(SIGABRT, signal_handler);
  signal(SIGINT, signal_handler);
  signal(SIGPIPE, SIG_IGN);

  //const char* appId = NULL;
  const char* c_appId = env->GetStringUTFChars(jni_appid, JNI_FALSE);
  appId = c_appId;
  env->ReleaseStringUTFChars(jni_appid, c_appId);
  if(appId.empty()){
    cout<<"get appId is NULL"<<endl;
    return JNI_FALSE;
  }
  
  //const char* channelKey = NULL;
  const char* c_channelKey = env->GetStringUTFChars(jni_channelKey, JNI_FALSE);
  channelKey = c_channelKey;
  env->ReleaseStringUTFChars(jni_channelKey, c_channelKey);
  if(channelKey.empty()){
     cout<<"get channel key is NULL"<<endl;
  }
  //const char* channelName = NULL; 
  const char* c_channelName = env->GetStringUTFChars(jni_channelName, JNI_FALSE);
  channelName = c_channelName;
  env->ReleaseStringUTFChars(jni_channelName, c_channelName);
  if(channelName.empty()){
    cout<<"channel name is empty!"<<endl;
    return JNI_FALSE;
  }
  uid = (int)jni_uid;
  if(uid < 0){
    cout<<"jni uid is smaller than 0, set 0!"<<endl;
    uid = 0;
  }
  jclass jRecordingJavaConfig =env->GetObjectClass(jni_recordingConfig); 
  if(!jRecordingJavaConfig){
    cout<<"jni_recordingConfig is NULL"<<endl;
    return JNI_FALSE;
  }
  //get parameters field ID
  jfieldID idleLimitSecFieldID = env->GetFieldID(jRecordingJavaConfig, "idleLimitSec", INT_SIGNATURE);
  jfieldID channelProfileFieldID = env->GetFieldID(jRecordingJavaConfig, "channelProfile", CHANNEL_PROFILE_SIGNATURE);
  jfieldID isVideoOnlyFid = env->GetFieldID(jRecordingJavaConfig, "isVideoOnly", BOOL_SIGNATURE);
  jfieldID isAudioOnlyFid = env->GetFieldID(jRecordingJavaConfig, "isAudioOnly", BOOL_SIGNATURE);
  jfieldID isMixingEnabledFid = env->GetFieldID(jRecordingJavaConfig, "isMixingEnabled", BOOL_SIGNATURE);
		
  jfieldID mixResolutionFid = env->GetFieldID(jRecordingJavaConfig, "mixResolution", STRING_SIGNATURE);
  jfieldID mixedVideoAudioFid = env->GetFieldID(jRecordingJavaConfig, "mixedVideoAudio", BOOL_SIGNATURE);
  jfieldID appliteDirFieldID = env->GetFieldID(jRecordingJavaConfig, "appliteDir", STRING_SIGNATURE);
  jfieldID recordFileRootDirFid = env->GetFieldID(jRecordingJavaConfig, "recordFileRootDir", STRING_SIGNATURE);
  jfieldID cfgFilePathFid = env->GetFieldID(jRecordingJavaConfig, "cfgFilePath", STRING_SIGNATURE);
  jfieldID secretFid = env->GetFieldID(jRecordingJavaConfig, "secret", STRING_SIGNATURE);
  jfieldID decryptionModeFid = env->GetFieldID(jRecordingJavaConfig, "decryptionMode", STRING_SIGNATURE);
  jfieldID lowUdpPortFid = env->GetFieldID(jRecordingJavaConfig, "lowUdpPort", INT_SIGNATURE);
  jfieldID highUdpPortFid = env->GetFieldID(jRecordingJavaConfig, "highUdpPort", INT_SIGNATURE);
  jfieldID captureIntervalFid = env->GetFieldID(jRecordingJavaConfig, "captureInterval", INT_SIGNATURE);
  jfieldID streamTypeFieldID = env->GetFieldID(jRecordingJavaConfig, "streamType", REMOTE_VIDEO_STREAM_SIGNATURE);
  jfieldID decodeAudioFieldID = env->GetFieldID(jRecordingJavaConfig, "decodeAudio", AUDIO_FORMAT_TYPE_SIGNATURE);
  jfieldID decodeVideoFieldID = env->GetFieldID(jRecordingJavaConfig, "decodeVideo", VIDEO_FORMAT_TYPE_SIGNATURE);
  jfieldID triggerModeFid = env->GetFieldID(jRecordingJavaConfig, "triggerMode", INT_SIGNATURE);
  jfieldID proxyServerFid = env->GetFieldID(jRecordingJavaConfig, "proxyServer", STRING_SIGNATURE);
  if (!idleLimitSecFieldID || !appliteDirFieldID || !channelProfileFieldID 
						|| !streamTypeFieldID || !decodeAudioFieldID || !decodeVideoFieldID || !isMixingEnabledFid) { 
            cout<<"get fieldID failed!";return JNI_FALSE;}
  //idle
  idleLimitSec = (int)env->GetIntField(jni_recordingConfig, idleLimitSecFieldID); 
  //appliteDir
  jstring appliteDir = (jstring)env->GetObjectField(jni_recordingConfig, appliteDirFieldID);
  const char * c_appliteDir = env->GetStringUTFChars(appliteDir, JNI_FALSE);
  applitePath = c_appliteDir;
  env->ReleaseStringUTFChars(appliteDir,c_appliteDir);
  env->DeleteLocalRef(appliteDir);
  //CHANNEL_PROFILE_TYPE
  jobject channelProfileObject = (env)->GetObjectField(jni_recordingConfig, channelProfileFieldID);
  //assert(channelProfileObject);
  jclass enumClass = env->GetObjectClass(channelProfileObject);
  if(!enumClass) {  
    cout<<"enumClass is null";
    return JNI_FALSE;
  }
  jmethodID getValue = env->GetMethodID(enumClass, "getValue", EMPTY_PARA_INT_RETURN);
  if (!getValue) {
    cout<<"method not found";
    return JNI_FALSE; /* method not found */
  }
  jint channelProfileValue = env->CallIntMethod(channelProfileObject, getValue);
  channelProfile=int(channelProfileValue);
  //streamType
  jobject streamTypeObject = (env)->GetObjectField(jni_recordingConfig, streamTypeFieldID);
  jclass streamTypeClass = env->GetObjectClass(streamTypeObject);
  assert(streamTypeObject);
  if(!streamTypeObject){cout<<"streamTypeEnum is NULL"; return JNI_FALSE;}
  jmethodID getValue2 = env->GetMethodID(streamTypeClass, "getValue", EMPTY_PARA_INT_RETURN);
  jint streamTypeValue = env->CallIntMethod(streamTypeObject, getValue2);
  streamType = int(streamTypeValue);
  //decodeAudio
  jobject jobDecodeAudio = (env)->GetObjectField(jni_recordingConfig, decodeAudioFieldID);
  jclass jcdecodeAudio = env->GetObjectClass(jobDecodeAudio);
  if(!jcdecodeAudio) {
    cout<<"jcdecodeAudio is null";
  }
  jmethodID jmidGetValue = env->GetMethodID(jcdecodeAudio, "getValue", EMPTY_PARA_INT_RETURN);
  if (!jmidGetValue) {
    cout<<"jmidGetValue not found";
    return JNI_FALSE; /* method not found */
  }
  jint decodeAudioValue = env->CallIntMethod(jobDecodeAudio, jmidGetValue);
  getAudioFrame = int(decodeAudioValue);
  env->DeleteLocalRef(jobDecodeAudio);
  env->DeleteLocalRef(jcdecodeAudio);
  //decodeVideo
  jobject jobDecodeVideo = (env)->GetObjectField(jni_recordingConfig, decodeVideoFieldID);
  jclass jcdecodeVideo = env->GetObjectClass(jobDecodeVideo);
  if(!jcdecodeVideo) {
    cout<<"jcdecodeVideo is null";
  }
  jmidGetValue = env->GetMethodID(jcdecodeVideo, "getValue", EMPTY_PARA_INT_RETURN);
  if (!jmidGetValue) {
    cout<<"jmidGetValue not found";
    return JNI_FALSE; /* method not found */
  }
  jint decodeVideoValue = env->CallIntMethod(jobDecodeVideo, jmidGetValue);
  getVideoFrame = int(decodeVideoValue);
  //isMixingEnabled
  jboolean isMixingEnabledValue = env->GetBooleanField(jni_recordingConfig, isMixingEnabledFid);
  isMixingEnabled = bool(isMixingEnabledValue);
  //isVideoOnly
  jboolean jisVideoOnly = (int)env->GetIntField(jni_recordingConfig, isVideoOnlyFid); 
  isVideoOnly = bool(jisVideoOnly);
  //isAudioOnly
  jboolean jisAudioOnly = (int)env->GetIntField(jni_recordingConfig, isAudioOnlyFid); 
  isAudioOnly = bool(jisAudioOnly);
  //mixResolution
  jstring jmixResolution = (jstring)env->GetObjectField(jni_recordingConfig, mixResolutionFid);
  const char * c_mixResolution = env->GetStringUTFChars(jmixResolution, JNI_FALSE);
  mixResolution = c_mixResolution;
  env->ReleaseStringUTFChars(jmixResolution, c_mixResolution);
  env->DeleteLocalRef(jmixResolution);

  //mixedVideoAudio
  jboolean jmixedVideoAudio = env->GetBooleanField(jni_recordingConfig, mixedVideoAudioFid);
  mixedVideoAudio = bool(jmixedVideoAudio);

  //recordFileRootDir
  jstring jrecordFileRootDir = (jstring)env->GetObjectField(jni_recordingConfig, recordFileRootDirFid);
  const char * c_recordFileRootDir = env->GetStringUTFChars(jrecordFileRootDir, JNI_FALSE);
  recordFileRootDir = c_recordFileRootDir;
  env->ReleaseStringUTFChars(jrecordFileRootDir, c_recordFileRootDir);
  env->DeleteLocalRef(jrecordFileRootDir);

  //cfgFilePath
  jstring jcfgFilePath = (jstring)env->GetObjectField(jni_recordingConfig, cfgFilePathFid);
  const char * c_cfgFilePath = env->GetStringUTFChars(jcfgFilePath, JNI_FALSE);
  cfgFilePath = c_cfgFilePath;
  env->ReleaseStringUTFChars(jcfgFilePath, c_cfgFilePath);
  env->DeleteLocalRef(jcfgFilePath);

  //secret
  jstring jsecret = (jstring)env->GetObjectField(jni_recordingConfig, secretFid);
  const char * c_secret = env->GetStringUTFChars(jsecret, JNI_FALSE);
  secret = c_secret;
  env->ReleaseStringUTFChars(jsecret, c_secret);
  env->DeleteLocalRef(jsecret);

  //proxyServer
  jstring jproxy = (jstring)env->GetObjectField(jni_recordingConfig, proxyServerFid);
  const char * c_proxy = env->GetStringUTFChars(jproxy, JNI_FALSE);
  proxyServer = c_proxy;
  env->ReleaseStringUTFChars(jproxy, c_proxy);
  env->DeleteLocalRef(jproxy);

  //decryptionMode
  jstring jdecryptionMode = (jstring)env->GetObjectField(jni_recordingConfig, decryptionModeFid);
  const char * c_decryptionMode = env->GetStringUTFChars(jdecryptionMode, JNI_FALSE);
  decryptionMode = c_decryptionMode;
  env->ReleaseStringUTFChars(jdecryptionMode, c_decryptionMode);
  env->DeleteLocalRef(jdecryptionMode);
  //lowUdpPort
  lowUdpPort = (int)env->GetIntField(jni_recordingConfig, lowUdpPortFid); 
  //highUdpPort
  highUdpPort = (int)env->GetIntField(jni_recordingConfig, highUdpPortFid); 
  //captureInterval
  captureInterval = (int)env->GetIntField(jni_recordingConfig, captureIntervalFid); 
  //triggerMode
  triggerMode = (int)env->GetIntField(jni_recordingConfig, triggerModeFid); 
  //paser parameters end
  env->DeleteLocalRef(jni_recordingConfig);

  agora::recording::RecordingConfig config;
  jniproxy::AgoraJniProxySdk jniRecorder;
  //important! Get a reference to this object's class

  jclass thisJcInstance = NULL;
  thisJcInstance = env->GetObjectClass(thisObj);
  if(!thisJcInstance) {
    cout<<"Jni cannot get java instance, error!!!";
    return JNI_FALSE;
  }
  jniRecorder.setJcAgoraJavaRecording(thisJcInstance);
  jniRecorder.setJobAgoraJavaRecording(thisObj);
  jniRecorder.initialize();

  config.idleLimitSec = idleLimitSec;
  config.channelProfile = static_cast<agora::linuxsdk::CHANNEL_PROFILE_TYPE>(channelProfile);

  config.isVideoOnly = isVideoOnly;
  config.isAudioOnly = isAudioOnly;
  config.isMixingEnabled = isMixingEnabled;
  config.mixResolution = (isMixingEnabled && !isAudioOnly)? const_cast<char*>(mixResolution.c_str()):NULL;
  config.mixedVideoAudio = mixedVideoAudio;

  config.appliteDir = const_cast<char*>(applitePath.c_str());	
  config.recordFileRootDir = const_cast<char*>(recordFileRootDir.c_str());
  config.cfgFilePath = const_cast<char*>(cfgFilePath.c_str());

  config.secret = secret.empty()? NULL:const_cast<char*>(secret.c_str());
  config.decryptionMode = decryptionMode.empty()? NULL:const_cast<char*>(decryptionMode.c_str());

  config.lowUdpPort = lowUdpPort;
  config.highUdpPort = highUdpPort;
  config.captureInterval = captureInterval;

  config.decodeAudio = static_cast<agora::linuxsdk::AUDIO_FORMAT_TYPE>(getAudioFrame);
  config.decodeVideo = static_cast<agora::linuxsdk::VIDEO_FORMAT_TYPE>(getVideoFrame);
  config.streamType = static_cast<agora::linuxsdk::REMOTE_VIDEO_STREAM_TYPE>(streamType);
  config.triggerMode = static_cast<agora::linuxsdk::TRIGGER_MODE_TYPE>(triggerMode);
  config.lang = static_cast<agora::linuxsdk::LANGUAGE_TYPE>(lang);
  config.proxyServer = proxyServer.empty()? NULL:const_cast<char*>(proxyServer.c_str());

  cout<<"appId:"<<appId<<",uid:"<<uid<<",channelKey:"<<channelKey<<",channelName:"<<channelName<<",applitePath:"
    <<applitePath<<",channelProfile:"<<channelProfile<<",getAudioFrame:"
    <<getAudioFrame<<",getVideoFrame:"<<getVideoFrame<<endl<<",idle:"<<idleLimitSec<<",lowUdpPort:"<<lowUdpPort<<",highUdpPort:"<<highUdpPort
    <<",captureInterval:"<<captureInterval<<",mixedVideoAudio:"<<mixedVideoAudio<<",mixResolution:"<<mixResolution<<",isVideoOnly:"<<isVideoOnly
    <<",isAudioOnly:"<<isAudioOnly<<",isMixingEnabled:"<<isMixingEnabled<<",triggerMode:"<<triggerMode<<",proxyServer"<<proxyServer<<endl;

  /**
   * change log_config Facility per your specific purpose like agora::base::LOCAL5_LOG_FCLT
   * Default:USER_LOG_FCLT. 
   * agora::base::log_config::setFacility(agora::base::LOCAL5_LOG_FCLT);  
   */
  if(!jniRecorder.createChannel(appId, channelKey, channelName, uid, config))
  {
    cerr << "Failed to create agora channel: " << channelName <<endl;
    return JNI_FALSE;
  }
  //tell java this para pointer
  jlong nativeObjectRef = jlong(&jniRecorder);
  //find java callback function and set this value
  jmethodID jmid = env->GetMethodID(thisJcInstance, "nativeObjectRef", LONG_PARA_VOID_RETURN);
  if(!jmid) {
    cout << "cannot find nativeObjectRef method " <<endl;
    return JNI_FALSE;
  }
  env->CallIntMethod(thisObj, jmid, nativeObjectRef);
  std::string recordingDir = jniRecorder.getRecorderProperties()->storageDir;
  cout<<"Recording directory is "<<jniRecorder.getRecorderProperties()->storageDir<<endl;
  jniRecorder.setJavaRecordingPath(env, recordingDir);
  while (!jniRecorder.stopped() && !g_bSignalStop) {
    usleep(1*1000*1000); //1s
  }
  if (g_bSignalStop) {
    jniRecorder.leaveChannel();
    jniRecorder.release();
    cout<<"jni layer stopped!";
  }
  jniRecorder.stopJavaProc(env);
  cout<<"Java_AgoraJavaRecording_createChannel  end"<<endl;
  return JNI_TRUE;
}

#ifdef __cplusplus
}
#endif
