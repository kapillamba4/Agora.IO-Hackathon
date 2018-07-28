if (!AgoraRTC.checkSystemRequirements()) {
  alert('browser is no support webRTC');
}

AgoraRTC.Logger.setLogLevel(AgoraRTC.Logger.DEBUG);

const url = new URL(window.location.href);

const appId = url.searchParams.get('appId');
const channelName = url.searchParams.get('channelName');
let uid = null;
let channelKey = null;

const client = AgoraRTC.createClient({ mode: 'h264_interop' });

client.init(
  appId,
  function() {
    console.log('client initialized');
  },
  function(err) {
    console.log('client init failed ', err);
  }
);

client.on('stream-published', function(evt) {
  console.log('local stream published');
});

client.on('error', function(err) {
  console.log('Got error msg:', err.reason);
  if (err.reason === 'DYNAMIC_KEY_TIMEOUT') {
    client.renewChannelKey(
      channelKey,
      function() {
        console.log('Renew channel key successfully');
      },
      function(err) {
        console.log('Renew channel key failed: ', err);
      }
    );
  }
});

client.on('stream-added', function(evt) {
  var stream = evt.stream;
  console.log('New stream added: ' + stream.getId());
  console.log('Subscribe ', stream);
  client.subscribe(stream, function(err) {
    console.log('Subscribe stream failed', err);
  });
});

client.on('stream-subscribed', function(evt) {
  var stream = evt.stream;
  console.log('Subscribe remote stream successfully: ' + stream.getId());
  if ($('div#video #agora_remote' + stream.getId()).length === 0) {
    $('div#video').append(
      '<div id="agora_remote' +
        stream.getId() +
        '" style="float:left; width:810px;height:607px;display:inline-block;"></div>'
    );
  }
  stream.play('agora_remote' + stream.getId());
});

client.on('stream-removed', function(evt) {
  var stream = evt.stream;
  stream.stop();
  $('#agora_remote' + stream.getId()).remove();
  console.log('Remote stream is removed ' + stream.getId());
});

client.on('peer-leave', function(evt) {
  var stream = evt.stream;
  if (stream) {
    stream.stop();
    $('#agora_remote' + stream.getId()).remove();
    console.log(evt.uid + ' leaved from this channel');
  }
});

function leave() {
  document.getElementById('leave').disabled = true;
  client.leave(
    function() {
      console.log('Leavel channel successfully');
    },
    function(err) {
      console.log('Leave channel failed');
    }
  );
}

function publish() {
  document.getElementById('publish').disabled = true;
  document.getElementById('unpublish').disabled = false;
  client.publish(localStream, function(err) {
    console.log('Publish local stream error: ' + err);
  });
}

function unpublish() {
  document.getElementById('publish').disabled = false;
  document.getElementById('unpublish').disabled = true;
  client.unpublish(localStream, function(err) {
    console.log('Unpublish local stream failed' + err);
  });
}

axios
  .get(`/dynamic_key?channelName=${channelName}&appId=${appId}`)
  .then(function({ data: dataChannelKey }) {
    channelKey = dataChannelKey;
    client.join(
      channelKey,
      channelName,
      uid,
      function(data) {
        uid = data;
        const localStream = AgoraRTC.createStream({
          streamID: uid,
          video: true,
          audio: true,
          screen: false
        });

        localStream.on('accessAllowed', function() {
          console.log('access allowed');
        });

        localStream.on('accessDenied', function() {
          alert('Please allow access to camera and mic');
        });

        localStream.init(
          function() {
            console.log('getUserMedia successfully');
            localStream.play('agora_local');

            client.publish(localStream, function(err) {
              console.log('Publish local stream error: ' + err);
            });

            client.on('stream-published', function(evt) {
              console.log('Publish local stream successfully');
            });
          },
          function(err) {
            console.log('getUserMedia failed', err);
          }
        );
      },
      function(err) {
        console.log('Join channel failed', err);
      }
    );
  })
  .catch(function(err) {
    console.log('AgoraRTC client init failed', err);
  });
