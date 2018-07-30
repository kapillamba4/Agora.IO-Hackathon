#ifndef _IAGORA_LINUXSDKCOMMON_H_
#define _IAGORA_LINUXSDKCOMMON_H_

#include <cstdint>
#include <string>

namespace agora {
namespace linuxsdk {

class IEngine{
    virtual ~IEngine() {};

};

class IEngineConfig {
    virtual ~IEngineConfig() {};
};

typedef unsigned char uchar_t;
typedef unsigned int uint_t;
typedef unsigned int uid_t;
typedef uint64_t u64_t;

enum ERROR_CODE_TYPE {
    ERR_OK = 0,
    //1~1000
    ERR_FAILED = 1,
    ERR_INVALID_ARGUMENT = 2,
    ERR_INTERNAL_FAILED = 3,
};

enum STAT_CODE_TYPE {
    STAT_OK = 0,
    STAT_ERR_FROM_ENGINE = 1,
    STAT_ERR_ARS_JOIN_CHANNEL = 2,
    STAT_ERR_CREATE_PROCESS = 3,
    STAT_ERR_MIXED_INVALID_VIDEO_PARAM = 4,
    STAT_ERR_NULL_POINTER = 5,
    STAT_ERR_PROXY_SERVER_INVALID_PARAM = 6,

    STAT_POLL_ERR = 0x8,
    STAT_POLL_HANG_UP = 0x10,
    STAT_POLL_NVAL = 0x20,
};

enum LEAVE_PATH_CODE {
    LEAVE_CODE_INIT = 0, 
    LEAVE_CODE_SIG = 1<<1,
    LEAVE_CODE_NO_USERS = 1<<2,
    LEAVE_CODE_TIMER_CATCH = 1<<3,
    LEAVE_CODE_CLIENT_LEAVE = 1 << 4,
};


enum WARN_CODE_TYPE {
    WARN_NO_AVAILABLE_CHANNEL = 103,
    WARN_LOOKUP_CHANNEL_TIMEOUT = 104,
    WARN_LOOKUP_CHANNEL_REJECTED = 105,
    WARN_OPEN_CHANNEL_TIMEOUT = 106,
    WARN_OPEN_CHANNEL_REJECTED = 107,
};

enum CHANNEL_PROFILE_TYPE
{
    CHANNEL_PROFILE_COMMUNICATION = 0,
    CHANNEL_PROFILE_LIVE_BROADCASTING = 1,
};

enum USER_OFFLINE_REASON_TYPE
{
    USER_OFFLINE_QUIT = 0,
    USER_OFFLINE_DROPPED = 1,
    USER_OFFLINE_BECOME_AUDIENCE = 2,
};

enum REMOTE_VIDEO_STREAM_TYPE
{
    REMOTE_VIDEO_STREAM_HIGH = 0,
    REMOTE_VIDEO_STREAM_LOW = 1,
};

enum VIDEO_FORMAT_TYPE {
    VIDEO_FORMAT_DEFAULT_TYPE = 0,
    VIDEO_FORMAT_H264_FRAME_TYPE = 1,
    VIDEO_FORMAT_YUV_FRAME_TYPE = 2,
    VIDEO_FORMAT_JPG_FRAME_TYPE = 3,
    VIDEO_FORMAT_JPG_FILE_TYPE = 4,
    VIDEO_FORMAT_JPG_VIDEO_FILE_TYPE = 5,
};

enum AUDIO_FORMAT_TYPE {
    AUDIO_FORMAT_DEFAULT_TYPE = 0,
    AUDIO_FORMAT_AAC_FRAME_TYPE = 1,
    AUDIO_FORMAT_PCM_FRAME_TYPE = 2,
    AUDIO_FORMAT_MIXED_PCM_FRAME_TYPE = 3,
};

enum AUDIO_FRAME_TYPE {
    AUDIO_FRAME_RAW_PCM = 0,
    AUDIO_FRAME_AAC = 1
};

enum MEMORY_TYPE {
    STACK_MEM_TYPE = 0,
    HEAP_MEM_TYPE = 1
};

enum VIDEO_FRAME_TYPE {
    VIDEO_FRAME_RAW_YUV = 0,
    VIDEO_FRAME_H264 = 1,
    VIDEO_FRAME_JPG = 2,
};

enum SERVICE_MODE {
    RECORDING_MODE = 0,//down stream
    SERVER_MODE = 1,//up-down stream
    IOT_MODE = 2,//up-down stream
};

enum TRIGGER_MODE_TYPE {
    AUTOMATICALLY_MODE = 0,
    MANUALLY_MODE = 1
};
enum LANGUAGE_TYPE {
    CPP_LANG = 0,
    JAVA_LANG = 1
};
class AudioPcmFrame {
    public:
    AudioPcmFrame(u64_t frame_ms, uint_t sample_rates, uint_t samples);
    ~AudioPcmFrame();
    public:
    u64_t frame_ms_;
    uint_t channels_; // 1
    uint_t sample_bits_; // 16
    uint_t sample_rates_; // 8k, 16k, 32k
    uint_t samples_;

    const uchar_t *pcmBuf_;
    uint_t pcmBufSize_;
};

class AudioAacFrame {
    public:
    explicit AudioAacFrame(u64_t frame_ms);
    ~AudioAacFrame();

    const uchar_t *aacBuf_;
    u64_t frame_ms_;
    uint_t aacBufSize_;

};

struct AudioFrame {
    AUDIO_FRAME_TYPE type;
    union {
        AudioPcmFrame *pcm;
        AudioAacFrame *aac;
    } frame;

    AudioFrame();
    ~AudioFrame();

    MEMORY_TYPE mType;
};

class VideoYuvFrame {
    public:
    VideoYuvFrame(u64_t frame_ms, uint_t width, uint_t height, uint_t ystride,
            uint_t ustride, uint_t vstride);
    ~VideoYuvFrame();

    u64_t frame_ms_;

    const uchar_t *ybuf_;
    const uchar_t *ubuf_;
    const uchar_t *vbuf_;

    uint_t width_;
    uint_t height_;

    uint_t ystride_;
    uint_t ustride_;
    uint_t vstride_;

    //all
    const uchar_t *buf_;
    uint_t bufSize_;
};

struct VideoH264Frame {
    public:
    VideoH264Frame():
        frame_ms_(0),
        frame_num_(0),
        buf_(NULL),
        bufSize_(0)
    {}

    ~VideoH264Frame(){}
    u64_t frame_ms_;
    uint_t frame_num_;

    //all
    const uchar_t *buf_;
    uint_t bufSize_;
};

struct VideoJpgFrame {
    public:
    VideoJpgFrame():
        frame_ms_(0),
        buf_(NULL),
        bufSize_(0){}

   ~VideoJpgFrame() {}
    u64_t frame_ms_;

    //all
    const uchar_t *buf_;
    uint_t bufSize_;
};

struct VideoFrame {
    VIDEO_FRAME_TYPE type;
    union {
        VideoYuvFrame *yuv;
        VideoH264Frame *h264;
        VideoJpgFrame *jpg;
    } frame;

    int rotation_; // 0, 90, 180, 270
    VideoFrame();
    ~VideoFrame();

    MEMORY_TYPE mType;
};

typedef struct VideoMixingLayout
{
    struct Region {
        uid_t uid;
        double x;//[0,1]
        double y;//[0,1]
        double width;//[0,1]
        double height;//[0,1]
        int zOrder; //optional, [0, 100] //0 (default): bottom most, 100: top most

        //  Optional
        //  [0, 1.0] where 0 denotes throughly transparent, 1.0 opaque
        double alpha;

        int renderMode;//RENDER_MODE_HIDDEN: Crop, RENDER_MODE_FIT: Zoom to fit
        Region()
            :uid(0)
             , x(0)
             , y(0)
             , width(0)
             , height(0)
             , zOrder(0)
             , alpha(1.0)
             , renderMode(1)
        {}

    };
    int canvasWidth;
    int canvasHeight;
    const char* backgroundColor;//e.g. "#C0C0C0" in RGB
    uint32_t regionCount;
    const Region* regions;
    const char* appData;
    int appDataLength;
    VideoMixingLayout()
        :canvasWidth(0)
         , canvasHeight(0)
         , backgroundColor(NULL)
         , regionCount(0)
         , regions(NULL)
         , appData(NULL)
         , appDataLength(0)
    {}
} VideoMixingLayout;

typedef struct UserJoinInfos {
    const char* storageDir;
    //new attached info add below

    UserJoinInfos():
        storageDir(NULL)
    {}
}UserJoinInfos;


}
}

#endif
