const moment = require('moment');
const {
  generateMediaChannelKey,
  generateRecordingKey
} = require('../node_modules/agora-dynamic-key/nodejs/src/DynamicKey5.js');

const { spawn } = require('child_process');
const path = require('path');
function getVideoDir(channelName, startTime) {
  return `${moment().format('yyyy_mm_dd')}/${channelName}_${startTime}`;
}

function createDynamicKey({ channelName, uid }) {
  const ts = Math.round(new Date().getTime() / 1000);
  const rnd = Math.round(Math.random() * 100000000);
  const key = generateMediaChannelKey(
    process.env.APP_ID,
    process.env.APP_CERTIFICATE,
    channelName,
    ts,
    rnd,
    uid
  );

  return key;
}

function createRecordingKey({ channelName, uid }) {
  const ts = Math.round(new Date().getTime() / 1000);
  const rnd = Math.round(Math.random() * 100000000);
  const key = generateRecordingKey(
    process.env.APP_ID,
    process.env.APP_CERTIFICATE,
    channelName,
    ts,
    rnd,
    uid
  );

  return key;
}

function startRecording(channelName) {
  return `--appId ${
    process.env.APP_ID
  } --channel ${channelName} --channelKey ${createDynamicKey({
    channelName
  })} --appliteDir ./Agora_Recording_SDK_for_Linux_FULL/bin --isMixingEnabled 0`;
}

function startRecording2(channelName) {
  return [
    `--appId`,
    `${process.env.APP_ID}`,
    `--channel`,
    `${channelName}`,
    `--channelKey`,
    `${createDynamicKey({ channelName })}`,
    `--appliteDir`,
    `./Agora_Recording_SDK_for_Linux_FULL/bin`,
    `--isMixingEnabled`,
    `0`
  ];
}

function spawnAgoraProcess() {
  console.log(__dirname);
  const child = spawn(
    `${path.resolve('./Agora_Recording_SDK_for_Linux_FULL/samples/cpp/recorder_local')}`,
    startRecording2(1000)
  );

  const regex1 = /(Recording directory is )(\S+)/;
  const regex2 = /(Recording directory is )/;
  child.stdout.on('data', function(data) {
    if (regex1.test(data.toString())) {
      process.env.RECORDING_DIRECTORY = data.toString().replace(regex2, '');
    }
    console.log('stdout: ' + data.toString());
  });

  child.stderr.on('data', function(data) {
    console.log('stderr: ' + data.toString());
  });

  child.on('exit', function(code) {
    spawnAgoraProcess();
    console.log('child process exited with code ' + code.toString());
  });
}

module.exports = {
  getVideoDir,
  createDynamicKey,
  createRecordingKey,
  startRecording,
  startRecording2,
  spawnAgoraProcess
};
