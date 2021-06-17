#include "Postprocessing.h"
#include "Texture.h"

#include "gl.h"
#include "imgui.h"

PostProcessing::PostProcessing(unsigned int width, unsigned int height)
{
    finalColorTex = new Texture(width, height);
    finalColorTex->InitColor();

    // create
    GLCall(glGenFramebuffers(1, &fboID));

    // attach
    GLCall(glBindFramebuffer(GL_FRAMEBUFFER, fboID));
    GLCall(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                  GL_TEXTURE_2D, finalColorTex->rendererID, 0));

    // depth
    glGenRenderbuffers(1, &rboID);
    glBindRenderbuffer(GL_RENDERBUFFER, rboID);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);                   // use a single renderbuffer object for both a depth AND stencil buffer.
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rboID); // now actually attach it

    // don't render color data
    // glDrawBuffer(GL_NONE);
    // glReadBuffer(GL_NONE);

    GLCall(glEnable(GL_BLEND));
    GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
    GLCall(glEnable(GL_DEPTH_TEST));
    GLCall(glClearColor(0.f, 0.f, 0.f, 1.f));

    // unbind
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

PostProcessing::~PostProcessing()
{
    delete finalColorTex;
}

void PostProcessing::Bind()
{
    // GLCall(glViewport(0, 0, width, height));
    GLCall(glBindFramebuffer(GL_FRAMEBUFFER, fboID));
    GLCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
}

void PostProcessing::UnBind()
{
    GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}

void PostProcessing::DrawDebugMenu()
{
    ImGui::Begin("post");
    {
        // Using a Child allow to fill all the space of the window.
        // It also alows customization
        ImGui::BeginChild("post");

        // Get the size of the child (i.e. the whole draw size of the windows).
        ImVec2 wsize = ImGui::GetWindowSize();
        // Because I use the texture from OpenGL, I need to invert the V from the UV.
        ImGui::Image((void *)finalColorTex->rendererID, wsize, ImVec2(0, 1), ImVec2(1, 0));
        ImGui::EndChild();
    }
    ImGui::End();
}
