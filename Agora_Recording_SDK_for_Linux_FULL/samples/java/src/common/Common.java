package io.agora.recording.common;
import java.nio.ByteBuffer;

public class Common{
  public enum ERROR_CODE_TYPE {
    ERR_OK(0),
    //1~1000
    ERR_FAILED(1),
    ERR_INVALID_ARGUMENT(2),
    ERR_INTERNAL_FAILED(3);
    private ERROR_CODE_TYPE(int value){}
  }

  public enum STAT_CODE_TYPE {
    STAT_ERR_FROM_ENGINE(1),
    STAT_ERR_ARS_JOIN_CHANNEL(2),
    STAT_ERR_CREATE_PROCESS(3),
    STAT_ERR_MIXED_INVALID_VIDEO_PARAM(4),
    STAT_ERR_NULL_POINTER(5),
    STAT_ERR_PROXY_SERVER_INVALID_PARAM(6),
    STAT_POLL_ERR(8),
    STAT_POLL_HANG_UP(16),
    STAT_POLL_NVAL(32);
    private STAT_CODE_TYPE(int value) {}
  }
  public enum LEAVE_PATH_CODE {
    LEAVE_CODE_INIT(0), 
    LEAVE_CODE_SIG(1<<1),
    LEAVE_CODE_NO_USERS(1<<2),
    LEAVE_CODE_TIMER_CATCH(1<<3),
    LEAVE_CODE_CLIENT_LEAVE(1<<4);
    private LEAVE_PATH_CODE(int code) {}
  }

  public enum WARN_CODE_TYPE {
    WARN_NO_AVAILABLE_CHANNEL(103),
    WARN_LOOKUP_CHANNEL_TIMEOUT(104),
    WARN_LOOKUP_CHANNEL_REJECTED(105),
    WARN_OPEN_CHANNEL_TIMEOUT(106),
    WARN_OPEN_CHANNEL_REJECTED(107);
    private WARN_CODE_TYPE(int value){}
  }

  public enum CHANNEL_PROFILE_TYPE {
    CHANNEL_PROFILE_COMMUNICATION(0),
    CHANNEL_PROFILE_LIVE_BROADCASTING(1);
    private int value;
    private CHANNEL_PROFILE_TYPE(int value) {
      this.value = value;
    }
    public int getValue() {
      return value;
    }
  }
  public enum USER_OFFLINE_REASON_TYPE  {
    USER_OFFLINE_QUIT(0),
    USER_OFFLINE_DROPPED(1),
    USER_OFFLINE_BECOME_AUDIENCE(2);
    private USER_OFFLINE_REASON_TYPE(int code) {}
  }
  public enum REMOTE_VIDEO_STREAM_TYPE {
    REMOTE_VIDEO_STREAM_HIGH(0),
    REMOTE_VIDEO_STREAM_LOW(1);
    private int value;
    private REMOTE_VIDEO_STREAM_TYPE(int value) {
      this.value = value;
    }
    public int getValue() {
      return value;
    }
  }
  public enum SERVICE_MODE {
    RECORDING_MODE(0),//down stream
    SERVER_MODE(1),//up-down stream
    IOT_MODE(2);//up-down stream
    private SERVICE_MODE(int value){}
  }

  public enum VIDEO_FORMAT_TYPE {
    VIDEO_FORMAT_DEFAULT_TYPE(0),
    VIDEO_FORMAT_H264_FRAME_TYPE(1),
    VIDEO_FORMAT_YUV_FRAME_TYPE(2),
    VIDEO_FORMAT_JPG_FRAME_TYPE(3),
    VIDEO_FORMAT_JPG_FILE_TYPE(4),
    VIDEO_FORMAT_JPG_VIDEO_FILE_TYPE(5);
    private int value;
    private VIDEO_FORMAT_TYPE(int value) {
    this.value = value;
    }
    private int getValue(){
      return value; 
    }
  }

  public enum AUDIO_FORMAT_TYPE {
    AUDIO_FORMAT_DEFAULT_TYPE(0),
    AUDIO_FORMAT_AAC_FRAME_TYPE(1),
    AUDIO_FORMAT_PCM_FRAME_TYPE(2),
    AUDIO_FORMAT_MIXED_PCM_FRAME_TYPE(3);
    private int value;
    private AUDIO_FORMAT_TYPE(int value) {
      this.value = value;
    }
    private int getValue(){
      return value;
    }
  }
  public class AUDIO_FRAME_TYPE {
    public final int AUDIO_FRAME_RAW_PCM = 0;
    public final int AUDIO_FRAME_AAC =1;
    public int type = 1;
    public int getValue(){
      return type;
    }
  }

  public class VIDEO_FRAME_TYPE {
    public final int VIDEO_FRAME_RAW_YUV = 0;
    public final int VIDEO_FRAME_H264 = 1;
    public final int VIDEO_FRAME_JPG = 2;
    public int type = 0;
    public int getValue(){
      return type;
    }
  }

  public class AudioFrame {
    public AUDIO_FRAME_TYPE type;
    public AudioPcmFrame pcm;
    public AudioAacFrame aac;
  }
  public class AudioPcmFrame {
    public AudioPcmFrame(long frame_ms, long sample_rates, long samples) {
    }
    public long frame_ms;
    public long channels; // 1
    public long sample_bits; // 16
    public long sample_rates; // 8k, 16k, 32k
    public long samples;

    public ByteBuffer pcmBuf;
    public long pcmBufSize;
  }
  public class AudioAacFrame {
    public AudioAacFrame(long framems) {
      frame_ms = framems;
      aacBufSize = 0;
    }
    public ByteBuffer aacBuf;
    public long frame_ms;
    public long aacBufSize;
  }
  public class VideoYuvFrame {
    VideoYuvFrame(long framems, int width, int height, int ystride,int ustride, int vstride){
      this.frame_ms = framems;
      this.width = width;
      this.height = height;
      this.ystride = ystride;
      this.ustride = ustride;
      this.vstride = vstride;
    }
    public long frame_ms;

    public ByteBuffer ybuf;
    public ByteBuffer ubuf;
    public ByteBuffer vbuf;

    public int width;
    public int height;

    public int ystride;
    public int ustride;
    public int vstride;
    //all
    public ByteBuffer buf;
    public long bufSize;
  }
  public class VideoH264Frame {
    VideoH264Frame(){
      frame_ms = 0;
      frame_num = 0;
      bufSize = 0; 
    }
    public long frame_ms;
    public long frame_num;
    //all
    public ByteBuffer buf;
    public long bufSize;
  }
  public class VideoJpgFrame {
    VideoJpgFrame(){
      frame_ms = 0;
      bufSize = 0;
    }
    public long frame_ms;
    //all
    public ByteBuffer buf;
    public long bufSize;
  }
  public class VideoFrame {
    public VideoYuvFrame yuv;
    public VideoH264Frame h264;
    public VideoJpgFrame jpg;
    public int rotation; // 0, 90, 180, 270
  }

  public class UserJoinInfos {
    String storageDir;
    UserJoinInfos(){
      storageDir = "";
    }
  }

  public class VideoMixingLayout
  {
    public int canvasWidth;
    public int canvasHeight;
    public String backgroundColor;//e.g. "#C0C0C0" in RGB
    public int regionCount;
    public Region[] regions;
    public String appData;
    public int appDataLength;
    public class Region {
      public long uid;
      public double x;//[0,1]
      public double y;//[0,1]
      public double width;//[0,1]
      public double height;//[0,1]
      public int zOrder; //optional, [0, 100] //0 (default): bottom most, 100: top most
      //Optional
      //[0, 1.0] where 0 denotes throughly transparent, 1.0 opaque
      public double alpha;
      public int renderMode;//RENDER_MODE_HIDDEN: Crop, RENDER_MODE_FIT: Zoom to fit
      public Region(){
        uid = 0;
        x = 0;
        y = 0;
        width = 0;
        height = 0;
        zOrder = 0;
        alpha = 1.0;
        renderMode = 0;
      }
    }
    public VideoMixingLayout() {
      canvasWidth = 0;
      canvasHeight = 0;
      backgroundColor = "";
      regionCount = 0;
      //regions = 0;
      appData = "";
      appDataLength = 0;
    }
  } 
}
