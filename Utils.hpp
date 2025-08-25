#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <cmath>
#include <iostream>
#include <format>

using point3 = glm::vec3;
using poinn2 = glm::vec2;
using vec2 = glm::vec2;
using vec3 = glm::vec3;
using vec4 = glm::vec4;
using mat3 = glm::mat3;
using mat4 = glm::mat4;
using color4 = glm::vec4;
using color3 = glm::vec3;

inline void CheckGLErrors()
{
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR)
    {
        std::cout << "OpenGL error: " << err << std::endl;
    }
}