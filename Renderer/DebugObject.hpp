
#pragma once
#include "Shader.hpp"
#include <string>
#include <unordered_map>

class Object
{
public:
    std::string name; // 如何确保name 唯一? 让Name设置交给一个类管理,而不是输入名字就传给对象
    glm::mat4 modelMatrix = glm::identity<glm::mat4>();
    Object();
    virtual void draw(glm::mat4 modelMatrix, Shader &shaders) = 0;
    virtual ~Object();
    void setName(const std::string &_name);
    void setModelTransform(glm::mat4 &_transform);
};

Object::Object() {}
Object::~Object() {}
void Object::setName(const std::string &_name) { name = _name; }
void Object::setModelTransform(glm::mat4 &_transform) { modelMatrix = _transform; }

class Cube : public Object
{
private:
    std::vector<float> vertices;
    GLuint vao;
    GLuint vbo;
    std::vector<float> generateCubeVertices(glm::vec3 size = glm::vec3(1.0f));

public:
    Cube(const glm::vec3 &size, const std::string _name = "Cube");
    void draw(glm::mat4 modelMatrix, Shader &shaders) override;
    ~Cube();
};

std::vector<float> Cube::generateCubeVertices(glm::vec3 size)
{
    const glm::vec3 halfSize = size * 0.5f;
    const glm::vec3 vertices[8] = {
        {-halfSize.x, -halfSize.y, halfSize.z},
        {halfSize.x, -halfSize.y, halfSize.z},
        {halfSize.x, halfSize.y, halfSize.z},
        {-halfSize.x, halfSize.y, halfSize.z},
        {-halfSize.x, -halfSize.y, -halfSize.z},
        {halfSize.x, -halfSize.y, -halfSize.z},
        {halfSize.x, halfSize.y, -halfSize.z},
        {-halfSize.x, halfSize.y, -halfSize.z}};
    const int indices[36] = {
        0, 1, 2, 0, 2, 3,
        1, 5, 6, 1, 6, 2,
        5, 4, 7, 5, 7, 6,
        4, 0, 3, 4, 3, 7,
        4, 5, 1, 4, 1, 0,
        3, 2, 6, 3, 6, 7};
    const glm::vec3 normals[6] = {
        {0, 0, 1}, {1, 0, 0}, {0, 0, -1}, {-1, 0, 0}, {0, -1, 0}, {0, 1, 0}};
    const glm::vec2 texCoords[4] = {
        {0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f}};
    std::vector<float> vertexData;
    for (int i = 0; i < 36; ++i)
    {
        int vertexIndex = indices[i];
        int faceIndex = i / 6;
        vertexData.push_back(vertices[vertexIndex].x);
        vertexData.push_back(vertices[vertexIndex].y);
        vertexData.push_back(vertices[vertexIndex].z);
        vertexData.push_back(normals[faceIndex].x);
        vertexData.push_back(normals[faceIndex].y);
        vertexData.push_back(normals[faceIndex].z);
        int texCoordIndex = i % 6;
        if (texCoordIndex >= 4)
            texCoordIndex -= 4;
        vertexData.push_back(texCoords[texCoordIndex].x);
        vertexData.push_back(texCoords[texCoordIndex].y);
    }
    return vertexData;
}

Cube::Cube(const glm::vec3 &size, const std::string _name)
{
    setName(_name);
    vertices = generateCubeVertices(size);
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
}

void Cube::draw(glm::mat4 modelMatrix, Shader &shaders)
{
    glBindVertexArray(vao);
    shaders.setMat4("model", modelMatrix);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}

Cube::~Cube() {}

class WireframeCube : public Object
{
private:
    std::vector<float> vertices;
    GLuint vao;
    GLuint vbo;
    std::vector<float> generateWireframeCubeVertices(glm::vec3 size = glm::vec3(1.0f));

public:
    WireframeCube(const glm::vec3 &size, const std::string _name = "WireframeCube");
    void draw(glm::mat4 modelMatrix, Shader &shaders) override;
    ~WireframeCube();
};

std::vector<float> WireframeCube::generateWireframeCubeVertices(glm::vec3 size)
{
    const glm::vec3 halfSize = size * 0.5f;
    
    // 立方体的8个顶点
    const glm::vec3 vertices[8] = {
        {-halfSize.x, -halfSize.y, -halfSize.z}, // 0: 左下后
        { halfSize.x, -halfSize.y, -halfSize.z}, // 1: 右下后
        { halfSize.x,  halfSize.y, -halfSize.z}, // 2: 右上后
        {-halfSize.x,  halfSize.y, -halfSize.z}, // 3: 左上后
        {-halfSize.x, -halfSize.y,  halfSize.z}, // 4: 左下前
        { halfSize.x, -halfSize.y,  halfSize.z}, // 5: 右下前
        { halfSize.x,  halfSize.y,  halfSize.z}, // 6: 右上前
        {-halfSize.x,  halfSize.y,  halfSize.z}  // 7: 左上前
    };
    
    // 立方体的12条边，每条边用2个顶点索引表示
    const int edges[24] = {
        // 后面的4条边
        0, 1,  1, 2,  2, 3,  3, 0,
        // 前面的4条边  
        4, 5,  5, 6,  6, 7,  7, 4,
        // 连接前后面的4条边
        0, 4,  1, 5,  2, 6,  3, 7
    };
    
    std::vector<float> vertexData;
    
    for (int i = 0; i < 24; ++i)
    {
        int vertexIndex = edges[i];
        const glm::vec3& vertex = vertices[vertexIndex];
        
        // 位置数据 (x, y, z)
        vertexData.push_back(vertex.x);
        vertexData.push_back(vertex.y);
        vertexData.push_back(vertex.z);
        
        // 法线数据 (对于线框，可以使用顶点位置的归一化作为法线)
        glm::vec3 normal = glm::normalize(vertex);
        vertexData.push_back(normal.x);
        vertexData.push_back(normal.y);
        vertexData.push_back(normal.z);
        
        // 纹理坐标 (线框不需要纹理，设为0)
        vertexData.push_back(0.0f);
        vertexData.push_back(0.0f);
    }
    
    return vertexData;
}

WireframeCube::WireframeCube(const glm::vec3 &size, const std::string _name)
{
    setName(_name);
    vertices = generateWireframeCubeVertices(size);
    
    // 生成并绑定VAO和VBO
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    
    // 位置属性 (location = 0)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    // 法线属性 (location = 1)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    // 纹理坐标属性 (location = 2)
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    
    glBindVertexArray(0);
}

void WireframeCube::draw(glm::mat4 modelMatrix, Shader &shaders)
{
    glBindVertexArray(vao);
    shaders.setMat4("model", modelMatrix);
    
    // 使用GL_LINES渲染线框，24个顶点组成12条线
    glDrawArrays(GL_LINES, 0, 24);
    
    glBindVertexArray(0);
}

WireframeCube::~WireframeCube() 
{
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
}


