#pragma once
#include "Shader.hpp"
#include "Texture.hpp"

inline void GenerateQuad(unsigned int &quadVAO, unsigned int &quadVBO)
{
    static float quadVertices[] = {
        // positions       // texCoords
        -1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
        1.0f, -1.0f, 0.0f, 1.0f, 0.0f,

        -1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        1.0f, 1.0f, 0.0f, 1.0f, 1.0f};
    // setup plane VAO
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
}

inline void DrawQuad()
{
    static float quadVertices[] = {
        // positions       // texCoords
        -1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
        1.0f, -1.0f, 0.0f, 1.0f, 0.0f,

        -1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        1.0f, 1.0f, 0.0f, 1.0f, 1.0f};

    static unsigned int quadVAO;
    static unsigned int quadVBO;
    static bool initialized = false;
    if (!initialized)
    {
        GenerateQuad(quadVAO, quadVBO);
        initialized = true;
    }
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 6);
    glBindVertexArray(0);
}

class Pass
{
protected:
    unsigned int FBO;
    Shader shaders;
    std::string vs_path;
    std::string fs_path;
    std::string gs_path;
    int vp_width;
    int vp_height;

    virtual void initializeGLResources() = 0;

public:
    Pass(int _vp_width = 0, int _vp_height = 0, std::string _vs_path = "",
         std::string _fs_path = "",
         std::string _gs_path = "")
        : vp_width(_vp_width), vp_height(_vp_height), vs_path(_vs_path), fs_path(_fs_path), gs_path(_gs_path)
    {
        shaders = Shader(vs_path.c_str(), fs_path.c_str(), gs_path.c_str());
    }
    virtual void reloadCurrentShaders()
    {
        shaders = Shader(vs_path.c_str(), fs_path.c_str(), gs_path.c_str());
        contextSetup();
    }
    void setToggle(bool status, std::string toggle)
    {
        shaders.use();
        shaders.setInt(toggle, status ? 1 : 0);
    }

    virtual void contextSetup() = 0;

    virtual void resize(int _width, int _height) = 0;
    virtual ~Pass()
    {
        glDeleteFramebuffers(1, &FBO);
    }
};

// 将最终通道渲染到屏幕
// 输出到默认FBO
class ScreenPass : public Pass
{
private:
    void initializeGLResources() {}

public:
    ScreenPass(int _vp_width, int _vp_height, std::string _vs_path,
               std::string _fs_path)
        : Pass(_vp_width, _vp_height, _vs_path, _fs_path)
    {
        initializeGLResources();
        contextSetup();
    }

    void contextSetup() {}

    void resize(int _width, int _height)
    {
        if (vp_width == _width &&
            vp_height == _height)
        {
            return;
        }
        vp_width = _width;
        vp_height = _height;

        contextSetup();
    }

    void render(unsigned int finalTextureID)
    {
        glViewport(0, 0, vp_width, vp_height);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        // 设置着色器参数
        shaders.use();
        shaders.setTextureAuto(finalTextureID, GL_TEXTURE_2D, 0, "tex_sampler");
        DrawQuad();
    }
};
