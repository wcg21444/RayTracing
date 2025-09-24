#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>
#include <string>
#include <iostream>

#include "GLResource.hpp"
#include "Texture.hpp"

// FBO 封装
class RenderTarget : public GLResource
{
private:
    unsigned int ID;
    std::vector<GLenum> attachments;
    int width;
    int height;

public:
    RenderTarget(int _width, int _height)
        : width(_width), height(_height)
    {
        glGenFramebuffers(1, &ID);
    }
    ~RenderTarget()
    {
        if (ID)
        {
            glDeleteFramebuffers(1, &ID);
            ID = 0;
        }
    }
    void bind()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, ID);
    }
    void unbind()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void attachColorTexture2D(TextureID textureID, GLenum attachment)
    {
        bind();
        glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_2D, textureID, 0);
        if (std::find(attachments.begin(), attachments.end(), attachment) == attachments.end())
        {
            attachments.push_back(attachment);
        }
    }

    void attachDepthTexture2D(TextureID textureID)
    {
        bind();
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, textureID, 0);
    }

    void attachDepthRenderBuffer(unsigned int depthRenderBufferID, GLenum format = GL_DEPTH_COMPONENT)
    {
        bind();
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRenderBufferID);
    }

    void attachDepthTexture2DArray(TextureID textureID, int layer)
    {
        bind();
        glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, textureID, 0, layer);
    }

    void attachColorTexture2DArray(TextureID textureID, GLenum attachment, int layer)
    {
        bind();
        glFramebufferTextureLayer(GL_FRAMEBUFFER, attachment, textureID, 0, layer);

        if (std::find(attachments.begin(), attachments.end(), attachment) == attachments.end())
        {
            attachments.push_back(attachment);
        }
    }

    void disableDrawColor()
    {
        bind();
        glDrawBuffer(GL_NONE);
    }

    void disableReadColor()
    {
        bind();
        glReadBuffer(GL_NONE);
    }

    void enableColorAttachments()
    {
        bind();
        glDrawBuffers(static_cast<GLsizei>(attachments.size()), attachments.data());
    }
    void resize(int _width, int _height)
    {
        width = _width;
        height = _height;
    }

    /// @brief
    /// @param options GL_COLOR_BUFFER_BIT , GL_DEPTH_BUFFER_BIT
    void clearBuffer(GLenum options, glm::vec4 clearColor = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f))
    {
        bind();
        glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
        glClear(options);
    }
    void setViewport()
    {
        glViewport(0, 0, width, height);
    }

    void checkStatus()
    {
        bind();
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        {
            std::cerr << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
        }
        unbind();
    }
};