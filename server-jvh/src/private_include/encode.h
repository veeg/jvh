

namespace jvh
{
    class Encoder
    {
    public:
        Encoder();
        ~Encoder();

        uint8_t *EncodeFrame (uint8_t *frame, int codec_id);

    private:

    };
}
