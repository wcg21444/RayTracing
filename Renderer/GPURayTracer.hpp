
#pragma once
#include "Pass.hpp"
#include "Camera.hpp"
#include "SkyTexPass.hpp"
#include "DebugObjectRenderer.hpp"
#include "Random.hpp"
#include <string>
#include <imgui.h>
#include <glm/gtc/type_ptr.hpp>
#include "Renderer.hpp"
#include "RenderState.hpp"

class GPURayTracer : public Pass
{
private:
    unsigned int FBO1;
    unsigned int FBO2;

    Texture2D raytraceTex1;
    Texture2D raytraceTex2;

    int samplesCount = 1;

    void initializeGLResources()
    {
        glGenFramebuffers(1, &FBO1);
        glGenFramebuffers(1, &FBO2);
        raytraceTex1.generate(vp_width, vp_height, GL_RGBA32F, GL_RGBA, GL_FLOAT, NULL);
        raytraceTex2.generate(vp_width, vp_height, GL_RGBA32F, GL_RGBA, GL_FLOAT, NULL);
    }

public:
    GPURayTracer() {}
    GPURayTracer(int _vp_width, int _vp_height, std::string _vs_path,
                 std::string _fs_path) : Pass(_vp_width, _vp_height, _vs_path, _fs_path)
    {
        initializeGLResources();
        contextSetup();
    }

    ~GPURayTracer()
    {
        glDeleteBuffers(1, &FBO1);
        glDeleteBuffers(1, &FBO2);
    }

    void reloadCurrentShaders() override
    {
        Pass::reloadCurrentShaders();
    }

    void contextSetup() override
    {
        glBindFramebuffer(GL_FRAMEBUFFER, FBO1);
        {
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, raytraceTex1.ID, 0);
        }
        glBindFramebuffer(GL_FRAMEBUFFER, FBO2);
        {
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, raytraceTex2.ID, 0);
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void resize(int _width, int _height)
    {
        vp_width = _width;
        vp_height = _height;
        raytraceTex1.resize(_width, _height);
        raytraceTex2.resize(_width, _height);
        contextSetup();
    }

    void resetSamples() // 改变场景的任何物体 参数都要调用这个方法
    {
        glBindFramebuffer(GL_FRAMEBUFFER, FBO1);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glBindFramebuffer(GL_FRAMEBUFFER, FBO2);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        samplesCount = 1;
    }

    unsigned int getTextures()
    {
        return raytraceTex1.ID;
    }
    void render(TextureID skyTexID)
    {
        static int toggleGammaCorrection = 1;
        static float gamma = 2.2f;
        const bool CHANGED = true;

        ImGui::Begin("RenderUI");
        {
            RenderState::Dirty |= ImGui::DragFloat3("CamPosition", glm::value_ptr(Renderer::Cam.position), 0.01f);
            RenderState::Dirty |= ImGui::DragFloat3("LookAtCenter", glm::value_ptr(Renderer::Cam.lookAtCenter), 0.01f);
            RenderState::Dirty |= ImGui::DragFloat("CamFocalLength", &Renderer::Cam.focalLength, 0.01f);

            ImGui::Text(std::format("HFov: {}", Renderer::Cam.getHorizontalFOV()).c_str());
            ImGui::Text(std::format("SamplesCount: {}", samplesCount).c_str());

            ImGui::End();
        }
        DebugObjectRenderer::SetCamera(&Renderer::Cam);
        DebugObjectRenderer::AddDrawCall([](Shader &_shaders) -> void
                                         { DebugObjectRenderer::DrawWireframeCube(_shaders, glm::scale(glm::identity<glm::mat4>(), glm::vec3(2.0f))); });

        glViewport(0, 0, vp_width, vp_height);
        glBindFramebuffer(GL_FRAMEBUFFER, FBO1); // Ping 着色
        shaders.use();
        shaders.setTextureAuto(raytraceTex2.ID, GL_TEXTURE_2D, 0, "lastSample");
        shade(skyTexID);
        glBindFramebuffer(GL_FRAMEBUFFER, FBO2); // Pong 着色
        shaders.setTextureAuto(raytraceTex1.ID, GL_TEXTURE_2D, 0, "lastSample");
        shade(skyTexID);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
    void shade(TextureID skyTexID)
    {

        // 设置着色器参数
        shaders.setUniform("width", vp_width);
        shaders.setUniform("height", vp_height);
        float rand = Random::randomFloats(Random::generator);
        shaders.setUniform("rand", rand);
        shaders.setUniform("samplesCount", samplesCount);

        Renderer::Cam.setToFragShader(shaders, "cam");
        /****************************************天空设置*****************************************************/
        SkySettings::SetShaderUniforms(shaders);
        shaders.setTextureAuto(skyTexID, GL_TEXTURE_CUBE_MAP, 0, "skybox");

        DrawQuad();
        samplesCount++;
    }

};
