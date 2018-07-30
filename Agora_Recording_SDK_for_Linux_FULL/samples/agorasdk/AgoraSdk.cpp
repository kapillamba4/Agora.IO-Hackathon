#include <csignal>
#include <cstdint>
#include <iostream>
#include <sstream> 
#include <string>
#include <vector>
#include <algorithm>

#include "IAgoraLinuxSdkCommon.h"
#include "IAgoraRecordingEngine.h"
#include "AgoraSdk.h"

#include "base/atomic.h"
#include "base/log.h" 
#include "base/opt_parser.h" 
namespace agora {

AgoraSdk::AgoraSdk(): IRecordingEngineEventHandler() {
  m_engine = NULL;
  m_stopped.store(false);
  m_storage_dir = "./";
}

AgoraSdk::~AgoraSdk() {
  if (m_engine) {
    m_engine->release();
  }
}

bool AgoraSdk::stopped() const {
  return m_stopped;
}

bool AgoraSdk::release() {
  if (m_engine) {
    m_engine->release();
    m_engine = NULL;
  }

  return true;
}

bool AgoraSdk::createChannel(const string &appid, const string &channelKey, const string &name,
    uint32_t uid, 
    agora::recording::RecordingConfig &config) 
{
    if ((m_engine = agora::recording::IRecordingEngine::createAgoraRecordingEngine(appid.c_str(), this)) == NULL)
        return false;

    if(linuxsdk::ERR_OK != m_engine->joinChannel(channelKey.c_str(), name.c_str(), uid, config))
        return false;

    m_config = config;
    return true;
}

bool AgoraSdk::leaveChannel() {
  if (m_engine) {
    m_engine->leaveChannel();
    m_stopped = true;
  }

  return true;
}

int AgoraSdk::startService() {
  if (m_engine) 
    return m_engine->startService();

  return 1;
}

int AgoraSdk::stopService() {
  if (m_engine) 
    return m_engine->stopService();

  return 1;
}

//Customize the layout of video under video mixing model
int AgoraSdk::setVideoMixLayout()
{
    recording::RecordingConfig *pConfig = getConfigInfo();
    size_t max_peers = pConfig->channelProfile == linuxsdk::CHANNEL_PROFILE_COMMUNICATION ? 7:17;
    if(!m_mixRes.m_videoMix) return 0;

    LOG_DIR(m_logdir.c_str(), INFO, "setVideoMixLayout: user size: %d, permitted max_peers:%d", m_peers.size(), max_peers);

    agora::linuxsdk::VideoMixingLayout layout;
    layout.canvasWidth = m_mixRes.m_width;
    layout.canvasHeight = m_mixRes.m_height;
    layout.backgroundColor = "#23b9dc";

    layout.regionCount = static_cast<uint32_t>(m_peers.size());

    if (!m_peers.empty()) {
        LOG_DIR(m_logdir.c_str(), INFO, "setVideoMixLayout: peers not empty");
        agora::linuxsdk::VideoMixingLayout::Region * regionList = new agora::linuxsdk::VideoMixingLayout::Region[m_peers.size()];

        regionList[0].uid = m_peers[0];
        regionList[0].x = 0.f;
        regionList[0].y = 0.f;
        regionList[0].width = 1.f;
        regionList[0].height = 1.f;
        regionList[0].zOrder = 0;
        regionList[0].alpha = 1.f;
        regionList[0].renderMode = 0;

        LOG_DIR(m_logdir.c_str(), INFO, "region 0 uid: %u, x: %f, y: %f, width: %f, height: %f, zOrder: %d, alpha: %f", regionList[0].uid, regionList[0].x, regionList[0].y, regionList[0].width, regionList[0].height, regionList[0].zOrder, regionList[0].alpha);


        float canvasWidth = static_cast<float>(m_mixRes.m_width);
        float canvasHeight = static_cast<float>(m_mixRes.m_height);

        float viewWidth = 0.235f;
        float viewHEdge = 0.012f;
        float viewHeight = viewWidth * (canvasWidth/canvasHeight);
        float viewVEdge = viewHEdge * (canvasWidth/canvasHeight);

        for (size_t i=1; i<m_peers.size(); i++) {
            if (i >= max_peers)
                break;

            regionList[i].uid = m_peers[i];

            float xIndex = static_cast<float>((i-1) % 4);
            float yIndex = static_cast<float>((i-1) / 4);
            regionList[i].x = xIndex * (viewWidth + viewHEdge) + viewHEdge;
            regionList[i].y = 1 - (yIndex + 1) * (viewHeight + viewVEdge);
            regionList[i].width = viewWidth;
            regionList[i].height = viewHeight;
            regionList[i].zOrder = 0;
            regionList[i].alpha = static_cast<double>(i + 1);
            regionList[i].renderMode = 0;
        }

        layout.regions = regionList;
//    LOG_DIR(m_logdir.c_str(), INFO, "region 0 uid: %d, x: %f, y: %f, width: %f, height: %f, zOrder: %d, alpha: %f", regionList[0].uid, regionList[0].x, regionList[0].y, regionList[0].width, regionList[0].height, regionList[0].zOrder, regionList[0].alpha);
    }
    else {
        layout.regions = NULL;
    }

    return setVideoMixingLayout(layout);
}

int AgoraSdk::setVideoMixingLayout(const agora::linuxsdk::VideoMixingLayout &layout)
{
   int result = -agora::linuxsdk::ERR_INTERNAL_FAILED;
   if(m_engine)
      result = m_engine->setVideoMixingLayout(layout);
   return result;
}

const agora::recording::RecordingEngineProperties* AgoraSdk::getRecorderProperties(){
    return m_engine->getProperties();
}

void AgoraSdk::onErrorImpl(int error, agora::linuxsdk::STAT_CODE_TYPE stat_code) {
    cerr << "Error: " << error <<",with stat_code:"<< stat_code << endl;
    leaveChannel();
}

void AgoraSdk::onWarningImpl(int warn) {
    cerr << "warn: " << warn << endl;
    //  leaveChannel();
}

void AgoraSdk::onJoinChannelSuccessImpl(const char * channelId, agora::linuxsdk::uid_t uid) {
    cout << "join channel Id: " << channelId << ", with uid: " << uid << endl;
}

void AgoraSdk::onLeaveChannelImpl(agora::linuxsdk::LEAVE_PATH_CODE code) {
    cout << "leave channel with code:" << code << endl;
}

void AgoraSdk::onUserJoinedImpl(unsigned uid, agora::linuxsdk::UserJoinInfos &infos) {
    cout << "User " << uid << " joined, RecordingDir:" << (infos.storageDir? infos.storageDir:"NULL") <<endl;
    if(infos.storageDir)
    {
        m_storage_dir = std::string(infos.storageDir);
        m_logdir = m_storage_dir;
    }

    m_peers.push_back(uid);
    
    //When the user joined, we can re-layout the canvas
    setVideoMixLayout();
}


void AgoraSdk::onUserOfflineImpl(unsigned uid, agora::linuxsdk::USER_OFFLINE_REASON_TYPE reason) {
    cout << "User " << uid << " offline, reason: " << reason << endl;
    m_peers.erase(std::remove(m_peers.begin(), m_peers.end(), uid), m_peers.end());
    
    //When the user is offline, we can re-layout the canvas
    setVideoMixLayout();
}

void AgoraSdk::audioFrameReceivedImpl(unsigned int uid, const agora::linuxsdk::AudioFrame *pframe) const 
{
  char uidbuf[65];
  snprintf(uidbuf, sizeof(uidbuf),"%u", uid);
  std::string info_name = m_storage_dir + std::string(uidbuf) /*+ timestamp_per_join_*/;

  const uint8_t* data = NULL; 
  uint32_t size = 0;
  if (pframe->type == agora::linuxsdk::AUDIO_FRAME_RAW_PCM) {
    info_name += ".pcm";  

    agora::linuxsdk::AudioPcmFrame *f = pframe->frame.pcm;
    data = f->pcmBuf_;
    size = f->pcmBufSize_;

    cout << "User " << uid << ", received a raw PCM frame ,channels:" << f->channels_ <<endl;

  } else if (pframe->type == agora::linuxsdk::AUDIO_FRAME_AAC) {
    info_name += ".aac";

    cout << "User " << uid << ", received an AAC frame" << endl;

    agora::linuxsdk::AudioAacFrame *f = pframe->frame.aac;
    data = f->aacBuf_;
    size = f->aacBufSize_;
  }

  FILE *fp = fopen(info_name.c_str(), "a+b");
  if(fp == NULL) {
      cout << "failed to open: " << info_name;
      cout<< " ";
      cout << "errno: " << errno;
      cout<< endl;
      return;
  }

  ::fwrite(data, 1, size, fp);
  ::fclose(fp);
}

void AgoraSdk::videoFrameReceivedImpl(unsigned int uid, const agora::linuxsdk::VideoFrame *pframe) const {
  char uidbuf[65];
  snprintf(uidbuf, sizeof(uidbuf),"%u", uid);
  const char * suffix=".vtmp";
  if (pframe->type == agora::linuxsdk::VIDEO_FRAME_RAW_YUV) {
    agora::linuxsdk::VideoYuvFrame *f = pframe->frame.yuv;
    suffix=".yuv";

    cout << "User " << uid << ", received a yuv frame, width: "
        << f->width_ << ", height: " << f->height_ ;
    cout<<",ystride:"<<f->ystride_<< ",ustride:"<<f->ustride_<<",vstride:"<<f->vstride_;
    cout<< endl;
  } else if(pframe->type == agora::linuxsdk::VIDEO_FRAME_JPG) {
    suffix=".jpg";
    agora::linuxsdk::VideoJpgFrame *f = pframe->frame.jpg;

    cout << "User " << uid << ", received an jpg frame, timestamp: "
    << f->frame_ms_ << endl;

    struct tm date;
    time_t t = time(NULL);
    localtime_r(&t, &date);
    char timebuf[128];
    sprintf(timebuf, "%04d%02d%02d%02d%02d%02d", date.tm_year + 1900, date.tm_mon + 1, date.tm_mday, date.tm_hour, date.tm_min, date.tm_sec);
    std::string file_name = m_storage_dir + std::string(uidbuf) + "_" + std::string(timebuf) + suffix;
    FILE *fp = fopen(file_name.c_str(), "w");
    if(fp == NULL) {
        cout << "failed to open: " << file_name;
        cout<< " ";
        cout << "errno: " << errno;
        cout<< endl;
        return;
    }

    ::fwrite(f->buf_, 1, f->bufSize_, fp);
    ::fclose(fp);
    return;
  } else {
    suffix=".h264";
    agora::linuxsdk::VideoH264Frame *f = pframe->frame.h264;

    cout << "User " << uid << ", received an h264 frame, timestamp: "
        << f->frame_ms_ << ", frame no: " << f->frame_num_ << endl;
  }

  std::string info_name = m_storage_dir + std::string(uidbuf) /*+ timestamp_per_join_ */+ std::string(suffix);
  FILE *fp = fopen(info_name.c_str(), "a+b");
  if(fp == NULL) {
        cout << "failed to open: " << info_name;
        cout<< " ";
        cout << "errno: " << errno;
        cout<< endl;
        return;
  }


  //store it as file
  if (pframe->type == agora::linuxsdk::VIDEO_FRAME_RAW_YUV) {
      agora::linuxsdk::VideoYuvFrame *f = pframe->frame.yuv;
      ::fwrite(f->buf_, 1, f->bufSize_, fp);
  }
  else {
      agora::linuxsdk::VideoH264Frame *f = pframe->frame.h264;
      ::fwrite(f->buf_, 1, f->bufSize_, fp);
  }
  ::fclose(fp);

}
}

