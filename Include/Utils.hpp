#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <cmath>
#include <iostream>
#include <format>
#include <memory>
#include <string>
#include <chrono>
#include <unordered_map>

#include "imgui.h"

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

inline GLuint GetTextureSizeLimit()
{
    GLint limit;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &limit);
    return limit;
}

inline vec3 DirectionOf(float x, float y, float z)
{
    return normalize(vec3(x, y, z));
}

inline vec3 DirectionOf(const vec3& v)
{
    return normalize(v);
}
inline vec3 DirectionOf(const point3& end, const point3& ori)
{
    return normalize(end - ori);
}

std::string GetCurrentWorkingDirectory();


namespace SimplifiedData
{
    void DumpFlatFloatData(const float* data, size_t size, std::string path);
    std::string DumpFlatFloatDataString(const float* data, size_t size);
}

namespace Output
{
    bool CreateParentDirectories(const std::string& filepath);

    void ExportShaderSource(const std::string& filename, const std::string& source, bool readonly);

    std::string GetFilenameNoExtension(const std::string& path_str);

} // namespace Output
glm::vec3 hsv2rgb(const glm::vec3& hsv);
inline ImVec2 operator-(const ImVec2& v1, const ImVec2& v2){
    return ImVec2(v1.x - v2.x, v1.y - v2.y);
}
inline ImVec2 operator+(const ImVec2& v1, const ImVec2& v2){
    return ImVec2(v1.x + v2.x, v1.y + v2.y);
}
inline ImVec2 operator*(const ImVec2& v, float scalar){
    return ImVec2(v.x * scalar, v.y * scalar);
}
inline ImVec2 operator/(const ImVec2& v, float scalar){
    return ImVec2(v.x / scalar, v.y / scalar);
}
inline ImVec4 operator-(const ImVec4& v1, const ImVec4& v2){
    return ImVec4(v1.x - v2.x, v1.y - v2.y, v1.z - v2.z, v1.w - v2.w);
}
inline ImVec4 operator+(const ImVec4& v1, const ImVec4& v2){
    return ImVec4(v1.x + v2.x, v1.y + v2.y, v1.z + v2.z, v1.w + v2.w);
}
inline ImVec4 operator*(const ImVec4& v, float scalar){
    return ImVec4(v.x * scalar, v.y * scalar, v.z * scalar, v.w * scalar);
}
inline ImVec4 operator/(const ImVec4& v, float scalar){
    return ImVec4(v.x / scalar, v.y / scalar, v.z / scalar, v.w / scalar);
}
inline ImVec2 ToImVec2(const glm::vec2& v) {
    return ImVec2(v.x, v.y);
}
inline glm::vec2 ToGlmVec2(const ImVec2& v) {
    return glm::vec2(v.x, v.y);
}
inline ImVec4 ToImVec4(const glm::vec4& v) {
    return ImVec4(v.x, v.y, v.z, v.w);
}
inline glm::vec4 ToGlmVec4(const ImVec4& v) {
    return glm::vec4(v.x, v.y, v.z, v.w);
}