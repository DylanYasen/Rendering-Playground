#include "FrameBuffer.h"
#include "gl.h"
#include "assert.h"
#include "Texture.h"

FrameBuffer::FrameBuffer(/* args */)
{
    GLCall(glGenFramebuffers(1, &ID));
    GLCall(glBindFramebuffer(GL_FRAMEBUFFER, ID));

    const GLenum drawBuffers[1] = {GL_COLOR_ATTACHMENT0};
    GLCall(glDrawBuffers(1, drawBuffers));
}

void FrameBuffer::Attach(AttachPoint attachPoint, Texture *texture)
{
    assert(texture);

    GLenum attachment = GL_COLOR_ATTACHMENT0;
    switch (attachPoint)
    {
    case AttachPoint::Color:
        attachment = GL_COLOR_ATTACHMENT0;
        break;

    case AttachPoint::Depth:
        attachment = GL_DEPTH;
        break;

    case AttachPoint::DepthStencil:
        attachment = GL_DEPTH24_STENCIL8;
        break;

    default:
        break;
    }

    unsigned int buffer;
    GLCall(glGenRenderbuffers(1, &buffer));
    GLCall(glBindRenderbuffer(GL_RENDERBUFFER, buffer));
    GLCall(glRenderbufferStorage(GL_RENDERBUFFER, attachment, texture->width, texture->height));
    GLCall(glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachment, GL_RENDERBUFFER, buffer));
    GLCall(glFramebufferTexture(GL_FRAMEBUFFER, attachment, texture->rendererID, 0));
}

FrameBuffer::~FrameBuffer()
{
    UnBind();
}

void FrameBuffer::Bind()
{
    GLCall(glBindFramebuffer(GL_FRAMEBUFFER, ID));
}

void FrameBuffer::UnBind()
{
    // GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}