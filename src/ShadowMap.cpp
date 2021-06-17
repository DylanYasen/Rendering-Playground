#include "ShadowMap.h"
#include "Texture.h"

#include "imgui.h"

#include "gl.h"

#include "Shader.h"

auto constexpr width = 1024;
auto constexpr height = 1024;

ShadowMap::ShadowMap(/* args */)
{
    depthMap = new Texture(width, height);
    depthMap->InitDepthAttachment();

    // create
    GLCall(glGenFramebuffers(1, &fboID));

    // attach
    GLCall(glBindFramebuffer(GL_FRAMEBUFFER, fboID));
    GLCall(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap->rendererID, 0));

    // don't render color data
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    // unbind
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    depthShader = new Shader("resources/shaders/depth.shader");
}

ShadowMap::~ShadowMap()
{
    delete depthMap;
}

void ShadowMap::Bind(const mat4 &lightSpaceMatrix)
{
    GLCall(glViewport(0, 0, width, height));
    GLCall(glBindFramebuffer(GL_FRAMEBUFFER, fboID));
    GLCall(glClear(GL_DEPTH_BUFFER_BIT));
}

void ShadowMap::UnBind()
{
    GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}

void ShadowMap::DrawDebugMenu()
{
    ImGui::Begin("shadow map");
    {
        // Using a Child allow to fill all the space of the window.
        // It also alows customization
        ImGui::BeginChild("dept");

        // Get the size of the child (i.e. the whole draw size of the windows).
        ImVec2 wsize = ImGui::GetWindowSize();
        // Because I use the texture from OpenGL, I need to invert the V from the UV.
        ImGui::Image((void *)depthMap->rendererID, wsize, ImVec2(0, 1), ImVec2(1, 0));
        ImGui::EndChild();
    }
    ImGui::End();
}
