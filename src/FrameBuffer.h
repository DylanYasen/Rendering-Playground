class FrameBuffer
{
public:
    enum AttachPoint
    {
        Color = 0,
        Depth = 1,
        DepthStencil = 2
    };

    struct Attachment
    {
        class Texture *texture = nullptr;
    };

    static constexpr unsigned int ATTACHMENT_COUNT = 2;
    Attachment attachments[FrameBuffer::ATTACHMENT_COUNT];

    FrameBuffer();
    ~FrameBuffer();

    void Bind();
    void UnBind();

    void Attach(AttachPoint attachPoint, class Texture *texture);

    unsigned int ID;
};
