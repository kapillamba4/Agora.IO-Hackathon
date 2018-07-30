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
#include "agorasdk/AgoraSdk.h" 
using std::string;
using std::cout;
using std::cerr;
using std::endl;

using agora::base::opt_parser;
using agora::linuxsdk::VideoFrame;
using agora::linuxsdk::AudioFrame;

atomic_bool_t g_bSignalStop;
atomic_bool_t g_bSignalStartService;
atomic_bool_t g_bSignalStopService;

void signal_handler(int signo) {
  (void)signo;

  // cerr << "Signal " << signo << endl;
  g_bSignalStop = true;
}

void start_service(int signo) {
    (void)signo;
    g_bSignalStartService = true;
}

void stop_service(int signo) {
    (void)signo;
    g_bSignalStopService = true;
}

int main(int argc, char * const argv[]) {
  uint32_t uid = 0;
  string appId;
  string channelKey;
  string name;
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
  int width = 0;
  int height = 0;
  int fps = 0;
  int kbps = 0;
  int triggerMode = agora::linuxsdk::AUTOMATICALLY_MODE;

  /**
   * change log_config Facility per your specific purpose like agora::base::LOCAL5_LOG_FCLT
   * Default:USER_LOG_FCLT. 
   * agora::base::log_config::setFacility(agora::base::LOCAL5_LOG_FCLT);
   */
  g_bSignalStop = false;
  g_bSignalStartService = false;
  g_bSignalStopService = false;

  signal(SIGQUIT, signal_handler);
  signal(SIGABRT, signal_handler);
  signal(SIGINT, signal_handler);
  signal(SIGPIPE, SIG_IGN);

  opt_parser parser;

  parser.add_long_opt("appId", &appId, "App Id/must", agora::base::opt_parser::require_argu);
  parser.add_long_opt("uid", &uid, "User Id default is 0/must", agora::base::opt_parser::require_argu);

  parser.add_long_opt("channel", &name, "Channel Id/must", agora::base::opt_parser::require_argu);
  parser.add_long_opt("appliteDir", &applitePath, "directory of app lite 'AgoraCoreService', Must pointer to 'Agora_Recording_SDK_for_Linux_FULL/bin/' folder/must",
          agora::base::opt_parser::require_argu);

  parser.add_long_opt("channelKey", &channelKey, "channelKey/option");
  parser.add_long_opt("channelProfile", &channelProfile, "channel_profile:(0:COMMUNICATION),(1:broadcast) default is 0/option");

  parser.add_long_opt("isAudioOnly", &isAudioOnly, "Default 0:A/V, 1:AudioOnly (0:1)/option");
  parser.add_long_opt("isVideoOnly", &isVideoOnly, "Default 0:A/V, 1:VideoOnly (0:1)/option");
  parser.add_long_opt("isMixingEnabled", &isMixingEnabled, "Mixing Enable? (0:1)/option");
  parser.add_long_opt("mixResolution", &mixResolution, "change default resolution for vdieo mix mode/option");
  parser.add_long_opt("mixedVideoAudio", &mixedVideoAudio, "mixVideoAudio:(0:seperated Audio,Video) (1:mixed Audio & Video), default is 0 /option");

  parser.add_long_opt("decryptionMode", &decryptionMode, "decryption Mode, default is NULL/option");
  parser.add_long_opt("secret", &secret, "input secret when enable decryptionMode/option");

  parser.add_long_opt("idle", &idleLimitSec, "Default 300s, should be above 3s/option");
  parser.add_long_opt("recordFileRootDir", &recordFileRootDir, "recording file root dir/option");

  parser.add_long_opt("lowUdpPort", &lowUdpPort, "default is random value/option");
  parser.add_long_opt("highUdpPort", &highUdpPort, "default is random value/option");

  parser.add_long_opt("getAudioFrame", &getAudioFrame, "default 0 (0:save as file, 1:aac frame, 2:pcm frame, 3:mixed pcm frame) (Can't combine with isMixingEnabled) /option");
  parser.add_long_opt("getVideoFrame", &getVideoFrame, "default 0 (0:save as file, 1:h.264, 2:yuv, 3:jpg buffer, 4:jpg file, 5:jpg file and video file) (Can't combine with isMixingEnabled) /option");
  parser.add_long_opt("captureInterval", &captureInterval, "default 5 (Video snapshot interval (second))");
  parser.add_long_opt("cfgFilePath", &cfgFilePath, "config file path / option");
  parser.add_long_opt("streamType", &streamType, "remote video stream type(0:STREAM_HIGH,1:STREAM_LOW), default is 0/option");
  parser.add_long_opt("triggerMode", &triggerMode, "triggerMode:(0: automatically mode, 1: manually mode) default is 0/option");
  parser.add_long_opt("proxyServer", &proxyServer, "proxyServer:format ip:port, eg,\"127.0.0.1:1080\"/option");

  if (!parser.parse_opts(argc, argv) || appId.empty() || name.empty()) {
    std::ostringstream sout;
    parser.print_usage(argv[0], sout);
    cout<<sout.str()<<endl;
    return -1;
  }
 
  if(triggerMode == agora::linuxsdk::MANUALLY_MODE) {
      signal(SIGUSR1, start_service);
      signal(SIGUSR2, stop_service);
  }
  
  if(!recordFileRootDir.empty() && !cfgFilePath.empty()){
    LOG(ERROR,"Client can't set both recordFileRootDir and cfgFilePath");
    return -1;
  }

  if(recordFileRootDir.empty() && cfgFilePath.empty())
      recordFileRootDir = ".";

  //Once recording video under video mixing model, client needs to config width, height, fps and kbps
  if(isMixingEnabled && !isAudioOnly) {
     if(4 != sscanf(mixResolution.c_str(), "%d,%d,%d,%d", &width,
                  &height, &fps, &kbps)) {
        LOG(ERROR, "Illegal resolution: %s", mixResolution.c_str());
        return -1;
     }
  }

  LOG(INFO, "uid %" PRIu32 " from vendor %s is joining channel %s",
          uid, appId.c_str(), name.c_str());

  agora::AgoraSdk recorder;
  agora::recording::RecordingConfig config;
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
  config.proxyServer = proxyServer.empty()? NULL:const_cast<char*>(proxyServer.c_str());

  config.lowUdpPort = lowUdpPort;
  config.highUdpPort = highUdpPort;
  config.captureInterval = captureInterval;

  config.decodeAudio = static_cast<agora::linuxsdk::AUDIO_FORMAT_TYPE>(getAudioFrame);
  config.decodeVideo = static_cast<agora::linuxsdk::VIDEO_FORMAT_TYPE>(getVideoFrame);
  config.streamType = static_cast<agora::linuxsdk::REMOTE_VIDEO_STREAM_TYPE>(streamType);
  config.triggerMode = static_cast<agora::linuxsdk::TRIGGER_MODE_TYPE>(triggerMode);

  recorder.updateMixModeSetting(width, height, isMixingEnabled ? !isAudioOnly:false);

  if (!recorder.createChannel(appId, channelKey, name, uid, config)) {
    cerr << "Failed to create agora channel: " << name << endl;
    return -1;
  }

  cout << "Recording directory is " << recorder.getRecorderProperties()->storageDir << endl;
  recorder.updateStorageDir(recorder.getRecorderProperties()->storageDir);
  
  while (!recorder.stopped() && !g_bSignalStop) {
      if(g_bSignalStartService) {
          recorder.startService();
          g_bSignalStartService = false;
      }

      if(g_bSignalStopService) {
          recorder.stopService();
          g_bSignalStopService = false;
      }

      sleep(1);
  }

  if (g_bSignalStop) {
    recorder.leaveChannel();
    recorder.release();
  }

  cerr << "Stopped \n";
  return 0;
}

