
namespace jvh
{

    //! Defines one encode operation
    struct stream_sequence {
        int se_frames_per_sequence;
        uint8_t **se_frames;
    };

    typedef enum pixel_format {
        PIX_FMT_YUV =
    }

    class Encoder
    {
    public:
        Encoder();
        ~Encoder();

        void stream_enqueue (uint8_t *data);
        stream_dequeue ();
    private:
        uint64_t m_frame_size;
        uint32_t m_frame_height;
        uint32_t m_frame_width;
        int64_t m_frame_timestamp;
        enum AVPixelFormat m_pix_fmt;

        AVFrame *frame;
        AVCodec *codec;
        AVCodecContext *m_codec_context = NULL;

        std::list<PCQueue<uint8_t*>> m_stream_out;
    };
}
