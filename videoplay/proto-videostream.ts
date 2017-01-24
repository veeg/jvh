class VideoStreamElement {

    // The parent HTML tag that will contain the entire video + overlay
    private elementParent: any;

    // The <video/> container element to stream media source into.
    private elementVideo: any;

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
        self.elementParent.classList.add('proto-videostream-content');
        self.setOverlayText("Initializing");

        // Create and append element
        self.elementVideo = document.createElement('video');
        self.elementVideo.setAttribute('autoplay', '');
        self.elementParent.appendChild(self.elementVideo);

        window.MediaSource = window.MediaSource || window.WebKitMediaSource;
        if (!!!window.MediaSource) {
            self.setOverlayText("MediaSource API unavailable");
            throw new Error('MediaSource API is not available');
        }

        // Bind event handlers so 'this' references class instead of event
        self.errorHandler = self.errorHandler.bind(self);

        self.mediaSource = new MediaSource();
        self.elementVideo.src = window.URL.createObjectURL(self.mediaSource);

        // Clean-up the error
        self.elementVideo.addEventListener('error', self.errorHandler);

        self.setOverlayText("");
    }

    public setOverlayText(text: string) {
        let self = this;

        self.elementParent.setAttribute('data-overlay-text', text);

        // We control the padding through data-overlay-padding attribute
        // We do this to create the overlay with a "nice" formated message,
        // whilst removing it when we actually have video to show (ugly little box if not)
        if (text == "") {
            self.elementParent.setAttribute('data-overlay-padding', "none");
        } else {
            self.elementParent.setAttribute('data-overlay-padding', "block");
        }
    }

    private errorHandler(e: Event) {
        let self = this;

        self.setOverlayText("Unexpected error");
        console.log("Media element raised erroenous event");
        console.log(e);
        self.elementVideo.removeEventListener('error', self.errorHandler);
        // TODO: Destroy all registerd streams

        throw new Error("Unexpected Media error");
    }
}

class StreamEntry {
    // Name of this stream entry
    public name: string;

    // Comma separated list of codecs, or something
    // XXX CLARIFY
    public codecs: string;

    // The type of video this stream entry is: webm
    public format: string;

    public constructor(name: string, format: string, codecs: string) {
        let self = this;

        self.name = name;
        self.format = format;
        self.codecs = codecs;
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

    private format: string;
    private codecs: string;

    // The VideoStreamElement who handles this video source
    private video: VideoStreamElement;

    public constructor(video: VideoStreamElement, format: string, codecs: string) {
        let self = this;

        self.video = video;
        self.format = format;
        self.codecs = codecs;
        self.sourceType = `video/${format}; codecs="${codecs}"`;

        // Bind event handlers so 'this' references class instead of event
        self.openHandler = self.openHandler.bind(self);
        self.updateendHandler = self.updateendHandler.bind(self);
        self.errorHandler = self.errorHandler.bind(self);
        self.abortHandler = self.abortHandler.bind(self);

        // Create the source buffer once media  source is open
        if (self.video.mediaSource.readyState === 'open') {
            self.createSourceBuffer();
        } else {
            self.video.mediaSource.addEventListener('sourceopen', self.openHandler);
        }
    }

    public appendBuffer(blob: any) {
        let self = this;

        if (self.bufferQueue === undefined) {
            console.log("Attempted to appendBuffer to undefined bufferQueue");
            throw Error("Missing source buffer.");
        }

        self.bufferQueue.push(blob);
        if (self.sourceBuffer.updating == false) {
            self.updateFromBufferQueue();
        } else {
          console.log("Not issuing updateFromBufferQueue");
        }
    }

    private createSourceBuffer() {
        let self = this;

        if (!window.MediaSource.isTypeSupported(self.sourceType)) {
            let text = `Unsupported format ${self.format} (codecs: ${self.codecs}`
            self.video.setOverlayText(text);
            throw Error('Unsupported type ' + self.sourceType);
        }

        self.sourceBuffer = self.video.mediaSource.addSourceBuffer(self.sourceType);
        self.sourceBuffer.mode = 'sequence';
        self.sourceBuffer.addEventListener('updateend', self.updateendHandler);

        self.bufferQueue = new Array();

        console.log("Created source buffer");
    }

    private updateFromBufferQueue() {
        let self = this;

        if (self.sourceBuffer == null) {
            throw new Error("SourceBuffer undefined");
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

        self.video.mediaSource.removeEventListener('sourceopen', self.openHandler);
        self.createSourceBuffer();
    }

    private updateendHandler(e: Event) {
        let self = this;

        self.updateFromBufferQueue();
    }

    private errorHandler(e: Event) {
        let self = this;

        console.log("VideoSource error");
        console.log(e);
    }

    private abortHandler(e: Event) {
        let self = this;

        console.log("VideoSource abort");
        console.log(e);
    }
}

class ProtoVideoStream {

    // The select video entry to stream from the server
    public selectedStreamEntry: string;

    //
    private streamEntries: Map<string, StreamEntry>;

    // The active websocket connection for this instance
    private websocket: any;

    // The internal stream element allocated
    private video: VideoStreamElement;

    // Internal allocation of a source buffer to feed the video stream element
    public videoStreamSource: VideoStreamSource;

    public constructor(id: string) {
        let self = this;

        self.video = new VideoStreamElement(id);
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

                        self.streamEntries = new Map<string, StreamEntry>();

                        let entries = msg.getStreamEntries().getEntriesList();
                        for (let i = 0; i < entries.length; i++) {
                            let entry = entries[i];
                            let struct = new StreamEntry(entry.getName(),
                                                         entry.getFormat(),
                                                         entry.getCodec());
                            self.streamEntries.set(entry.getName(), struct);
                        }

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
            self.video.setOverlayText("Connection closed...");
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

        if (self.streamEntries.has(self.selectedStreamEntry) == false) {
            self.video.setOverlayText("Stream '" + self.selectedStreamEntry +"' unavailable");
            throw new Error("Stream entry '"+ self.selectedStreamEntry + "' unavailable");
        }

        // Remove active stream source buffer
        if (self.videoStreamSource !== undefined) {
            // TODO: Remove active sourceBuffer (if active)
        }

        let entry = self.streamEntries.get(self.selectedStreamEntry);
        self.videoStreamSource = new VideoStreamSource(self.video, entry.format, entry.codecs);

        // Send our selection to the server!
        var message = new proto.videostream.FromClient;
        message.setSelectStreamEntry(self.selectedStreamEntry);
        self.send(message);
    }
}

