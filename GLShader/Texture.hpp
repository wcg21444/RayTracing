#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <iostream>

/// @brief 管理GL Texture资源
class Texture
{
public:
    unsigned int ID = 0;
    GLenum Target = GL_TEXTURE_2D;   // what type of texture we're dealing with
    GLenum InternalFormat = GL_RGBA; // number of color components
    GLenum Format = GL_RGBA;         // the format each texel is stored in
    GLenum Type = GL_UNSIGNED_BYTE;
    GLenum FilterMin = GL_LINEAR_MIPMAP_LINEAR; // what filter method to use during minification
    GLenum FilterMax = GL_LINEAR;               // what filter method to use during magnification
    GLenum WrapS = GL_REPEAT;                   // wrapping method of the S coordinate
    GLenum WrapT = GL_REPEAT;                   // wrapping method of the T coordinate
    GLenum WrapR = GL_REPEAT;                   // wrapping method of the R coordinate
    bool Mipmapping = true;

    unsigned int Width = 0;
    unsigned int Height = 0;

    void Generate(unsigned int width, unsigned int height, GLenum internalFormat, GLenum format, GLenum type, void *data);
    void SetData(void *data);

    void SetFilterMin(GLenum filter);

    void SetFilterMax(GLenum filter);

    void Resize(int ResizeWidth, int ResizeHeight);

    void SetWrapMode(GLenum wrapMode);

    Texture();

    ~Texture();
};