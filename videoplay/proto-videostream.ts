class VideoStreamElement {

    // The parent HTML tag that will contain the entire video + overlay
    private elementParent: any;

    // The <video/> container element to stream media source into.
    private elementVideo: any;

    private elementOverlay: any;

    // The MediaSource used to feed the video.
    public mediaSource: any;

    /*
     */
    public constructor(parentId: string) {
        let self = this;

        // This is the parent element. Everything within is cleared
        // Prepare Video and MediaSource
        self.elementParent = document.querySelector(parentId);
        if (self.elementParent === undefined || self.elementParent === null) {
            throw new Error('Document id not available: ' + parentId);
        }

        // Clear everything previously declared within
        self.elementParent.innerHTML = "";

        // Create and append overlay element
        self.elementOverlay = document.createElement('div');
        self.elementParent.appendChild(self.elementOverlay);

        // Create and append video element
        let tmpDiv = document.createElement('div');
        self.elementParent.appendChild(tmpDiv);
        self.elementVideo = document.createElement('video');
        self.elementVideo.setAttribute('autoplay', '');
        tmpDiv.appendChild(self.elementVideo);

        window.MediaSource = window.MediaSource || window.WebKitMediaSource;
        if (!!!window.MediaSource) {
            // TODO: Populate overlay message
            throw new Error('MediaSource API is not available');
        }

        // Bind event handlers so 'this' references class instead of event
        self.errorHandler = self.errorHandler.bind(self);

        self.mediaSource = new MediaSource();
        self.elementVideo.src = window.URL.createObjectURL(self.mediaSource);

        // Clean-up the error
        self.elementVideo.addEventListener('error', self.errorHandler);
    }

    private errorHandler(e: Event) {
        let self = this;

        console.log("Media element raised erroenous event");
        console.log(e);
        self.elementVideo.removeEventListener('error', self.errorHandler);
        // TODO: Destroy all registerd streams
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

    // The VideoStreamElement who handles this video source
    private vse: VideoStreamElement;

    public constructor(vse: VideoStreamElement) {
        let self = this;

        self.vse = vse;
        self.sourceType = 'video/webm; codecs="vp8"';

        // Bind event handlers so 'this' references class instead of event
        self.openHandler = self.openHandler.bind(self);

        // Create the source buffer once media  source is open
        if (self.vse.mediaSource.readyState === 'open') {
            self.createSourceBuffer();
        } else {
            self.vse.mediaSource.addEventListener('sourceopen', self.openHandler);
        }
    }

    public appendBuffer(blob: any) {
        let self = this;

        if (self.bufferQueue === undefined) {
            console.log("Attempted to appendBuffer to undefined bufferQueue");
            throw Error("Missing source buffer.");
        }

        self.bufferQueue.push(blob);
        self.updateFromBufferQueue();
    }

    private createSourceBuffer() {
        let self = this;

        if (!window.MediaSource.isTypeSupported(self.sourceType)) {
            throw Error('Unsupported type ' + self.sourceType);
        }

        self.sourceBuffer = self.vse.mediaSource.addSourceBuffer(self.sourceType);
        self.sourceBuffer.addEventListener('updateend', self.updateFromBufferQueue);

        self.bufferQueue = new Array();

        console.log("Created source buffer");
    }

    private updateFromBufferQueue() {
        let self = this;

        if (self.sourceBuffer == null) {
            // XXX: Why is this happeneing? Vy is the sourceBuffer null/undefined
            return;
        }
        if (self.bufferQueue.length == 0) {
            return;
        }
        if (self.sourceBuffer.updating) {
            console.log("sourceBuffer updating - try again later");
            return;
        }

        console.log("Appending buffer to SourceBuffer");
        self.sourceBuffer.appendBuffer(self.bufferQueue.shift());
    }

    private openHandler(e: Event) {
        let self = this;

        self.vse.mediaSource.removeEventListener('sourceopen', self.openHandler);
        self.createSourceBuffer();
    }
}

class ProtoVideoStream {

    // The select video entry to stream from the server
    public selectedStreamEntry: string;

    //
    private streamEntries: Map<string, boolean>;

    // The active websocket connection for this instance
    private websocket: any;

    // The internal stream element allocated
    private vse: VideoStreamElement;

    // Internal allocation of a source buffer to feed the video stream element
    public videoStreamSource: VideoStreamSource;

    public constructor(id: string) {
        let self = this;

        self.vse = new VideoStreamElement(id);
    }

    private send (message) {
        this.websocket.send (message.serializeBinary());
    }

    public connectWebSocket(address: string, port: number) {
        let self = this;
        this.websocket = new WebSocket('ws://' + address + ':' + port);

        // Handle 'onopen'
        this.websocket.onopen = function () {
            console.log("New websocket connection opened to " + address + " on port " + port)

            // Send request for stream entries
            var message = new proto.videostream.FromClient;
            message.setRequestStreamEntries (true);
            self.send(message);
        }

        // Handle 'onmessage'
        this.websocket.onmessage = function (e) {

            // Update
            let reader = new FileReader();

            reader.onload = function ()
            {
                var msg = proto.videostream.ToClient.deserializeBinary (this.result);
                var messageType = proto.videostream.ToClient.MsgCase;

                switch (msg.getMsgCase())
                {
                    case messageType.PAYLOAD:
                        console.log("proto: payload");

                        let buffer = new Uint8Array(msg.getPayload().getPayloadList_asU8());
                        let payload = msg.getPayload().getPayloadList_asU8();
                        self.videoStreamSource.appendBuffer(payload[0]);
                        break;

                    case messageType.STREAM_ENTRIES:
                        console.log("proto: stream entries");
                        // TODO: populate array with array from protobuf msg
                        self.streamEntries = new Map<string, boolean>();
                        //TEMPORARY
                        self.streamEntries["parrot"] = true

                        self.changeSelectedVideoEntry();
                        break;

                    default:
                        console.log("No handler for message type" + msg.getMsgCase())
                        break;
                }
            }

            reader.readAsArrayBuffer (e.data);
        }

        // Handle 'onclose'
        this.websocket.onclose = function () {
            console.log("websocket onclose");
        }
    }

    // Let this be the selctor to choose which entry to request from the server
    public selectStreamEntry(entryId: string) {
        let self = this;

        console.log("User selected stream entry: " + entryId);
        self.selectedStreamEntry = entryId;

        // Only handle the change if we are connected and know the stream entries list
        if (self.streamEntries === undefined)
            return;

        self.changeSelectedVideoEntry();
    }

    private changeSelectedVideoEntry() {
        let self = this;

        console.log("Changing selected stream entry: " + self.selectedStreamEntry);
        // TODO: Panic if entryId is not in internal list retrieved from server.

        if (self.streamEntries.has(self.selectedStreamEntry) == false) {
            // TODO: Add text to video overlay
            //console.log("Stream entry " + self.selectedStreamEntry + " unavailable");
            //return;
        }

        // Remove active stream source buffer
        if (self.videoStreamSource !== undefined) {
            // TODO: Remove active sourceBuffer (if active)
        }

        // TODO: Get codec type from streamEntries and feed this to videostreamsource
        // XXX: streamEntries should probably be a map
        self.videoStreamSource = new VideoStreamSource(self.vse);

        var message = new proto.videostream.FromClient;
        message.setSelectStreamEntry(self.selectedStreamEntry);
        self.send(message);
    }
}

