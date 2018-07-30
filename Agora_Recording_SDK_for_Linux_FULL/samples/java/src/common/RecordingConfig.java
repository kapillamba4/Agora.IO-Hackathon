import io.agora.recording.common.*;
import io.agora.recording.common.Common.*;

class RecordingEngineProperties{
  private String storageDir;
  public String GetStorageDir(){
    return storageDir;
  }
}


class RecordingConfig { 
  public boolean isAudioOnly;
  public boolean isVideoOnly;
  public boolean isMixingEnabled;
  public boolean mixedVideoAudio;
  public String mixResolution;
  public String decryptionMode;
  public String secret;
  public String appliteDir;
  public String recordFileRootDir;
  public String cfgFilePath;
  //decodeVideo: default 0 (0:save as file, 1:h.264, 2:yuv, 3:jpg buffer, 4:jpg file, 5:jpg file and video file)
  public VIDEO_FORMAT_TYPE decodeVideo;
  //decodeAudio:  (default 0 (0:save as file, 1:aac frame, 2:pcm frame, 3:mixed pcm frame) (Can't combine with isMixingEnabled) /option)
  public AUDIO_FORMAT_TYPE decodeAudio;
  public int lowUdpPort;
  public int highUdpPort;
  public int idleLimitSec;
  public int captureInterval;
  //channelProfile:0 braodacast, 1:communicate; default is 1
  public CHANNEL_PROFILE_TYPE channelProfile;
  //streamType:0:get low stream 1:get high stream; default is 0
  public REMOTE_VIDEO_STREAM_TYPE streamType;
  public int triggerMode;
  public String proxyServer; //format ipv4:port
}
