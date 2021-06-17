class PostProcessing
{
public:
    PostProcessing(unsigned int width, unsigned int height);
    ~PostProcessing();

    void Bind();
    void UnBind();

    void DrawDebugMenu();

    // class Shader *depthShader;
    class Texture *finalColorTex;

    unsigned int fboID;
    unsigned int rboID;
};