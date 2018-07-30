#ifndef _IAGORA_RECORDINGENGINE_H_
#define _IAGORA_RECORDINGENGINE_H_
#include "IAgoraLinuxSdkCommon.h"

namespace agora {
namespace recording {

class IRecordingEngineEventHandler {

public:
    virtual ~IRecordingEngineEventHandler() {}
    
    /**
     *  Callback when an error occurred during the runtime of recording engine
     *
     *
     *  @param error        Error code
     *  @param stat_code    state code
     *
     */
    virtual void onError(int error, agora::linuxsdk::STAT_CODE_TYPE stat_code) = 0;

    /**
     *  Callback when an warning occurred during the runtime of recording engine
     *
     *
     *  @param warn         warning code
     *
     */
    virtual void onWarning(int warn) = 0;
   
    /**
     *  Callback when the user hase successfully joined the specified channel
     *
     *
     *  @param channelID    channel ID 
     *  @param uid          User ID
     *
     */
    virtual void onJoinChannelSuccess(const char * channelId, uid_t uid) = 0;
   
    /**
     *  Callback when recording application successfully left the channel
     *
     *
     *  @param code        leave path code
     *
     */
    virtual void onLeaveChannel(agora::linuxsdk::LEAVE_PATH_CODE code) = 0;

    /**
     *  Callback when another user successfully joined the channel
     *
     *
     *  @param uid          user ID
     *  @param infos        user join information    
     *
     */
    virtual void onUserJoined(uid_t uid, agora::linuxsdk::UserJoinInfos &infos) = 0;
   
    /**
     *  Callback when a user left the channel or gone offline
     *
     *
     *  @param uid          user ID
     *  @param reason       offline reason    
     *
     */
    virtual void onUserOffline(uid_t uid, agora::linuxsdk::USER_OFFLINE_REASON_TYPE reason) = 0;

    /**
     *  Callback when received a audio frame
     *
     *
     *  @param uid          user ID
     *  @param frame        pointer to received audio frame    
     *
     */
    virtual void audioFrameReceived(unsigned int uid, const agora::linuxsdk::AudioFrame *frame) const = 0;

    /**
     *  Callback when received a video frame
     *
     *
     *  @param uid          user ID
     *  @param frame        pointer to received video frame    
     *
     */
    virtual void videoFrameReceived(unsigned int uid, const agora::linuxsdk::VideoFrame *frame) const = 0;
};

 

typedef struct RecordingConfig {
    bool isAudioOnly;
    bool isVideoOnly;
    bool isMixingEnabled;
    bool mixedVideoAudio;
    char * mixResolution;
    char * decryptionMode;
    char * secret;
    char * appliteDir;
    char * recordFileRootDir;
    char * cfgFilePath;
    agora::linuxsdk::VIDEO_FORMAT_TYPE decodeVideo;
    agora::linuxsdk::AUDIO_FORMAT_TYPE decodeAudio; 
    int lowUdpPort;
    int highUdpPort;  
    int idleLimitSec;
    int captureInterval;
    agora::linuxsdk::CHANNEL_PROFILE_TYPE channelProfile;
    agora::linuxsdk::REMOTE_VIDEO_STREAM_TYPE streamType;
    agora::linuxsdk::TRIGGER_MODE_TYPE triggerMode;
    agora::linuxsdk::LANGUAGE_TYPE lang;
    char * proxyServer;

    RecordingConfig(): channelProfile(agora::linuxsdk::CHANNEL_PROFILE_COMMUNICATION),
        isAudioOnly(false),
        isVideoOnly(false),
        isMixingEnabled(false),
        mixResolution(NULL),
        decryptionMode(NULL),
        secret(NULL),
        idleLimitSec(300),
        appliteDir(NULL),
        recordFileRootDir(NULL),
        cfgFilePath(NULL),
        lowUdpPort(0),
        highUdpPort(0),
        captureInterval(5),
        decodeAudio(agora::linuxsdk::AUDIO_FORMAT_DEFAULT_TYPE),
        decodeVideo(agora::linuxsdk::VIDEO_FORMAT_DEFAULT_TYPE),
        mixedVideoAudio(false),
        streamType(agora::linuxsdk::REMOTE_VIDEO_STREAM_HIGH),
        triggerMode(agora::linuxsdk::AUTOMATICALLY_MODE),
        lang(agora::linuxsdk::CPP_LANG),
        proxyServer(NULL)
    {}

    virtual ~RecordingConfig() {}
} RecordingConfig;

typedef struct RecordingEngineProperties {
    char* storageDir;
    RecordingEngineProperties(): storageDir(NULL)
                          {}
}RecordingEngineProperties;

class IRecordingEngine {
public:

    /**
     *  create a new recording engine instance
     *
     *  @param appId        The App ID issued to the application developers by Agora.io.
     *  @param eventHandler the callback interface
     *
     *  @return a recording engine instance pointer
     */
    static IRecordingEngine* createAgoraRecordingEngine(const char * appId, IRecordingEngineEventHandler *eventHandler);

    virtual ~IRecordingEngine() {}

    /**
     *  This method lets the recording engine join a channel, and start recording
     *
     *  @param channelKey This parameter is optional if the user uses a static key, or App ID. In this case, pass NULL as the parameter value. More details refer to http://docs-origin.agora.io/en/user_guide/Component_and_Others/Dynamic_Key_User_Guide.html
     *  @param channelId  A string providing the unique channel id for the AgoraRTC session
     *  @param uid        The uid of recording client
     *  @param config     The config of current recording
     *
     *  @return 0: Method call succeeded. <0: Method call failed.
     */
    virtual int joinChannel(const char * channelKey, const char *channelId, uid_t uid, const RecordingConfig &config) = 0;

    /**
     *  set the layout of video mixing
     *
     *  @param layout layout setting
     *
     *  @return 0: Method call succeeded. <0: Method call failed.
     */
    virtual int setVideoMixingLayout(const agora::linuxsdk::VideoMixingLayout &layout) = 0;

    /**
     *  Stop recording
     *
     *  @return 0: Method call succeeded. <0: Method call failed.
     */
    virtual int leaveChannel() = 0;

    /**
     *  release recording engine
     *
     *  @return 0: Method call succeeded. <0: Method call failed.
     */
    virtual int release() = 0;

    /**
     * Get recording properties
     */
    virtual const RecordingEngineProperties* getProperties() = 0;

    /**
     *  start service under manually trigger mode
     *
     *  @return 0: Method call succeeded. <0: Method call failed.
     */
    virtual int startService() = 0;

    /**
     *  start service under manually trigger mode
     *
     *  @return 0: Method call succeeded. <0: Method call failed.
     */
    virtual int stopService() = 0;
};

}
}

#endif
