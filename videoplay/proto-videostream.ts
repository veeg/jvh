class VideoStreamElement {
    // The <video/> container element to stream media source into.
    private element: any;
    // The MediaSource used to feed the video.
    public mediaSource: any;

    /*
     * videoId: The <video/> id to attach videostream
     */
    public constructor(videoId: string) {
        let self = this;

        console.log("CONSTRUCTING: " + videoId);
        window.MediaSource = window.MediaSource || window.WebKitMediaSource;
        if (!!!window.MediaSource) {
            throw new Error('MediaSource API is not available');
        }

        // Prepare Video and MediaSource
        self.element = document.querySelector(videoId);
        console.log(this.videoElement);
        if (this.element === undefined || this.element === null) {
            throw new Error('Document id not available: ' + videoId);
        }

        self.mediaSource = new MediaSource();
        self.element.src = window.URL.createObjectURL(this.mediaSource);

        // Clean-up the error
        self._errorHandler = function () {
            self.videoElement.removeEventListener('error', self._errorHandler);
            // TODO: Destroy all registerd streams
        }
        self.videoElement.addEventListener('error', self._errorHandler);

        // XXX; this we cannot do
        // Handle mediaSource ended
        this.mediaSource.addEventListener('sourceended', function (e) {
            console.log("TRACE: SOURCE CLOSED");
            console.log(e);

            this.sourceBuffer = undefined;
            this.bufferQueue = [];
        }, false);
    }

}

/*
 * Encapsulates a single source to the VideoStreamElement
 */
class VideoStreamSource {

    // Every blob to be attached to the mediaSource is queues up before attachment.
    private bufferQueue: Array<any>;

    // The SourceBuffer returned from mediaSource
    private sourceBuffer: any;

    // The source type string. e.g, 'video/webm; codecs="vorbis, vp8"'
    private sourceType: string;

    public constructor(vse: VideoSourceElement) {
        let self = this;

        self.vse = vse;

        // Create the source buffer once media  source is open
        if (self.vse.mediaSource === 'open') {
            self.createSourceBuffer();
        } else {
            // TODO: register addEventListener('sourceopen', _) on mediasource to create source buffer then
        }
    }

    private createSourceBuffer() {
        let self = this;

        if (!window.MediaSource.isTypeSupported(self.sourceType)) {
            throw Error('Unsupported type ' + self.sourceType);
        }

        self.sourceBuffer = self.vse.mediaSource.addSourceBuffer(self.sourceType);
        self.sourceBuffer.addEventListener('updateend', self.updateFromBufferQueue);
    }

    private updateFromBufferQueue() {
        let self = this;

        console.log("TRACE: UpdateFromBufferQueue");
        if (self.sourceBuffer === undefined) {
            console.log("sourceBuffer undefined...");
            return;
        }
        if (self.bufferQueue.length == 0) {
            console.log("bufferQueue empty");
            return;
        }
        if (self.sourceBuffer.updating) {
            console.log("sourceBuffer updating - try again later");
            return;
        }

        self.sourceBuffer.appendBuffer(self.bufferQueue.shift());

        // Video not playing - resume it
        if (self.vse.element.paused) {
            self.vse.element.play();
        }
    }


}

class VideoStreamProtobufWebSocket {

    // The select video entry to stream from the server
    selectedVideoEntry: string;

    // The active websocket connection for this instance
    private websocket: any;

    public constructor() {
        let self = this;
    }

    private send (message) {
        this.websocket.send (message.serializeBinary());
    }

    public connectWebSocket(address: string, port: number) {
        let instance = this;
        this.websocket = new WebSocket('ws://' + address + ':' + port);

        // Handle 'onopen'
        this.websocket.onopen = function () {
            console.log("New websocket connection opened to " + address + " on port " + port)

            // Send request for stream entries
            var message = new proto.videostream.FromClient;
            message.setRequestStreamEntries (true);
            instance.send(message);

            instance.selectVideoEntry("parrot");
        }

        // Handle 'onmessage'
        this.websocket.onmessage = function (e) {
            console.log("websocket on message");

            // Update
            let reader = new FileReader();

            reader.onload = function ()
            {

                var msg = proto.videostream.ToClient.deserializeBinary (this.result);
                var messageType = proto.videostream.ToClient.MsgCase;

                console.log(msg);

                switch (msg.getMsgCase())
                {
                    case messageType.PAYLOAD:
                        console.log("Payload");

                        let buffer = new Uint8Array(msg.getPayload().getPayloadList_asU8());
                        //let buffer = msg.getPayload().getPayloadList_asU8();
                        console.log(buffer)
                        instance.bufferQueue.push(buffer);
                        instance.updateFromBufferQueue();
                        break;

                    case messageType.STREAM_ENTRIES:
                        console.log("Stream entries");
                        break;

                    default:
                        console.log("No handler for message type" + msg.getMsgCase())
                        break;
                }

                //let msg = proto.videostreampb.ToClient.deserializeBinary (this.result);
                //let messageType = proto.videostreampb.ToClient.MsgCase;

                //switch (msg.getMsgCase()) {

                //    case messageType.RESPONSE:
                //        console.log("TYPE: payload");
                //        instance.bufferQueue.push(buffer);
                //        instance.updateFromBufferQueue();

                //    default:
                //        console.log("Received unknown response from Disir Service");
                //}
            }

            reader.readAsArrayBuffer (e.data);
        }

        // Handle 'onclose'
        this.websocket.onclose = function () {
            console.log("websocket onclose");
        }
    }

    // Let this be the selctor to choose which entry to request from the server
    public selectVideoEntry(entryId: string) {
        this.selectedVideoEntry = entryId;
        console.log("selectVideoEntry: " + entryId);

        // XXX: Check if list is retrieved - reset internal state instead maybe?

        // TODO: Panic if entryId is not in internal list retrieved from server.

        // TODO: Reset current selected entry (if any)
        // Remove active sourceBuffer (if active)

        // TODO: Get video format and codecs from internal list retrieved from server.

        this.bufferQueue = []
        let instance = this;

        var message = new proto.videostream.FromClient;
        message.setSelectStreamEntry("parrot");
        instance.send(message);


        // XXX: Need to manage contents of this block dynamically. Can probably not have multiple
        // event listeners doing this shit
        //
        this.mediaSource.addEventListener('sourceopen', function (e) {
            console.log("TRACE: SOURCE OPEND");
            // We can only add a source buffer to the mediaSource once its properly opened
            instance.sourceBuffer = this.addSourceBuffer('video/webm; codecs="vp8"');
            instance.sourceBuffer.addEventListener('updateend', function () {
                console.log("TRACE: UPDATE END");
                instance.updateFromBufferQueue();
            });
        }, false);
    }
}

