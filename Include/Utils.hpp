#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <cmath>
#include <iostream>
#include <format>

using point3 = glm::vec3;
using point2 = glm::vec2;
using vec2 = glm::vec2;
using vec3 = glm::vec3;
using vec4 = glm::vec4;
using mat3 = glm::mat3;
using mat4 = glm::mat4;
using color4 = glm::vec4;
using color3 = glm::vec3;

const color4 RED = color4(1.0f, 0.0f, 0.0f, 1.0f);
const color4 GREEN = color4(0.0f, 1.0f, 0.0f, 1.0f);
const color4 BLUE = color4(0.0f, 0.0f, 1.0f, 1.0f);

inline void CheckGLErrors()
{
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR)
    {
        std::cout << "OpenGL error: " << err << std::endl;
    }
}

inline vec3 DirectionOf(float x, float y, float z)
{
    return normalize(vec3(x, y, z));
}

inline vec3 DirectionOf(const vec3 &v)
{
    return normalize(v);
}
inline vec3 DirectionOf(const point3 &end, const point3 &ori)
{
    return normalize(end - ori);
}

