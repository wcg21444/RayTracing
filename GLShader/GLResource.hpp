#pragma once
#include <glad/glad.h>
class GLResource
{
public:
    GLuint ID = 0;

public:
    GLResource() = default;
    GLResource(const GLResource &other) = delete;            // 禁用拷贝构造函数
    GLResource &operator=(const GLResource &other) = delete; // 禁用拷贝赋值运算符
    GLResource(GLResource &&other) noexcept                  // 启用移动构造函数
        : ID(other.ID)
    {
        other.ID = 0;
    }
    GLResource &operator=(GLResource &&other) noexcept // 启用移动赋值运算符
    {
        if (this != &other)
        {
            // 移动资源
            ID = other.ID;
            other.ID = 0; 
        }
        return *this;
    }
    virtual ~GLResource() {}
};
