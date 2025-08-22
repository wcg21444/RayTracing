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
#include <unordered_set>
#include <iostream>

class Shader
{
public:
    std::string vs_path;
    std::string fs_path;
    std::string gs_path;
    unsigned int progrm_ID;
    bool used = false;
    std::unordered_map<std::string, int> textureLocationMap;
    std::unordered_map<std::string, int> uniformLocationMap;
    std::unordered_set<std::string> warningMsgSet;
    int location_ID;

private:
    std::string loadShaderFile(const char *shader_path);
    GLint getUniformLocationSafe(const std::string &name);
    void compileShader(const char *shader_source, GLenum shader_type, unsigned int &shader_id);

public:
    // 构造函数
    Shader();
    Shader(const char *vs_path, const char *fs_path, const char *gs_path = nullptr);
    ~Shader();

    Shader &operator=(Shader &&other) noexcept;

    bool hasUniform(const std::string &name)
    {
        return glGetUniformLocation(progrm_ID, name.c_str()) != -1;
    }

    void use()
    {
        glUseProgram(progrm_ID);
        used = true;
    }

    // Uniform 设置方法
    void setUniform4fv(const std::string &name, GLsizei count, const float *value);
    void setUniform4fv(const std::string &name, const glm::vec4 &vec4);
    void setUniform3fv(const std::string &name, const glm::vec3 &vec3);
    void setMat4(const std::string &name, const glm::mat4 &mat);
    void setFloat(const std::string &name, float f);
    void setInt(const std::string &name, int i);

    /*********************统一重载方法*************************************/
    void setUniform(const std::string &name, const glm::vec4 &vec4);
    void setUniform(const std::string &name, const glm::vec3 &vec3);
    void setUniform(const std::string &name, const glm::vec2 &vec2);
    void setUniform(const std::string &name, const glm::mat4 &mat);
    void setUniform(const std::string &name, float f);
    void setUniform(const std::string &name, int i);

    void setTextureAuto(GLuint textureID, GLenum textureTarget, int shaderTextureLocation, const std::string &samplerUniformName);

    // 静态工具方法
    inline static GLenum getTextureUnitEnum(int textureLocation);
    inline static GLint getTextureUnitsLimits();
};