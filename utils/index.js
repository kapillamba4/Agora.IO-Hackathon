const moment = require('moment');

module.exports = {
  getVideoDir: (channelName, startTime) => {
    return `${moment().format('yyyy_mm_dd')}/${channelName}_${startTime}`;
  },
  startRecording: channelName => {
    return `/app/recorder_local --appId ${process.env.APP_ID} 
                                --channel ${channelName} 
                                --appliteDir /app/bin 
                                --cfgFilePath /app/agoraSdkConfig.json
                                --isMixingEnabled 0`;
  }
};
