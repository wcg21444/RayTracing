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

// 绘制公共球体. 用于生成cubemap或处理cubemap
// 使用cubemapSphere.vs 作为顶点着色器
inline void DrawSphere()
{
    // 静态变量，用于存储球体的VAO和顶点数量，确保只生成一次
    static unsigned int sphereVAO = 0;
    static size_t indexCount;

    // 如果球体VAO尚未生成，则进行初始化
    if (sphereVAO == 0)
    {
        glGenVertexArrays(1, &sphereVAO);

        unsigned int vbo, ebo;
        glGenBuffers(1, &vbo);
        glGenBuffers(1, &ebo);

        std::vector<glm::vec3> positions;
        std::vector<glm::vec2> uv;
        std::vector<glm::vec3> normals;
        std::vector<unsigned int> indices;

        const unsigned int X_SEGMENTS = 64; // 经度分段数
        const unsigned int Y_SEGMENTS = 64; // 纬度分段数
        const float PI = 3.14159265359f;

        // 生成顶点数据
        for (unsigned int y = 0; y <= Y_SEGMENTS; ++y)
        {
            for (unsigned int x = 0; x <= X_SEGMENTS; ++x)
            {
                float xSegment = (float)x / (float)X_SEGMENTS;
                float ySegment = (float)y / (float)Y_SEGMENTS;
                float xPos = std::cos(xSegment * 2.0f * PI) * std::sin(ySegment * PI);
                float yPos = std::cos(ySegment * PI);
                float zPos = std::sin(xSegment * 2.0f * PI) * std::sin(ySegment * PI);

                positions.push_back(glm::vec3(xPos, yPos, zPos));
                uv.push_back(glm::vec2(xSegment, ySegment));
                normals.push_back(glm::vec3(xPos, yPos, zPos)); // 球体的法线和位置向量相同
            }
        }

        // 生成索引数据（用于绘制三角形）
        for (unsigned int y = 0; y < Y_SEGMENTS; ++y)
        {
            for (unsigned int x = 0; x < X_SEGMENTS; ++x)
            {
                indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
                indices.push_back(y * (X_SEGMENTS + 1) + x);
                indices.push_back(y * (X_SEGMENTS + 1) + x + 1);

                indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
                indices.push_back(y * (X_SEGMENTS + 1) + x + 1);
                indices.push_back((y + 1) * (X_SEGMENTS + 1) + x + 1);
            }
        }

        indexCount = indices.size();

        // 绑定VAO，然后绑定并填充VBO和EBO
        glBindVertexArray(sphereVAO);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(glm::vec3) + uv.size() * sizeof(glm::vec2) + normals.size() * sizeof(glm::vec3), NULL, GL_STATIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, positions.size() * sizeof(glm::vec3), &positions[0]);
        glBufferSubData(GL_ARRAY_BUFFER, positions.size() * sizeof(glm::vec3), uv.size() * sizeof(glm::vec2), &uv[0]);
        glBufferSubData(GL_ARRAY_BUFFER, positions.size() * sizeof(glm::vec3) + uv.size() * sizeof(glm::vec2), normals.size() * sizeof(glm::vec3), &normals[0]);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

        // 设置顶点属性指针
        // 位置属性
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void *)0);
        // 纹理坐标属性
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void *)(positions.size() * sizeof(glm::vec3)));
        // 法线属性
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void *)(positions.size() * sizeof(glm::vec3) + uv.size() * sizeof(glm::vec2)));
    }

    // 绘制球体
    glBindVertexArray(sphereVAO);
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indexCount), GL_UNSIGNED_INT, 0);
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
        : vp_width(_vp_width), vp_height(_vp_height), vs_path(_vs_path), fs_path(_fs_path), gs_path(_gs_path),
          shaders(_vs_path.c_str(), _fs_path.c_str(), _gs_path.c_str())
    {
        // shaders = Shader(vs_path.c_str(), fs_path.c_str(), gs_path.c_str());
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

    void render(unsigned int outputLayer0, unsigned int outputLayer1)
    {
        glViewport(0, 0, vp_width, vp_height);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        // 设置着色器参数
        shaders.use();
        shaders.setTextureAuto(outputLayer0, GL_TEXTURE_2D, 0, "outputLayer0");
        shaders.setTextureAuto(outputLayer1, GL_TEXTURE_2D, 0, "outputLayer1");
        DrawQuad();
    }
};
