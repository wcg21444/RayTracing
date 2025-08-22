#include "Shader.hpp"

#include <format>
#include <iostream>

// ���캯��
Shader::Shader() : location_ID(0), progrm_ID(0) {}

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
        shader_buf = loadShaderFile(vs_path);
        const char *vertexShaderSource = shader_buf.c_str();
        compileShader(vertexShaderSource, GL_VERTEX_SHADER, vertexShader);
    }
    catch (const std::exception &e)
    {
        std::cerr << "Shader Load Error (Vertex): " << e.what() << std::endl;
    }

    // Config Fragment Shader
    try
    {
        shader_buf = loadShaderFile(fs_path);
        const char *fragmentShaderSource = shader_buf.c_str();
        compileShader(fragmentShaderSource, GL_FRAGMENT_SHADER, fragmentShader);
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
            shader_buf = loadShaderFile(gs_path);
            const char *geometryShadersource = shader_buf.c_str();
            compileShader(geometryShadersource, GL_GEOMETRY_SHADER, geometryShader);
        }
        catch (const std::exception &e)
        {
            std::cerr << "Shader Load Error (Geometry): " << e.what() << std::endl;
        }
    }

    // Cofig Shader Program
    progrm_ID = glCreateProgram();
    glAttachShader(progrm_ID, vertexShader);
    glAttachShader(progrm_ID, fragmentShader);
    if (hasGS)
    {
        glAttachShader(progrm_ID, geometryShader);
    }
    glLinkProgram(progrm_ID);

    int success;
    char infoLog[512];
    glGetProgramiv(progrm_ID, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(progrm_ID, 512, NULL, infoLog);
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

// ��������
Shader::~Shader()
{
    if (progrm_ID)
    {
        std::cout << std::format("Shader Program ID:{} Was Deleted~\n", progrm_ID);
        glDeleteProgram(progrm_ID);
    }
}

// �ƶ���ֵ�����
Shader &Shader::operator=(Shader &&other) noexcept
{
    if (this != &other)
    {
        if (progrm_ID)
        {
            std::cout << std::format("Shader Program ID:{} Was Deleted\n", progrm_ID);
            glDeleteProgram(progrm_ID);
        }
        this->progrm_ID = other.progrm_ID;
        this->used = other.used;
        this->vs_path = std::move(other.vs_path);
        this->fs_path = std::move(other.fs_path);
        this->gs_path = std::move(other.gs_path);
        this->textureLocationMap = std::move(other.textureLocationMap);
        this->location_ID = other.location_ID;
        other.progrm_ID = 0; // �ͷ�Դ������Դ
        other.used = false;
        other.location_ID = 0;
    }
    return *this;
}

// ˽�з���ʵ��
std::string Shader::loadShaderFile(const char *shader_path)
{
    std::fstream shader_file(shader_path, std::ios::in);
    if (!shader_file.is_open())
    {
        std::cerr << "(errno " << errno << "): " << strerror(errno) << std::endl;
        throw std::runtime_error("Failed to open shader file: " + std::string(shader_path));
    }
    std::stringstream shader_buffer;
    shader_buffer << shader_file.rdbuf();
    if (shader_file.fail() && !shader_file.eof())
    {
        std::cerr << "(errno " << errno << "): " << strerror(errno) << std::endl;
        throw std::runtime_error("Failed to read shader file: " + std::string(shader_path));
    }
    return shader_buffer.str();
}

GLint Shader::getUniformLocationSafe(const std::string &name)
{
    if (!used)
    {
        throw std::runtime_error("Attempted to set uniform '" + name + "' while shader is not active (glUseProgram was not called).");
    }
    if (uniformLocationMap.find(name) != uniformLocationMap.end()) // Location����
    {
        return uniformLocationMap.at(name);
    }
    GLint location = glGetUniformLocation(progrm_ID, name.c_str());
    if (location == -1)
    {
        if (!warningMsgSet.contains(std::format("{}{}", name, progrm_ID)))
        {
            warningMsgSet.insert(std::format("{}{}", name, progrm_ID));
            std::cout << std::format("Warning: Uniform {} not found in shader program: {} ", name, progrm_ID);
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

void Shader::compileShader(const char *shader_source, GLenum shader_type, unsigned int &shader_id)
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

// Uniform ���÷���ʵ��
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

// Ϊ vec3 ��д������
void Shader::setUniform(const std::string &name, const glm::vec3 &vec3)
{
    GLint location = getUniformLocationSafe(name);
    if (location != -1)
    {
        glUniform3fv(location, 1, glm::value_ptr(vec3));
    }
}

// Ϊ vec2 ��д������ (GLM vec2)
void Shader::setUniform(const std::string &name, const glm::vec2 &vec2)
{
    GLint location = getUniformLocationSafe(name);
    if (location != -1)
    {
        glUniform2fv(location, 1, glm::value_ptr(vec2));
    }
}

// Ϊ mat4 ��д������
void Shader::setUniform(const std::string &name, const glm::mat4 &mat)
{
    GLint location = getUniformLocationSafe(name);
    if (location != -1)
    {
        glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(mat));
    }
}

// Ϊ float ��д������
void Shader::setUniform(const std::string &name, float f)
{
    GLint location = getUniformLocationSafe(name);
    if (location != -1)
    {
        glUniform1f(location, f);
    }
}

// Ϊ int ��д������
void Shader::setUniform(const std::string &name, int i)
{
    GLint location = getUniformLocationSafe(name);
    if (location != -1)
    {
        glUniform1i(location, i);
    }
}

/**
 * @brief �Զ�������������ɫ����
 * * �˺���ּ�ڼ�����󶨹��̡����ڲ�ά��һ��ӳ��� (textureLocationMap)��
 * ����ɫ���еĲ����� (sampler) ������һ��Ψһ������Ԫ ID (location_ID) ����������
 * ���һ���������ǵ�һ�ΰ󶨣����ᱻ����һ���µ�����Ԫ��
 * ��󣬺����ἤ���Ӧ������Ԫ�����������󶨵��õ�Ԫ��
 * ��ͨ�� glUniform1i ��������Ԫ ID ���ݸ���ɫ���е� uniform ������
 * * @param textureID Ҫ�󶨵��������� OpenGL ID��
 * @param textureTarget ����Ŀ�����ͣ����� GL_TEXTURE_2D��GL_TEXTURE_CUBE_MAP �ȡ�
 * @param shaderTextureLocation ռλ��
 * @param samplerUniformName ��ɫ���� sampler uniform ���������ƣ����� "u_AlbedoMap"��
 */
void Shader::setTextureAuto(GLuint textureID, GLenum textureTarget, int shaderTextureLocation, const std::string &samplerUniformName)
{
    // ��һ��Ϊ�ò����� uniform ������
    if (textureLocationMap.find(samplerUniformName) == textureLocationMap.end())
    {
        // ����������������Ψһ������Ԫ ID ��������
        textureLocationMap.insert({samplerUniformName, location_ID});
        location_ID++;
    }

    // ��ӳ����л�ȡ�ò�������Ӧ������Ԫ ID
    int location = textureLocationMap.at(samplerUniformName);

    // ������Ԫ ID ת��Ϊ GL_TEXTURE0��GL_TEXTURE1 ��ö��ֵ
    GLenum activeTextureUnit = getTextureUnitEnum(location);

    glActiveTexture(activeTextureUnit);

    glBindTexture(textureTarget, textureID);

    GLint samplerLoc = getUniformLocationSafe(samplerUniformName);

    if (samplerLoc != -1)
    {
        // ������Ԫ ID (location) ���ݸ���ɫ���е� uniform ����
        // ������ɫ����֪��Ӧ�ô��ĸ�����Ԫ��ȡ����
        glUniform1i(samplerLoc, location);
    }
}

// ��̬���߷���ʵ��
inline GLenum Shader::getTextureUnitEnum(int textureLocation)
{
    if (getTextureUnitsLimits() == -1)
    {
        throw std::runtime_error("OpenGL texture limits not initialized.");
    }
    if (textureLocation < 0 || textureLocation >= getTextureUnitsLimits())
    {
        std::string errorMsg = "Texture location " + std::to_string(textureLocation) +
                               " out of bounds. Max texture units: " + std::to_string(getTextureUnitsLimits()) + ".";
        throw std::out_of_range(errorMsg);
    }
    return GL_TEXTURE0 + textureLocation;
}

inline GLint Shader::getTextureUnitsLimits()
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