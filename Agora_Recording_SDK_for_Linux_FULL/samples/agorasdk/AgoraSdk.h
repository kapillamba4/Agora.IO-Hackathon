#include <csignal>
#include <cstdint>
#include <iostream>
#include <sstream> 
#include <string>
#include <vector>
#include <algorithm>

#include "IAgoraLinuxSdkCommon.h"
#include "IAgoraRecordingEngine.h"

#include "base/atomic.h"
#include "base/log.h" 
#include "base/opt_parser.h" 

namespace agora {

using std::string;
using std::cout;
using std::cerr;
using std::endl;

using agora::base::opt_parser;
using agora::linuxsdk::VideoFrame;
using agora::linuxsdk::AudioFrame;


struct MixModeSettings {
    int m_height;
    int m_width;
    bool m_videoMix;
    MixModeSettings():
        m_height(0),
        m_width(0),
        m_videoMix(false)
    {};
};


class AgoraSdk : virtual public agora::recording::IRecordingEngineEventHandler {
    public:
        AgoraSdk();
        virtual ~AgoraSdk();

        virtual bool createChannel(const string &appid, const string &channelKey, const string &name,  agora::linuxsdk::uid_t uid,
                agora::recording::RecordingConfig &config);
        virtual int setVideoMixLayout();
        virtual bool leaveChannel();
        virtual bool release();
        virtual bool stopped() const;
        virtual void updateMixModeSetting(int width, int height, bool isVideoMix) {
            m_mixRes.m_width = width;
            m_mixRes.m_height = height;
            m_mixRes.m_videoMix = isVideoMix;
        }
        virtual const agora::recording::RecordingEngineProperties* getRecorderProperties();
        virtual void updateStorageDir(const char* dir) { m_storage_dir = dir? dir:"./"; }

        virtual int startService();
        virtual int stopService();

        virtual int setVideoMixingLayout(const agora::linuxsdk::VideoMixingLayout &layout);
        virtual agora::recording::RecordingConfig* getConfigInfo() { return &m_config;}
    protected:
        virtual void onError(int error, agora::linuxsdk::STAT_CODE_TYPE stat_code) {
            onErrorImpl(error, stat_code);
        }
        virtual void onWarning(int warn) {
            onWarningImpl(warn);
        }

        virtual void onJoinChannelSuccess(const char * channelId, agora::linuxsdk::uid_t uid) {
            onJoinChannelSuccessImpl(channelId, uid);
        }
        virtual void onLeaveChannel(agora::linuxsdk::LEAVE_PATH_CODE code) {
            onLeaveChannelImpl(code);
        }

        virtual void onUserJoined(agora::linuxsdk::uid_t uid, agora::linuxsdk::UserJoinInfos &infos) {
            onUserJoinedImpl(uid, infos);
        }
        virtual void onUserOffline(agora::linuxsdk::uid_t uid, agora::linuxsdk::USER_OFFLINE_REASON_TYPE reason) {
            onUserOfflineImpl(uid, reason);
        }

        virtual void audioFrameReceived(unsigned int uid, const agora::linuxsdk::AudioFrame *frame) const {
            audioFrameReceivedImpl(uid, frame);
        }
        virtual void videoFrameReceived(unsigned int uid, const agora::linuxsdk::VideoFrame *frame) const {
            videoFrameReceivedImpl(uid, frame);
        }


    protected:
        void onErrorImpl(int error, agora::linuxsdk::STAT_CODE_TYPE stat_code);
        void onWarningImpl(int warn);

        void onJoinChannelSuccessImpl(const char * channelId, agora::linuxsdk::uid_t uid);
        void onLeaveChannelImpl(agora::linuxsdk::LEAVE_PATH_CODE code);

        void onUserJoinedImpl(agora::linuxsdk::uid_t uid, agora::linuxsdk::UserJoinInfos &infos);
        void onUserOfflineImpl(agora::linuxsdk::uid_t uid, agora::linuxsdk::USER_OFFLINE_REASON_TYPE reason);

        void audioFrameReceivedImpl(unsigned int uid, const agora::linuxsdk::AudioFrame *frame) const;
        void videoFrameReceivedImpl(unsigned int uid, const agora::linuxsdk::VideoFrame *frame) const;

    protected:
        atomic_bool_t m_stopped;
        std::vector<agora::linuxsdk::uid_t> m_peers;
        std::string m_logdir;
        std::string m_storage_dir;
        MixModeSettings m_mixRes;
        agora::recording::RecordingConfig m_config;
        agora::recording::IRecordingEngine *m_engine;
};


}
