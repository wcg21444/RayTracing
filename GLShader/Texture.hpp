#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>
#include <string>
#include <iostream>
#include <cassert>

#include "GLResource.hpp"

using TextureID = unsigned int;
/// @brief 管理GL Texture资源
class Texture2D : public GLResource
{
public:
    GLenum Target;         // 纹理类型，如 GL_TEXTURE_2D、GL_TEXTURE_3D 等
    GLenum InternalFormat; // 纹理在 GPU 中存储的内部格式（例如：GL_RGBA8）
    GLenum Format;         // 纹理在 CPU 中存储的格式（例如：GL_RGBA）
    GLenum Type;           // 纹素数据的数据类型（例如：GL_UNSIGNED_BYTE）
    GLenum FilterMin;      // 纹理缩小时使用的过滤方式
    GLenum FilterMax;      // 纹理放大时使用的过滤方式
    GLenum WrapS;          // S 轴（水平）的纹理环绕方式
    GLenum WrapT;          // T 轴（垂直）的纹理环绕方式
    GLenum WrapR;          // R 轴（深度）的纹理环绕方式，主要用于 3D 纹理
    bool Mipmapping;       // 是否启用多级渐远纹理（Mipmapping）

    unsigned int Width;  // 纹理的宽度（以像素为单位）
    unsigned int Height; // 纹理的高度（以像素为单位）

    Texture2D();
    Texture2D(Texture2D &&) noexcept = default;
    Texture2D &operator=(Texture2D &&) noexcept = default;
    void generate(unsigned int width, unsigned int height, GLenum internalFormat, GLenum format, GLenum type, void *data, bool mipMapping = true);
    void generateComputeStorage(unsigned int width, unsigned int height, GLenum internalFormat);

    void setData(void *data);

    void setFilterMin(GLenum filter);

    void setFilterMax(GLenum filter);

    void resize(int ResizeWidth, int ResizeHeight);
    void resizeComputeStorage(int ResizeWidth, int ResizeHeight);

    void setWrapMode(GLenum wrapMode);

    ~Texture2D();
};

// Texture是对Texture GL对象的封装
// GL对象 包括 对象ID 内部格式 格式 数据类型 这些初始化属性; 以及状态属性,Filter,Wrap是对使用方开放的.
//  方法 生成Texture : 初始化资源
//       设置状态
//       设置数据
//       调整大小
/// @brief 管理GL CubeTexture资源
class TextureCube : public GLResource
{
public:
    enum FaceEnum
    {
        Right = GL_TEXTURE_CUBE_MAP_POSITIVE_X,
        Left = GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
        Top = GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
        Bottom = GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
        Front = GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
        Back = GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
    };
    //{ Right, Left, Top, Bottom, Front, Back }
    inline static const std::vector<FaceEnum> FaceTargets = {Right, Left, Top, Bottom, Front, Back};

    inline static std::vector<glm::mat4> GenearteViewMatrices(const glm::vec3 &position)
    {
        std::vector<glm::mat4> viewMatrices;
        viewMatrices.push_back(
            glm::lookAt(position, position + glm::vec3(1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0)));
        viewMatrices.push_back(
            glm::lookAt(position, position + glm::vec3(-1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0)));
        viewMatrices.push_back(
            glm::lookAt(position, position + glm::vec3(0.0, 1.0, 0.0), glm::vec3(0.0, 0.0, 1.0)));
        viewMatrices.push_back(
            glm::lookAt(position, position + glm::vec3(0.0, -1.0, 0.0), glm::vec3(0.0, 0.0, -1.0)));
        viewMatrices.push_back(
            glm::lookAt(position, position + glm::vec3(0.0, 0.0, 1.0), glm::vec3(0.0, -1.0, 0.0)));
        viewMatrices.push_back(
            glm::lookAt(position, position + glm::vec3(0.0, 0.0, -1.0), glm::vec3(0.0, -1.0, 0.0)));
        return viewMatrices;
    }

private:
    GLenum Target;         // 纹理类型
    GLenum InternalFormat; // 纹理在 GPU 中存储的内部格式（例如：GL_RGBA8）
    GLenum Format;         // 纹理在 CPU 中存储的格式（例如：GL_RGBA）
    GLenum Type;           // 纹素数据的数据类型（例如：GL_UNSIGNED_BYTE）
    GLenum FilterMin;      // 纹理缩小时使用的过滤方式
    GLenum FilterMax;      // 纹理放大时使用的过滤方式
    GLenum WrapS;          // S 轴（水平）的纹理环绕方式
    GLenum WrapT;          // T 轴（垂直）的纹理环绕方式
    GLenum WrapR;          // R 轴（深度）的纹理环绕方式，主要用于 3D 纹理
    bool Mipmapping;       // 是否启用多级渐远纹理（Mipmapping）
public:
    TextureID ID; // 纹理对象的 ID，由 OpenGL 分配

    unsigned int Width;  // 正方形纹理的宽度（以像素为单位）
    unsigned int Height; // 正方形纹理的高度（以像素为单位）

    TextureCube();
    TextureCube(TextureCube &&) noexcept = default;
    TextureCube &operator=(TextureCube &&) noexcept = default;

    void generate(unsigned int width, unsigned int height, GLenum internalFormat, GLenum format, GLenum type, GLenum filterMax, GLenum filterMin, bool mipmap);

    void setFaceData(FaceEnum faceTarget, void *data);

    void setFilterMin(GLenum filter);

    void setFilterMax(GLenum filter);

    void resize(int ResizeWidth, int ResizeHeight);

    void setWrapMode(GLenum wrapMode);

    ~TextureCube();
};

class Texture2DArray : public GLResource
{
public:
    GLenum Target;         // 纹理类型，如 GL_TEXTURE_2D、GL_TEXTURE_3D 等
    GLenum InternalFormat; // 纹理在 GPU 中存储的内部格式（例如：GL_RGBA8）
    GLenum Format;         // 纹理在 CPU 中存储的格式（例如：GL_RGBA）
    GLenum Type;           // 纹素数据的数据类型（例如：GL_UNSIGNED_BYTE）
    GLenum FilterMin;      // 纹理缩小时使用的过滤方式
    GLenum FilterMax;      // 纹理放大时使用的过滤方式
    GLenum WrapS;          // S 轴（水平）的纹理环绕方式
    GLenum WrapT;          // T 轴（垂直）的纹理环绕方式
    GLenum WrapR;          // R 轴（深度）的纹理环绕方式，主要用于 3D 纹理
    bool Mipmapping;       // 是否启用多级渐远纹理（Mipmapping）

    unsigned int Width;  // 纹理的宽度（以像素为单位）
    unsigned int Height; // 纹理的高度（以像素为单位）
    unsigned int Depth;  // 纹理层数

    Texture2DArray();
    Texture2DArray(Texture2DArray &&) noexcept = default;
    Texture2DArray &operator=(Texture2DArray &&) noexcept = default;
    void generate(unsigned int width, unsigned int height, unsigned int depth, GLenum internalFormat, GLenum format, GLenum type, void *data, bool mipMapping = true);

    void setData(void *data, unsigned int layer);

    void setFilterMin(GLenum filter);

    void setFilterMax(GLenum filter);

    void resize(int ResizeWidth, int ResizeHeight);

    void setWrapMode(GLenum wrapMode);

    ~Texture2DArray();
};