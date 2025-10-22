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

namespace Profiler
{
    // 用两个方法, 划分开始结束区间 , 监测区间内代码耗时,每个区间用户自定义名字
    // 输出区间耗时到imgui
    using namespace std::chrono;
    struct TimeBeginEnd
    {
        high_resolution_clock::time_point startTime;
        high_resolution_clock::time_point endTime;
    };
    extern std::unordered_map<std::string, TimeBeginEnd> TimeBlocks;

    //可以考虑改成配对的方式,Begin传入名字,这应该需要一个栈来维护
    void BeginTimeBlock(const std::string& name);
    void EndTimeBlock(const std::string& name);

    void RenderUI();
} // namespace Profiler