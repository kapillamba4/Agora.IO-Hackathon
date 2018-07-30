#include "../../agorasdk/AgoraSdk.h"
#include "string.h"
#include "iostream"
#include "jni.h"

#include "jni/commonJniDef.h"

using namespace std;
namespace jniproxy
{

class AgoraJniProxySdk : public agora::AgoraSdk {
public:
  typedef void (*callback_t ) (void);
  AgoraJniProxySdk();
  ~AgoraJniProxySdk(); 
  void initialize();
  virtual void onError(int error, agora::linuxsdk::STAT_CODE_TYPE stat_code);
  virtual void onWarning(int warn);

  virtual void onJoinChannelSuccess(const char * channelId, agora::linuxsdk::uid_t uid) {
    cout<<"AgoraJniProxySdk onJoinChannelSuccess"<<endl;
  }
  virtual void onLeaveChannel(agora::linuxsdk::LEAVE_PATH_CODE code);
  virtual void onUserJoined(agora::linuxsdk::uid_t uid, agora::linuxsdk::UserJoinInfos &infos);
  virtual void onUserOffline(agora::linuxsdk::uid_t uid, agora::linuxsdk::USER_OFFLINE_REASON_TYPE reason);
  virtual void audioFrameReceived(unsigned int uid, const agora::linuxsdk::AudioFrame *frame) const;
  virtual void videoFrameReceived(unsigned int uid, const agora::linuxsdk::VideoFrame *frame) const;

  void setJobAgoraJavaRecording(jobject job){
    mJavaAgoraJavaRecordingObject = job;
  }
  void setJcAgoraJavaRecording(jclass jc){
    mJavaAgoraJavaRecordingClass = jc;
  }
  void stopJavaProc(JNIEnv* env);
  void setJavaRecordingPath(JNIEnv* env, std::string& storeDir);

  jobject newJObject(JNIEnv* env, jclass jcls, jmethodID jmtd) const;

  jclass newGlobalJClass(JNIEnv* env, const char* className);
  jobject newGlobalJObject(JNIEnv* env, jclass jc, const char* signature);
  //cache field ids & method ids
  void cacheJavaCBFuncMethodIDs4Video(JNIEnv* env, const char* className);
  //yuv,h264,jpg init method
  void cacheJavaVideoFrameInitMethodIDs(JNIEnv* env, const char* className);
  void cacheAudioPcmFrame(JNIEnv* env);
  void cacheJavaObject(JNIEnv* env);
  //void cacheJavaCBFuncMethodIDs4YUV(JNIEnv* env, const char* className);
  //void cacheJavaCBFuncMethodIDs4YUV(JNIEnv* env, const char* className);
  //void cacheJavaCBFuncMethodIDs4YUV(JNIEnv* env, const char* className);
  //void cacheJavaCBFuncMethodIDs4YUV(JNIEnv* env, const char* className);
  jmethodID safeGetMethodID(JNIEnv* env, jclass clazz, const char* name, const char* sig) const;
  jfieldID safeGetFieldID(JNIEnv* env, jclass clazz, const char* name, const char* sig) const;
  jfieldID safeGetFieldID2(JNIEnv* env, jclass clazz, const char* name, const char* sig) const;
  jobject newGlobalJObject2(JNIEnv* env, jclass jc, jmethodID initMid) const;

public:
   
private:
  void initJavaObjects(JNIEnv* env, bool init);
  int staticInitCBFuncMid(JNIEnv* env, jclass jclazz);
  int staticInitVideoYuvFrameFid(JNIEnv* env, jclass jclazz);
  
  template<typename T1, typename T2 >
  int staticInitCommonFrameFid(JNIEnv* env, jclass clazz, GETID_TYPE type, T1& src, T2& dest);

private:
  static jmethodID mJavaRecvVideoMtd;
  static jmethodID mJavaRecvAudioMtd;
  static jmethodID mJavaVideoFrameInitMtd;
  //yuv jpg h264 init method
  static jmethodID mJavaVideoYuvFrameInitMtd;
  static jmethodID mJavaVideoJpgFrameInitMtd;
  static jmethodID mJavaVideoH264FrameInitMtd;

  static jmethodID m_CBObjectMethodIDs[MID_CBOBJECT_NUM];
  static jfieldID m_VideoYuvFrameFieldIDs[FID_YUVNUM];
  static jfieldID m_VideoH264FrameFieldIDs[FID_H264NUM];
  static jfieldID m_VideoJpgFrameFieldIDs[FID_JPGNUM];
  
  static jfieldID m_AudioFrameFieldIDs[FID_AF_NUM];
  static jfieldID m_AudioPcmFrameFieldIDs[FID_PCMNUM];
  static jfieldID m_AudioAacFrameFieldIDs[FID_AACNUM];
  
private:
  //audio
  bool fillJAudioFrameByFields(JNIEnv* env, const agora::linuxsdk::AudioFrame*& frame, jclass jcAudioFrame, jobject& jobAudioFrame) const;
  //pcm
  bool fillAudioPcmFrame(JNIEnv* env, const agora::linuxsdk::AudioFrame*& frame,jclass& jcAudioFrame, jobject& jobAudioFrame) const;
  bool fillPcmAllFields(JNIEnv* env, jobject& job, jclass& jc, const agora::linuxsdk::AudioFrame*& frame) const;
  //aac
  bool fillAudioAacFrame(JNIEnv* env, const agora::linuxsdk::AudioFrame*& frame,jclass& jcAudioFrame, jobject& jobAudioFrame) const;
  bool fillAacAllFields(JNIEnv* env, jobject& job, jclass& jc, const agora::linuxsdk::AudioFrame*& frame) const;

  //video
  bool fillVideoFrameByFields(JNIEnv* env, const agora::linuxsdk::VideoFrame*& frame, jclass jcVideoFrame, jobject jobVideoFrame) const;
  bool fillVideoOfYUV(JNIEnv* env, const agora::linuxsdk::VideoFrame*& frame, jclass& jcVideoFrame, jobject& jobVideoFrame) const;
  bool fillVideoOfJPG(JNIEnv* env, const agora::linuxsdk::VideoFrame*& frame, jclass& jcVideoFrame, jobject& jobVideoFrame) const;
  bool fillVideoOfH264(JNIEnv* env, const agora::linuxsdk::VideoFrame*& frame, jclass& jcVideoFrame, jobject& jobVideoFrame) const;
private:
  std::string m_logdir;
private:
  //define java object
  //video
  jclass mJavaVideoFrameClass;
  jobject mJavaVideoFrameObject;
  jclass mJavaVideoYuvFrameClass;
  jobject mJavaVideoYuvFrameObject;
  
  jclass mJavaVideoJpgFrameClass;
  jobject mJavaVideoJpgFrameObject;
  
  jclass mJavaVideoH264FrameClass;
  jobject mJavaVideoH264FrameObject;
  
  //type
  jclass mJavaVideoFrameTypeClass;
  jobject mJavaVideoFrameTypeObject;
  jmethodID mJavaVideoFrameTypeInitMtd; 
  jfieldID mJavaVideoFrameTypeTypeFid;

  //audio
  jclass mJavaAudioFrameClass;
  jobject mJavaAudioFrameObject;

  //pcm field
  jfieldID mJavaAudioAacFrameFid;

  //pcm
  jclass mJavaAudioPcmFrameClass;
  jmethodID mJavaAudioPcmFrameInitMtd;
  jobject mJavaAudioPcmFrameObject;

  //aac
  jclass mJavaAudioAacFrameClass;
  jmethodID mJavaAudioAacFrameInitMtd;
  jobject mJavaAudioAacFrameObject;

  //AudioFrameType
  jclass mJavaAudioFrameTypeClass;
  jmethodID mJavaAudioFrameTypeInitMtd;
  jobject mJavaAudioFrameTypeObject;
  jfieldID mJavaAudioFrameTypeFid;
  jfieldID mJavaAudioFrameTypeTypeFid;

  jfieldID mJavaVideoFrameYuvFid;
  jfieldID mJavaVideoFrameH264Fid;
  jfieldID mJavaVideoFrameJpgFid;
  jmethodID mJavaAudioFrameInitMtd;

private:
  jclass mJavaAgoraJavaRecordingClass;
  jobject mJavaAgoraJavaRecordingObject;
};

}//endnamespace
