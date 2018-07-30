
//define signatures
#define LONG_SIGNATURE "J"
#define INT_SIGNATURE "I"
#define BOOL_SIGNATURE "Z"
#define DOUBLE_SIGNATURE "D"
#define LONG_PARA_VOID_RETURN "(J)V" 
#define EMPTY_PARA_INT_RETURN "()I"
#define SG_INT "I"
#define STRING_SIGNATURE "Ljava/lang/String;"
#define BYTEARRAY "Ljava/nio/ByteBuffer;"
#define CHANNEL_PROFILE_SIGNATURE "Lio/agora/recording/common/Common$CHANNEL_PROFILE_TYPE;"
#define REMOTE_VIDEO_STREAM_SIGNATURE "Lio/agora/recording/common/Common$REMOTE_VIDEO_STREAM_TYPE;"
//-----------------Video Begin-------------------
//#define VIDEOFRAME_SIGNATURE "io/agora/recording/common/Common$VideoFrame"
#define VIDEOFRAME_H264_SIGNATURE "Lio/agora/recording/common/Common$VideoH264Frame;"
#define VIDEOFRAME_YUV_SIGNATURE "Lio/agora/recording/common/Common$VideoYuvFrame;"
#define VIDEOFRAME_JPG_SIGNATURE "Lio/agora/recording/common/Common$VideoJpgFrame;" 
#define VIDEO_FRAME_TYPE_SIGNATURE "Lio/agora/recording/common/Common$VIDEO_FRAME_TYPE;"
#define AUDIO_FORMAT_TYPE_SIGNATURE "Lio/agora/recording/common/Common$AUDIO_FORMAT_TYPE;"
#define VIDEO_FORMAT_TYPE_SIGNATURE "Lio/agora/recording/common/Common$VIDEO_FORMAT_TYPE;"
#define VIDEOMIXLAYOUT_SIGNATURE "[Lio/agora/recording/common/Common$VideoMixingLayout$Region;"
//jmethodID signature
#define SN_MTD_COMMON_INIT "(Lio/agora/recording/common/Common;)V"
//video  init jmethodID 
#define SN_MTD_VIDEO_YUV_FRAME_INIT "(Lio/agora/recording/common/Common;JIIIII)V"
#define SN_MTD_VIDEO_H264_FRAME_INIT "(Lio/agora/recording/common/Common;)V"
#define SN_MTD_VIDEO_JPG_FRAME_INIT "(Lio/agora/recording/common/Common;)V"
//class name
//VIDEO
#define CN_REP "io/agora/recording/common/RecordingEngineProperties"
#define CN_VIDEO_FRAME "io/agora/recording/common/Common$VideoFrame"
#define CN_VIDEO_YUV_FRAME "io/agora/recording/common/Common$VideoYuvFrame"
#define CN_VIDEO_JPG_FRAME "io/agora/recording/common/Common$VideoJpgFrame"
#define CN_VIDEO_H264_FRAME "io/agora/recording/common/Common$VideoH264Frame"

//video frame type clas
#define CN_VIDEO_FRAME_TYPE "io/agora/recording/common/Common$VIDEO_FRAME_TYPE"

//init jmethodID
#define SG_MTD_INIT "<init>"
#define SG_COMMON_INIT "<init>"
#define MTD_TYPE "type"

#define VIDEO_FRAME_SIGNATURE "(Lio/agora/recording/common/Common;)V"
#define MTD_INIT_VIDEO_FRAME "(Lio/agora/recording/common/Common;)V"

//#define SN_CB_FUNC_RECEIVE_VIDEOFRAME "(JLio/agora/recording/common/Common$VideoFrame;)V"
#define SN_CB_FUNC_RECEIVE_VIDEOFRAME "(JILio/agora/recording/common/Common$VideoFrame;I)V"
//declare java callback function
#define CB_FUNC_RECEIVE_VIDEOFRAME "videoFrameReceived"

#define CB_FUNC_RECEIVE_AUDIOFRAME "audioFrameReceived"

//declare filedIDs
#define FID_VIDEO_YUV_FRAME_BUF "buf"
#define FID_VIDEO_YUV_FRAME_BUFSIZE "bufSize"
#define FID_VIDEO_FRAME_YUV "yuv"
#define FID_VIDEO_FRAME_JPG "jpg"
#define FID_VIDEO_FRAME_H264 "h264"
#define FID_FRAME_MS "frame_ms"

//static java class init jmethodIDs
//static jmethodID mJavaVideoFrameInitMtd;
//static jmethodID mJavaVideoYuvFrameInitMtd;

//-----------------Video End-------------------

//-----------------Audio Begin-------------------
//class name
//Audio
#define CN_AUDIO_FRAME "io/agora/recording/common/Common$AudioFrame"
#define CN_AUDIO_FRAME_TYPE "io/agora/recording/common/Common$AUDIO_FRAME_TYPE"
#define CN_AUDIO_AAC_FRAME "io/agora/recording/common/Common$AudioAacFrame"
#define CN_AUDIO_PCM_FRAME "io/agora/recording/common/Common$AudioPcmFrame"
//#define SN_CB_FUNC_RECEIVE_AUDIOFRAME "(JLio/agora/recording/common/Common$AudioFrame;)V"
#define SN_CB_FUNC_RECEIVE_AUDIOFRAME "(JILio/agora/recording/common/Common$AudioFrame;)V"

#define SN_AUDIO_FRAME_TYPE "Lio/agora/recording/common/Common$AUDIO_FRAME_TYPE;"
#define SN_INIT_MTD_AUDIO_FRAME "(Lio/agora/recording/common/Common;JJJ)V"

#define SN_AUDIO_PCM_FRAME "Lio/agora/recording/common/Common$AudioPcmFrame;"
#define SN_AUDIO_AAC_FRAME "Lio/agora/recording/common/Common$AudioAacFrame;"

//-----------------Audio End-------------------

//common
//audio pcm 
#define FID_CHANNELS "channels"
#define FID_SAMPLE_BITS "sample_bits"
#define FID_SAMPLE_RATES "sample_rates"
#define FID_SAMPLE "samples"
#define FID_PCMBUF "pcmBuf"
#define FID_PCMBUFFERSIZE "pcmBufSize"
#define FID_PCM "pcm"
#define FID_AAC "aac"
#define FID_CHANNELNUM "frame_num"

//-----------------Callback Java method----------------

enum GETID_TYPE{
  MTDID,
  FIDID
};
struct JavaObjectMethod {
  int id;
  const char* name;
  const char* signature;
};
enum CBObjectMethodId {
  MID_ON_ERROR = 0,
  MID_ON_WARNING = 1,
  MID_ON_USEROFFLINE = 2,
  MID_ON_USERJOINED = 3,
  MID_ON_AUDIOFRAME_RECEIVED = 4,
  MID_ON_VIDEOFRAME_RECEIVED = 5,
  MID_ON_LEAVECHANNEL = 6,
  MID_CBOBJECT_NUM = 7,
};
enum ClassMethodId {
  MID_GET_RANDOM_UUID = 0,
  MID_GET_LOCAL_HOST = 1,
  MID_GET_LOCAL_HOST_LIST = 2,
  MID_CLASS_NUM = 3,
};
static JavaObjectMethod jCBObjectMethods[] = {
  { MID_ON_ERROR, "onError", "(II)V" },
  { MID_ON_WARNING, "onWarning", "(I)V" },
  { MID_ON_USEROFFLINE, "onUserOffline", "(JI)V" },
  { MID_ON_USERJOINED, "onUserJoined", "(JLjava/lang/String;)V"},
  { MID_ON_AUDIOFRAME_RECEIVED, "audioFrameReceived", "(JILio/agora/recording/common/Common$AudioFrame;)V"},
  { MID_ON_VIDEOFRAME_RECEIVED, "videoFrameReceived","(JILio/agora/recording/common/Common$VideoFrame;I)V"},
  { MID_ON_LEAVECHANNEL, "onLeaveChannel", "(I)V"}
};
//video
enum VIDEOFRAME{
  MID_INIT = 0,
  MID_YUV = 1,
  MID_H264 = 2,
  MID_JPG = 3
};
static JavaObjectMethod jVideoFrameMethods[] = {
  {MID_INIT, "<init>" "(Lio/agora/recording/common/Common;)V"},
  {MID_YUV, "yuv", "Lio/agora/recording/common/Common$VideoYuvFrame;"},
  {MID_H264,"h264","Lio/agora/recording/common/Common$VideoH264Frame;"},
  {MID_JPG, "jpg", "Lio/agora/recording/common/Common$VideoJpgFrame;"}
};
enum FID_VIDEOYUVFRAME{
  FID_FRAMEMS = 0,
  FID_YUVBUF = 1,
  FID_YUVBUFSIZE =2,
  FID_YUVWIDTH = 3,
  FID_YUVHEIGHT = 4,
  FID_YUVNUM = 5,
};

static JavaObjectMethod jVideoYuvFrameFields[] = {
  {FID_FRAMEMS, FID_FRAME_MS, LONG_SIGNATURE},
  {FID_YUVBUF, FID_VIDEO_YUV_FRAME_BUF, "Ljava/nio/ByteBuffer;"},
  {FID_YUVBUFSIZE,FID_VIDEO_YUV_FRAME_BUFSIZE, LONG_SIGNATURE},
  {FID_YUVWIDTH,"width", "I"},
  {FID_YUVHEIGHT, "height", "I"},
};
enum FID_VIDEOH264FRAME{
  FID_H264_FRAMEMS = 0 ,
  FID_H264_FRAMENUM = 1,
  FID_H264_BUF = 2,
  FID_H264_BUF_SIZE = 3,
  FID_H264NUM = 4
};
static JavaObjectMethod jVideoH264FrameFields[] = {
  {FID_H264_FRAMEMS, FID_FRAME_MS, LONG_SIGNATURE},
  {FID_H264_FRAMENUM, FID_CHANNELNUM, LONG_SIGNATURE},
  {FID_H264_BUF, FID_VIDEO_YUV_FRAME_BUF, BYTEARRAY},
  {FID_H264_BUF_SIZE, FID_VIDEO_YUV_FRAME_BUFSIZE, LONG_SIGNATURE}
};
enum FID_JPGFRAME{
  FID_JPG_FRAMEMS = 0,
  FID_JPG_BUF = 1,
  FID_JPG_BUFSIZE = 2,
  FID_JPGNUM = 3
};
static JavaObjectMethod jVideoJpgFrameFields[] = {
  {FID_JPG_FRAMEMS, FID_FRAME_MS, LONG_SIGNATURE},
  {FID_JPG_BUF, FID_VIDEO_YUV_FRAME_BUF, BYTEARRAY},
  {FID_JPG_BUFSIZE, FID_VIDEO_YUV_FRAME_BUFSIZE, LONG_SIGNATURE},
};
//audio
enum FID_AUDIOFRAME{
  FID_AF_PCM = 0,
  FID_AF_AAC = 1,
  FID_AF_NUM= 2
};
static JavaObjectMethod jAudioFrameFields[] = {
  {FID_AF_PCM, FID_PCM, SN_AUDIO_PCM_FRAME},
  {FID_AF_AAC, FID_AAC, SN_AUDIO_AAC_FRAME}
};
enum FID_PCMFRAME{
  FID_PCM_FRAMEMS = 0,
  FID_PCM_CHANNELS = 1,
  FID_PCM_SAMPLEBITS = 2,
  FID_PCM_SAMPLERATES = 3,
  FID_PCM_SAMPLES = 4,
  FID_PCM_BUF = 5,
  FID_PCM_BUFSIZE = 6,
  FID_PCMNUM = 7
};
static JavaObjectMethod jAudioPcmFrameFields[] = {
  {FID_PCM_FRAMEMS, FID_FRAME_MS,LONG_SIGNATURE},
  {FID_PCM_CHANNELS, FID_CHANNELS, LONG_SIGNATURE},
  {FID_PCM_SAMPLEBITS, FID_SAMPLE_BITS, LONG_SIGNATURE},
  {FID_PCM_SAMPLERATES, FID_SAMPLE_RATES, LONG_SIGNATURE},
  {FID_PCM_SAMPLES, FID_SAMPLE, LONG_SIGNATURE},
  {FID_PCM_BUF, FID_PCMBUF, BYTEARRAY},
  {FID_PCM_BUFSIZE, FID_PCMBUFFERSIZE, LONG_SIGNATURE},
};
enum FID_AACFRAME{
  FID_AAC_FRAMEMS = 0,
  FID_AAC_BUF = 1,
  FID_AAC_BUFSIZE = 2,
  FID_AACNUM = 3
};
static JavaObjectMethod jAudioAacFrameFields[] = {
  {FID_AAC_FRAMEMS, FID_FRAME_MS,LONG_SIGNATURE},
  {FID_AAC_BUF, FID_PCMBUF, BYTEARRAY},
  {FID_AAC_BUFSIZE, FID_PCMBUFFERSIZE, LONG_SIGNATURE}
};



