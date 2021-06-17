#include "Scene.h"
#include "Renderable.h"

#include "Camera.h"
#include "Light.h"
#include "Shader.h"

// shadow map temp
#include "ShadowMap.h"
#include "Postprocessing.h"
#include "gl.h"
#include "FullscreenQuad.h"
#include "Texture.h"
#include "Quad.h"

// debug
#include "imgui.h"

const int WIDTH = 1024;
const int HEIGHT = 768;

FullscreenQuad *quad = nullptr;

Quad *ground = nullptr;

Scene::Scene(std::string name)
    : m_name(name)
{
    shadowMap = new ShadowMap();
    postProcessing = new PostProcessing(WIDTH, HEIGHT);

    {
        diffuseShader = new Shader("resources/shaders/material.shader");
        shadowMapShader = new Shader("resources/shaders/shadowMap.shader");
        defaultShader = new Shader("resources/shaders/default.shader");
        postPorcessShader = new Shader("resources/shaders/postprocess.shader");
    }

    quad = new FullscreenQuad();

    ground = new Quad();
    ground->m_transform.scale = vec3(100, 100, 1);
    ground->m_transform.rotation = glm::angleAxis(glm::radians(90.0f), vec3(1, 0, 0));

    {
        diffuseShader->Bind();
        diffuseShader->SetUniform1i("shadowMap", shadowMap->depthMap->rendererID);
        diffuseShader->Unbind();
    }

    {
        defaultShader->Bind();
        defaultShader->SetUniform1i("shadowMap", shadowMap->depthMap->rendererID);
        defaultShader->Unbind();
    }

    {
        postPorcessShader->Bind();
        postPorcessShader->SetUniform1i("colorOutput", postProcessing->finalColorTex->rendererID);
        postPorcessShader->Unbind();
    }
}

Scene::~Scene()
{
    for (auto &r : m_renderables)
    {
        r->Destroy();
    }

    delete diffuseShader;
    delete shadowMapShader;
    delete defaultShader;
    delete postPorcessShader;

    delete shadowMap;
    delete quad;
}

void Scene::SetCamera(Camera *camera)
{
    m_camera = camera;

    const mat4 &vp = camera->projMatrix * camera->viewMatrix;

    {
        diffuseShader->Bind();
        diffuseShader->SetUniform3f("viewPos", camera->eyePos);
        diffuseShader->SetUniformMat4f("u_vp", vp);
        diffuseShader->Unbind();
    }

    {
        defaultShader->Bind();
        // defaultShader->SetUniform3f("viewPos", camera->eyePos);
        defaultShader->SetUniformMat4f("u_vp", vp);
        defaultShader->Unbind();
    }
}

void Scene::SetLight(Light *light)
{
    m_light = light;

    {
        diffuseShader->Bind();
        diffuseShader->SetUniform3f("light.ambient", light->ambient);
        diffuseShader->SetUniform3f("light.diffuse", light->diffuse);
        diffuseShader->SetUniform3f("light.specular", light->specular);
        diffuseShader->SetUniform3f("lightPos", light->transform.position);
        diffuseShader->SetUniformMat4f("lightSpaceMatrix", light->lightSpaceMatrix);
        diffuseShader->Unbind();
    }

    {
        shadowMapShader->Bind();
        shadowMapShader->SetUniformMat4f("lightSpaceMatrix", light->lightSpaceMatrix);
        shadowMapShader->Unbind();
    }

    {
        defaultShader->Bind();
        defaultShader->SetUniformMat4f("lightSpaceMatrix", light->lightSpaceMatrix);
        defaultShader->Unbind();
    }
}

void Scene::Render(Renderer *renderer)
{
    static const auto renderScene = [&](Shader *shader) {
        for (auto &r : m_renderables)
        {
            shader->Bind();
            r->PreRender(this, renderer, shader);
            r->Render(this, renderer, shader);
            shader->Unbind();
        };
    };

    // render shadow map
    shadowMap->Bind(m_light->lightSpaceMatrix);
    renderScene(shadowMapShader);
    {
        shadowMapShader->Bind();
        ground->PreRender(this, renderer, shadowMapShader);
        ground->Render(this, renderer, shadowMapShader);
        shadowMapShader->Unbind();
    }
    shadowMap->UnBind();

    // reder scene normal
    postProcessing->Bind();
    {
        GLCall(glViewport(0, 0, WIDTH, HEIGHT));
        // GLCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

        shadowMap->depthMap->Bind();

        renderScene(diffuseShader);

        // ground
        {
            defaultShader->Bind();
            ground->PreRender(this, renderer, defaultShader);
            ground->Render(this, renderer, defaultShader);
            defaultShader->Unbind();
        }

        shadowMap->depthMap->Unbind();
    }
    postProcessing->UnBind();

    postPorcessShader->Bind();
    postProcessing->finalColorTex->Bind();
    quad->Render(this, renderer, postPorcessShader);
    postProcessing->finalColorTex->Unbind();
    postPorcessShader->Unbind();

    // debug
    {
        shadowMap->DrawDebugMenu();
        postProcessing->DrawDebugMenu();

        ImGui::Begin("scene", 0, ImGuiWindowFlags_AlwaysAutoResize);
        if (m_camera->DrawDebugMenu())
            SetCamera(m_camera);
        if (m_light->DrawDebugMenu())
            SetLight(m_light);
        ImGui::End();
    }
}
