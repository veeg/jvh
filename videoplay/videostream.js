
const NUM_CHUNKS = 50;

// Check that we can do this shit
window.MediaSource = window.MediaSource || window.WebKitMediaSource;
if (!!!window.MediaSource) {
  alert('MediaSource API is not available');
}


//
// Logger that is totally unneccesary
//
function Logger(id) {
  this.el = document.getElementById('log');
}
Logger.prototype.log = function(msg) {
  var fragment = document.createDocumentFragment();
  fragment.appendChild(document.createTextNode(msg));
  fragment.appendChild(document.createElement('br'));
  this.el.appendChild(fragment);
};
Logger.prototype.clear = function() {
  this.el.textContent = '';
};
var logger = new Logger('log');


//
// Video variables
//
var video = document.querySelector('video');
var mediaSource = new MediaSource();
var bufferQueue = [];
var sourceBuffer = null;

video.src = window.URL.createObjectURL(mediaSource);

//
// Media Source callbacks
//
mediaSource.addEventListener('sourceended', function () {
  logger.log("SOURCE CLOSED!")
  sourceBuffer = undefined
}, false);

mediaSource.addEventListener('sourceopen', function (e) {
  logger.log("SOURCE OPENED!")
  video.play();

  sourceBuffer = mediaSource.addSourceBuffer('video/webm; codecs="vp8"');

  sourceBuffer.addEventListener('updateend', function() {
    logger.log("UpdateEnd - Updating from queue (if any)");
    updateFromQueue();
  });



}, false);




//
// Update sourcerBuffer from queued buffer packets
//
function updateFromQueue() {
  logger.log("UpdateFromQueue");
  if (sourceBuffer === undefined) {
    logger.log("sourceBuffer undefined");
    return;
  }
  if (bufferQueue.length == 0 || sourceBuffer.updating) {
    logger.log("length 0(" +bufferQueue.length + ") or updating (" + sourceBuffer.updating + ")");
    return;
  }

  logger.log("Mother trugger");
  sourceBuffer.appendBuffer(bufferQueue.shift());
}


//
// Temp method to slice selected file and queue it up
//
function displayMofo (uInt8Array) {
    var file = new Blob([uInt8Array], {type: 'video/webm'});
    var chunkSize = Math.ceil(file.size / NUM_CHUNKS);

    // Slice the video into NUM_CHUNKS and append each to the media element.
    var i = 0;

    logger.log('mediaSource readyState: ' + mediaSource.readyState);

    // Recursively append
    (function readChunk_(i) {
        var reader = new FileReader();

        // Reads aren't guaranteed to finish in the same order they're started in,
        // so we need to read + append the next chunk after the previous reader
        // is done (onload is fired).
        reader.onload = function(e) {
            var buffer = new Uint8Array(e.target.result);
            logger.log('appending chunk:' + i + ". Size: " + chunkSize);
            bufferQueue.push(buffer);
            updateFromQueue();

            if (i < NUM_CHUNKS) {
            // Resursive read
              readChunk_(++i);
            }
        };

        var startByte = chunkSize * i;
        var chunk = file.slice(startByte, startByte + chunkSize);

        reader.readAsArrayBuffer(chunk);
    })(i);  // Start the recursive call by self calling.
}

//function displayAll(uInt8Array) {
//    var file = new Blob([uInt8Array], {type: 'video/webm'});
//
//    var reader = new FileReader();
//    reader.onload = function(e) {
//      var buffer = new Uint8Array(e.target.result);
//      sourceBuffer.appendBuffer(buffer);
//    };
//    reader.readAsArrayBuffer(file);
//}

// Callback to select local file
function readSingleFile(e) {
  var file = e.target.files[0];
  if (!file) {
    return;
  }
  var reader = new FileReader();
  reader.onload = function(e) {
    var contents = e.target.result;
    displayMofo(contents);
    //displayAll(contents)
  };
  reader.readAsArrayBuffer(file);
}

// Register file-input callback
document.getElementById('file-input')
  .addEventListener('change', readSingleFile, false);


// Next test: serve file from web socket and serve it slice wise
// https://github.com/phoboslab/jsmpeg/blob/master/stream-server.js

// Super awesome websocket
function JVHWebSocket () {
  var ws;

  this.connect = function (host, port)
  {
    ws = new WebSocket('ws://' + host + ":" + port);

    ws.onopen = function () {
      console.log("New websocket connection opened to " + host + ":" + port);
    }

    ws.onmessage = function (e) {
      console.log("on message");

      //
      // Dump entire contents into stream
      //
      var reader = new FileReader();
      reader.onload = function(e) {
          var buffer = new Uint8Array(e.target.result);
          logger.log('appending chunk:' + i + ". Size: " + chunkSize);
          bufferQueue.push(buffer);
          updateFromQueue();
      };
      reader.readAsArrayBuffer(e.data);
    }
  }
}


