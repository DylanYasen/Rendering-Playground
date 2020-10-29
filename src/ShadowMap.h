#include "Math.h"

class ShadowMap
{
public:
    ShadowMap(/* args */);
    ~ShadowMap();

    void Bind(const mat4& lightSpaceMatrix);
    void UnBind();

    class Shader *depthShader;
    class Texture *depthMap;

    unsigned int fboID;

    void DrawDebugMenu();
};