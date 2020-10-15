#include <vector>
#include <string>

class Renderable;
class Camera;
class Light;
class Renderer;

class Scene
{

public:
    Scene(std::string name = "untitled");
    ~Scene();

    void Render(Renderer* renderer) const;

    void AddRenderable(Renderable *r) { m_renderables.push_back(r); };
    void SetCamera(Camera *camera) { m_camera = camera; };
    void SetLight(Light *light) { m_light = light; }

    Camera *GetCamera() const { return m_camera; }
    Light *GetLight() const { return m_light; }

private:
    std::vector<Renderable *> m_renderables;
    Camera *m_camera;
    Light *m_light;
    std::string m_name;
};
