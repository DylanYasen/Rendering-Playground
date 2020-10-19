#include "gl.h"

#include <stdio.h>

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_set>

#include "FileUtils.h"
#include "IndexBuffer.h"
#include "Renderer.h"
#include "Shader.h"
#include "Texture.h"
#include "VertexArray.h"
#include "VertexBuffer.h"
#include "VertexBufferLayout.h"

#include "Scene.h"
#include "Asset.h"
#include "Light.h"

// imgui
#include "Camera.h"
#include "Math.h"
#include "Transform.h"
#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_sdl.h"

#include "Allocator.h"

const int WIDTH = 1024;
const int HEIGHT = 768;

void glClearError()
{
    while (glGetError() != GL_NO_ERROR)
        ;
}

bool glCheckError(const char *func, const char *file, int line)
{
    while (GLenum error = glGetError())
    {
        printf("[OpenGL Error] code:[%d] file:[%s] func:[%s] line:[%d]\n", error,
               file, func, line);
        return false;
    }
    return true;
}

namespace GL
{
    VertexBuffer vb;
    IndexBuffer ib;
    VertexArray *va;
    Shader *shader;
    Renderer *renderer;
    Texture *texture;

    bool bInit = false;
    SDL_Window *window = NULL;
    int location = -1;

    Scene *scene = nullptr;
    Camera *camera = nullptr;

    namespace triangle
    {
        bool init()
        {
            shader = new Shader("resources/shaders/basic.shader");

            GLCall(glClearColor(0.f, 0.f, 0.f, 1.f));

            const float vertexData[] = {-0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.5f, -0.5f, 0.0f,
                                        1.0f, 0.0f, 0.0f, 0.5f, 0.0f, 0.0f, 1.0f};

            va = new VertexArray();
            {
                vb = VertexBuffer(vertexData, 5 * 3 * sizeof(float));

                VertexBufferLayout layout;
                layout.Push<float>(2); // pos
                layout.Push<float>(3); // color

                va->AddBuffer(vb, layout);
            }

            const unsigned int indexData[] = {0, 1, 2};
            ib = IndexBuffer(indexData, 3);

            return true;
        }

        float r = 0.0f;
        float increment = 0.05f;

        void draw()
        {
            renderer->Clear();

            shader->Bind();
            shader->SetUniform4f("u_color", r, 0.1f, 0.1f, 1.0f);

            renderer->Render(*va, ib, *shader);

            if (r > 1.0f)
            {
                increment = -0.05f;
            }
            else if (r < 0.0f)
            {
                increment = 0.05f;
            }
            r += increment;
        }
    } // namespace triangle

    namespace rect
    {
        bool init()
        {
            // GLCall(glClearColor(0.f, 0.f, 0.f, 1.f));

            // const float vertexData[] = {100.0f, 100.0f, 0.0f, 0.0f, 200.0f, 100.0f,
            //                             1.0f, 0.0f, 200.0f, 200.0f, 1.0f, 1.0f,
            //                             100.0f, 200.0f, 0.0f, 1.0f};

            // va = new VertexArray();
            // {
            //     vb = VertexBuffer(vertexData, sizeof(vertexData));

            //     VertexBufferLayout layout;
            //     layout.Push<float>(2);
            //     layout.Push<float>(2);
            //     va->AddBuffer(vb, layout);
            // }

            // const unsigned int indexData[] = {0, 1, 2, 2, 3, 0};
            // ib = IndexBuffer(indexData, 6);

            // mat4 proj = HMM_Orthographic(0.0f, WIDTH, 0, HEIGHT, -10, 10);
            // mat4 view = HMM_Translate(HMM_Vec3(-100, 0, 0));
            // mat4 model = HMM_Translate(HMM_Vec3(200, 200, 0));

            // mat4 mvp = proj * view * model;

            // shader = new Shader("resources/shaders/sprite.shader");
            // shader->Bind();
            // shader->SetUniformMat4f("u_mvp", mvp);

            // texture = new Texture("resources/textures/mob.PNG");
            // texture->Bind();
            // shader->SetUniform1i("u_texture", 0);

            return true;
        }

        void draw()
        {
            renderer->Clear();
            //            renderer->Draw(*va, ib, *shader);
        }
    } // namespace rect

    namespace cube
    {
        mat4 model;
        mat4 view;
        mat4 projection;

        bool init()
        {
            //            model = HMM_Scale(HMM_Vec3(1, 1, 1)) * HMM_Rotate(30, HMM_Vec3(0, 1, 0)) *
            //                    HMM_Translate(HMM_Vec3(0, 0, 0));
            //            view = HMM_Translate(HMM_Vec3(0.0f, 0.0f, -5.0f));
            //            projection = HMM_Perspective(60.0f, WIDTH / HEIGHT, 0.1f, 1000.0f);
            //
            //            shader = new Shader("resources/shaders/cube.shader");
            //            shader->Bind();
            //            shader->SetUniformMat4f("u_m", model);
            //            shader->SetUniformMat4f("u_v", view);
            //            shader->SetUniformMat4f("u_p", projection);
            //
            //            texture = new Texture("resources/textures/cube.PNG");
            //            texture->Bind();
            //            shader->SetUniform1i("u_tex", 0);
            //
            //            const float vertices[] = {
            //                -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 0.5f, -0.5f, -0.5f, 1.0f, 0.0f,
            //                0.5f, 0.5f, -0.5f, 1.0f, 1.0f, 0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
            //                -0.5f, 0.5f, -0.5f, 0.0f, 1.0f, -0.5f, -0.5f, -0.5f, 0.0f, 0.0f,
            //
            //                -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 0.5f, -0.5f, 0.5f, 1.0f, 0.0f,
            //                0.5f, 0.5f, 0.5f, 1.0f, 1.0f, 0.5f, 0.5f, 0.5f, 1.0f, 1.0f,
            //                -0.5f, 0.5f, 0.5f, 0.0f, 1.0f, -0.5f, -0.5f, 0.5f, 0.0f, 0.0f,
            //
            //                -0.5f, 0.5f, 0.5f, 1.0f, 0.0f, -0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
            //                -0.5f, -0.5f, -0.5f, 0.0f, 1.0f, -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
            //                -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, -0.5f, 0.5f, 0.5f, 1.0f, 0.0f,
            //
            //                0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
            //                0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
            //                0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 0.5f, 0.5f, 0.5f, 1.0f, 0.0f,
            //
            //                -0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 0.5f, -0.5f, -0.5f, 1.0f, 1.0f,
            //                0.5f, -0.5f, 0.5f, 1.0f, 0.0f, 0.5f, -0.5f, 0.5f, 1.0f, 0.0f,
            //                -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
            //
            //                -0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
            //                0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.5f, 0.5f, 0.5f, 1.0f, 0.0f,
            //                -0.5f, 0.5f, 0.5f, 0.0f, 0.0f, -0.5f, 0.5f, -0.5f, 0.0f, 1.0f};
            //
            //            va = new VertexArray();
            //            {
            //                vb = VertexBuffer(vertices, sizeof(vertices));
            //
            //                VertexBufferLayout layout;
            //                layout.Push<float>(3);
            //                layout.Push<float>(2);
            //                va->AddBuffer(vb, layout);
            //            }
            //
            //            glEnable(GL_DEPTH_TEST);

            return true;
        }

        void draw()
        {
            renderer->Clear();

            //            va->Bind();
            //            shader->Bind();
            //
            //            // rotate
            //            {
            //                model = model * HMM_Rotate(1.0f, HMM_Vec3(1, 1, 0));
            //                shader->SetUniformMat4f("u_m", model);
            //            }

            glDrawArrays(GL_TRIANGLES, 0, 36);
        }
    } // namespace cube

    void initScene()
    {
        // std::string filepath = "resources/models/lighthouse/source/Cotman_Sam.fbx";
        // std::string filepath = "resources/models/nanosuit/source/suit.fbx";
        std::string filepath = "resources/models/gilnean-chapel/gilneas.fbx";
        // std::string filepath = "resources/models/junkrat/junkrat.fbx";
        // std::string filepath = "resources/models/chaman-ti-pche/model.fbx";
        // std::string filepath = "resources/models/robot/robot.fbx";

        // junkrat material
        {
            //  transform.rotAngle = -90;
            //  transform.rotAxis = HMM_Vec3(1,0,0);
            //  shader->SetUniform1f("material.shininess", 32.0f);
        }

        {
            vec3 eyepos = vec3(0.0f, 100.0f, 100.0f);
            vec3 targetpos = vec3(0.0f, 0.0f, 0.0f);
            vec3 upVec = vec3(0.0f, 1.0f, 0.0f);
            camera = new Camera(80, WIDTH / HEIGHT, 1, 10000, eyepos, targetpos);
        }

        Light *light = new Light();
        {
            Transform t(vec3(200, 500, 100));
            light->transform = t;
        }

        scene = new Scene("scene1");
        {
            scene->SetCamera(camera);
            scene->SetLight(light);
        }
        {
            auto asset = new Asset(filepath);
            scene->AddRenderable(asset);
        }
    }

    bool initGL()
    {
        renderer = new Renderer();

        GLCall(glEnable(GL_BLEND));
        GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

        GLCall(glEnable(GL_DEPTH_TEST));

        GLCall(glClearColor(0.f, 0.f, 0.f, 1.f));

        initScene();

        return true;
    }

    void init(SDL_Window *win)
    {
        if (!win)
            return;
        window = win;

// Decide GL+GLSL versions
#if __APPLE__
        // GL 3.2 Core + GLSL 150
        const char *glsl_version = "#version 150";
        SDL_GL_SetAttribute(
            SDL_GL_CONTEXT_FLAGS,
            SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG); // Always required on Mac
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
#else
        // GL 3.0 + GLSL 130
        const char *glsl_version = "#version 130";
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#endif

        int maxSamples = 0;
        {
            // todo: unfortunately can only do this after window creation.
            // don't feel like hacking a temp window just to do a query
            // glGetIntegerv ( GL_MAX_SAMPLES, &maxSamples );
            SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
            SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);
        }

        SDL_GLContext context = SDL_GL_CreateContext(window);
        if (context == NULL)
        {
            printf("OpenGL context could not be created! SDL Error: %s\n",
                   SDL_GetError());
            return;
        }

        if (!gladLoadGL())
        {
            printf("gladLoadGL failed");
            return;
        }

        printf("Vendor:   %s\n", glGetString(GL_VENDOR));
        printf("Renderer: %s\n", glGetString(GL_RENDERER));
        printf("Version:  %s\n", glGetString(GL_VERSION));

        // Vsync
        if (SDL_GL_SetSwapInterval(1) < 0)
        {
            printf("Warning: Unable to set VSync! SDL Error: %s\n", SDL_GetError());
        }

        // init imgui
        {
            IMGUI_CHECKVERSION();
            ImGui::CreateContext();
            ImGui::StyleColorsDark();
            ImGui_ImplSDL2_InitForOpenGL(window, context);
            ImGui_ImplOpenGL3_Init(glsl_version);
        }

        if (!initGL())
        {
            printf("Unable to initialize OpenGL!\n");
        }
        else
        {
            bInit = true;
        }
    }

    void render(const FrameData &frameData)
    {
        if (!bInit)
            return;

        renderer->Clear();

        // imgui
        {
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplSDL2_NewFrame(window);
            ImGui::NewFrame();
        }

        scene->Render(renderer);

        // draw frame info
        {
            ImGui::Begin("perf");
            {
                char str[30];
                sprintf(str, "frame\n%.2f ms", frameData.deltaTime);
                ImGui::PlotLines(str, frameData.frametimes, IM_ARRAYSIZE(frameData.frametimes), 0, 0,
                                 10, 20);
            }

            {
                for (int i = 0; i < (int)EResourceType::Max; i++)
                {
                    EResourceType type = (EResourceType)i;
                    size_t size = MemTracker::resourceMap[(uint8_t)type];
                    const char *str = ResourceTypeToStr(type);
                    ImGui::LabelText(str, "%.2f mb", size / 1000000.0);
                }
            }

            ImGui::End();
        }

        // imgui
        {
            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        }

        SDL_GL_SwapWindow(window);
    }

    void processInput(const SDL_Event &e) { ImGui_ImplSDL2_ProcessEvent(&e); }
} // namespace GL
