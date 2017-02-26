
namespace capture
{
    class StreamComm
    {
    public:
        StreamComm (uint32_t port);
        ~StreamComm ();
    private:
        ProtoTCP *m_protoTCP;


    };
}
