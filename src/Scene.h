#include <vector>
#include <string>

class Renderable;
class Camera;
class Light;
class Renderer;
class ShadowMap;

class Scene
{

public:
    Scene(std::string name = "untitled");
    ~Scene();

    void Render(Renderer *renderer);

    void AddRenderable(Renderable *r) { m_renderables.push_back(r); };
    void SetCamera(Camera *camera);
    void SetLight(Light *light);

    void UseDiffuseShader();

    Camera *GetCamera() const { return m_camera; }
    Light *GetLight() const { return m_light; }

private:
    std::vector<Renderable *> m_renderables;
    Camera *m_camera;
    Light *m_light;
    std::string m_name;

    ShadowMap *shadowMap;

    class PostProcessing* postProcessing;

    class Shader *diffuseShader;
    class Shader *shadowMapShader;
    class Shader *defaultShader;
    class Shader *postPorcessShader;
};
