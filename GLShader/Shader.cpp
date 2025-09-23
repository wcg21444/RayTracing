#include "Shader.hpp"

#include <format>
#include <iostream>

// 构造函数
Shader::Shader() : texLocationID(0), programID(0) {}

Shader::Shader(const char *vs_path, const char *fs_path, const char *gs_path) : Shader()
{
    this->vs_path = vs_path;
    this->fs_path = fs_path;
    this->gs_path = gs_path ? gs_path : "";
    bool hasGS = gs_path && gs_path[0] != '\0';

    std::string shader_buf;
    unsigned int vertexShader, fragmentShader, geometryShader;

    // Config Vertex Shader
    try
    {
        shader_buf = LoadShaderFile(vs_path);
        const char *vertexShaderSource = shader_buf.c_str();
        CompileShader(vertexShaderSource, GL_VERTEX_SHADER, vertexShader);
    }
    catch (const std::exception &e)
    {
        std::cerr << "Shader Load Error (Vertex): " << e.what() << std::endl;
    }

    // Config Fragment Shader
    try
    {
        shader_buf = LoadShaderFile(fs_path);
        const char *fragmentShaderSource = shader_buf.c_str();
        CompileShader(fragmentShaderSource, GL_FRAGMENT_SHADER, fragmentShader);
    }
    catch (const std::exception &e)
    {
        std::cerr << "Shader Load Error (Fragment): " << e.what() << std::endl;
    }

    // Config Geometry Shader
    if (hasGS)
    {
        try
        {
            shader_buf = LoadShaderFile(gs_path);
            const char *geometryShadersource = shader_buf.c_str();
            CompileShader(geometryShadersource, GL_GEOMETRY_SHADER, geometryShader);
        }
        catch (const std::exception &e)
        {
            std::cerr << "Shader Load Error (Geometry): " << e.what() << std::endl;
        }
    }

    // Cofig Shader Program
    programID = glCreateProgram();
    glAttachShader(programID, vertexShader);
    glAttachShader(programID, fragmentShader);
    if (hasGS)
    {
        glAttachShader(programID, geometryShader);
    }
    glLinkProgram(programID);

    int success;
    char infoLog[512];
    glGetProgramiv(programID, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(programID, 512, NULL, infoLog);
        std::cerr << "VS : " << std::string(vs_path) << "FS : " << std::string(fs_path) << std::endl;
        std::cerr << "ERROR::SHADER::PROGRAM::LINK_FAILED\n"
                  << infoLog << std::endl;
        throw std::runtime_error("Shader program link failed.");
    }

    // Delete our used Shaders
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    if (hasGS)
    {
        glDeleteShader(geometryShader);
    }
}

// 析构函数
Shader::~Shader()
{
    if (programID)
    {
        std::cout << std::format("Shader Program ID:{} Was Deleted~\n", programID);
        glDeleteProgram(programID);
    }
}

// 移动赋值运算符
Shader &Shader::operator=(Shader &&other) noexcept
{
    if (this != &other)
    {
        if (programID)
        {
            std::cout << std::format("Shader Program ID:{} Was Deleted\n", programID);
            glDeleteProgram(programID);
        }
        this->programID = other.programID;
        this->used = other.used;
        this->vs_path = std::move(other.vs_path);
        this->fs_path = std::move(other.fs_path);
        this->gs_path = std::move(other.gs_path);
        this->textureLocationMap = std::move(other.textureLocationMap);
        this->texLocationID = other.texLocationID;
        other.programID = 0; // 释放源对象资源
        other.used = false;
        other.texLocationID = 0;
    }
    return *this;
}

std::string Shader::LoadShaderFile(const char *shader_path)
{
    return GLSLPT::ShaderInclude::load(shader_path).src;
}

GLint Shader::getUniformLocationSafe(const std::string &name)
{
    if (!used)
    {
        throw std::runtime_error("Attempted to set uniform '" + name + "' while shader is not active (glUseProgram was not called).");
    }
    if (uniformLocationMap.find(name) != uniformLocationMap.end()) // Location缓存
    {
        return uniformLocationMap.at(name);
    }
    GLint location = glGetUniformLocation(programID, name.c_str());
    if (location == -1)
    {
        if (!warningMsgSet.contains(std::format("{}{}", name, programID)))
        {
            warningMsgSet.insert(std::format("{}{}", name, programID));
            std::cout << std::format("Warning: Uniform {} not found in shader program: {} ", name, programID);
            std::cout << std::format("   Vertex Shader Path: {} ", vs_path);
            std::cout << std::format("   Fragment Shader Path: {} ", fs_path);
            if (!(gs_path == ""))
            {
                std::cout << std::format("   Geometry Shader Path: {} ", gs_path);
            }
        }
    }
    uniformLocationMap.insert({name, location});
    return location;
}

void Shader::CompileShader(const char *shader_source, GLenum shader_type, unsigned int &shader_id)
{
    shader_id = glCreateShader(shader_type);
    glShaderSource(shader_id, 1, &shader_source, NULL);
    glCompileShader(shader_id);

    int success;
    char infoLog[512];
    glGetShaderiv(shader_id, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(shader_id, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::COMPILATION_FAILED (" << shader_type << ")\n"
                  << infoLog << std::endl;
        throw std::runtime_error("Failed to compile shader.");
    }
}

// Uniform 设置方法实现
void Shader::setUniform4fv(const std::string &name, GLsizei count, const float *value)
{
    GLint location = getUniformLocationSafe(name);
    if (location != -1)
    {
        glUniform4fv(location, count, value);
    }
}

void Shader::setUniform4fv(const std::string &name, const glm::vec4 &vec4)
{
    GLint location = getUniformLocationSafe(name);
    if (location != -1)
    {
        glUniform4fv(location, 1, glm::value_ptr(vec4));
    }
}

void Shader::setUniform3fv(const std::string &name, const glm::vec3 &vec3)
{
    GLint location = getUniformLocationSafe(name);
    if (location != -1)
    {
        glUniform3fv(location, 1, glm::value_ptr(vec3));
    }
}

void Shader::setMat4(const std::string &name, const glm::mat4 &mat)
{
    GLint location = getUniformLocationSafe(name);
    if (location != -1)
    {
        glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(mat));
    }
}

void Shader::setFloat(const std::string &name, float f)
{
    GLint location = getUniformLocationSafe(name);
    if (location != -1)
    {
        glUniform1f(location, f);
    }
}

void Shader::setInt(const std::string &name, int i)
{
    GLint location = getUniformLocationSafe(name);
    if (location != -1)
    {
        glUniform1i(location, i);
    }
}

void Shader::setUniform(const std::string &name, const glm::vec4 &vec4)
{
    GLint location = getUniformLocationSafe(name);
    if (location != -1)
    {
        glUniform4fv(location, 1, glm::value_ptr(vec4));
    }
}

// 为 vec3 编写的重载
void Shader::setUniform(const std::string &name, const glm::vec3 &vec3)
{
    GLint location = getUniformLocationSafe(name);
    if (location != -1)
    {
        glUniform3fv(location, 1, glm::value_ptr(vec3));
    }
}

// 为 vec2 编写的重载 (GLM vec2)
void Shader::setUniform(const std::string &name, const glm::vec2 &vec2)
{
    GLint location = getUniformLocationSafe(name);
    if (location != -1)
    {
        glUniform2fv(location, 1, glm::value_ptr(vec2));
    }
}

// 为 mat4 编写的重载
void Shader::setUniform(const std::string &name, const glm::mat4 &mat)
{
    GLint location = getUniformLocationSafe(name);
    if (location != -1)
    {
        glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(mat));
    }
}

// 为 float 编写的重载
void Shader::setUniform(const std::string &name, float f)
{
    GLint location = getUniformLocationSafe(name);
    if (location != -1)
    {
        glUniform1f(location, f);
    }
}

// 为 int 编写的重载
void Shader::setUniform(const std::string &name, int i)
{
    GLint location = getUniformLocationSafe(name);
    if (location != -1)
    {
        glUniform1i(location, i);
    }
}

/**
 * @brief 自动管理并绑定纹理到着色器。
 * * 此函数旨在简化纹理绑定过程。它内部维护一个映射表 (textureLocationMap)，
 * 将着色器中的采样器 (sampler) 名称与一个唯一的纹理单元 ID (texLocationID) 关联起来。
 * 如果一个采样器是第一次绑定，它会被分配一个新的纹理单元。
 * 随后，函数会激活对应的纹理单元，将纹理对象绑定到该单元，
 * 并通过 glUniform1i 将该纹理单元 ID 传递给着色器中的 uniform 变量。
 * * @param textureID 要绑定的纹理对象的 OpenGL ID。
 * @param textureTarget 纹理目标类型，例如 GL_TEXTURE_2D、GL_TEXTURE_CUBE_MAP 等。
 * @param shaderTextureLocation 占位符
 * @param samplerUniformName 着色器中 sampler uniform 变量的名称，例如 "u_AlbedoMap"。
 */
void Shader::setTextureAuto(GLuint textureID, GLenum textureTarget, int shaderTextureLocation, const std::string &samplerUniformName)
{
    // 第一次为该采样器 uniform 绑定纹理
    if (textureLocationMap.find(samplerUniformName) == textureLocationMap.end())
    {
        // 将采样器名称与其唯一的纹理单元 ID 关联起来
        textureLocationMap.insert({samplerUniformName, texLocationID});
        texLocationID++;
    }

    // 从映射表中获取该采样器对应的纹理单元 ID
    int location = textureLocationMap.at(samplerUniformName);

    // 将纹理单元 ID 转换为 GL_TEXTURE0、GL_TEXTURE1 等枚举值
    GLenum activeTextureUnit = GetTextureUnitEnum(location);

    glActiveTexture(activeTextureUnit);

    glBindTexture(textureTarget, textureID);

    GLint samplerLoc = getUniformLocationSafe(samplerUniformName);

    if (samplerLoc != -1)
    {
        // 将纹理单元 ID (location) 传递给着色器中的 uniform 变量
        // 这样着色器就知道应该从哪个纹理单元读取数据
        glUniform1i(samplerLoc, location);
    }
}

// 静态工具方法实现
inline GLenum Shader::GetTextureUnitEnum(int textureLocation)
{
    if (GetTextureUnitsLimits() == -1)
    {
        throw std::runtime_error("OpenGL texture limits not initialized.");
    }
    if (textureLocation < 0 || textureLocation >= GetTextureUnitsLimits())
    {
        std::string errorMsg = "Texture location " + std::to_string(textureLocation) +
                               " out of bounds. Max texture units: " + std::to_string(GetTextureUnitsLimits()) + ".";
        throw std::out_of_range(errorMsg);
    }
    return GL_TEXTURE0 + textureLocation;
}

inline GLint Shader::GetTextureUnitsLimits()
{
    static GLint s_maxTextureUnits = -1;
    if (s_maxTextureUnits == -1)
    {
        glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &s_maxTextureUnits);
        if (s_maxTextureUnits == -1)
        {
            std::cerr << "Warning: Could not query GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS. Setting to default 32." << std::endl;
            s_maxTextureUnits = 32;
        }
    }
    return s_maxTextureUnits;
}

ComputeShader::ComputeShader()
{
}

ComputeShader::ComputeShader(const char *cs_path)
{

    this->cs_path = cs_path;

    std::string shader_buf;
    unsigned int computeShader;

    // Compile Compute Shader
    try
    {
        shader_buf = LoadShaderFile(cs_path);
        const char *computeShaderSource = shader_buf.c_str();
        CompileShader(computeShaderSource, GL_COMPUTE_SHADER, computeShader);
    }
    catch (const std::exception &e)
    {
        std::cerr << "Shader Load Error (Compute): " << e.what() << std::endl;
    }

    // Config Shader Program
    programID = glCreateProgram();
    glAttachShader(programID, computeShader);
    glLinkProgram(programID);

    int success;
    char infoLog[512];
    glGetProgramiv(programID, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(programID, 512, NULL, infoLog);
        std::cerr << "CS : " << std::string(cs_path) << std::endl;
        std::cerr << "ERROR::SHADER::PROGRAM::LINK_FAILED\n"
                  << infoLog << std::endl;
        throw std::runtime_error("Shader program link failed.");
    }
    glDeleteShader(computeShader);
}

ComputeShader::~ComputeShader()
{
    if (programID)
    {
        std::cout << std::format("Shader Program ID:{} Was Deleted~\n", programID);
        glDeleteProgram(programID);
    }
}

ComputeShader &ComputeShader::operator=(ComputeShader &&other) noexcept
{
    if (this != &other)
    {
        if (programID)
        {
            std::cout << std::format("Shader Program ID:{} Was Deleted\n", programID);
            glDeleteProgram(programID);
        }
        // Move the resources from the other object to this one
        this->cs_path = std::move(other.cs_path);
        this->programID = other.programID;
        this->used = other.used;
        this->uniformLocationMap = std::move(other.uniformLocationMap);
        this->warningMsgSet = std::move(other.warningMsgSet);
        this->texLocationID = other.texLocationID;

        // Invalidate the other object
        other.programID = 0;
        other.used = false;
        other.texLocationID = 0;
    }
    return *this;
}
GLint ComputeShader::getUniformLocationSafe(const std::string &name)
{
    if (!used)
    {
        throw std::runtime_error("Attempted to set uniform '" + name + "' while shader is not active (glUseProgram was not called).");
    }
    if (uniformLocationMap.find(name) != uniformLocationMap.end()) // Location缓存
    {
        return uniformLocationMap.at(name);
    }
    GLint location = glGetUniformLocation(programID, name.c_str());
    if (location == -1)
    {
        if (!warningMsgSet.contains(std::format("{}{}", name, programID)))
        {
            warningMsgSet.insert(std::format("{}{}", name, programID));
            std::cout << std::format("Warning: Uniform {} not found in shader program: {} ", name, programID);
            std::cout << std::format("   Compute Shader Path: {} ", cs_path);
        }
    }
    uniformLocationMap.insert({name, location});
    return location;
}