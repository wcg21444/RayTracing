
#pragma once
#include "Pass.hpp"
#include <string>
#include <imgui.h>

class PostProcessor : public Pass
{
private:
    Texture2D processedTex;

    void initializeGLResources()
    {
        glGenFramebuffers(1, &FBO);
        processedTex.generate(vp_width, vp_height, GL_RGBA16F, GL_RGBA, GL_FLOAT, NULL);
    }

public:
    PostProcessor() {}
    PostProcessor(int _vp_width, int _vp_height, std::string _vs_path,
                  std::string _fs_path) : Pass(_vp_width, _vp_height, _vs_path, _fs_path)
    {
        initializeGLResources();
        contextSetup();
    }

    void contextSetup()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, FBO);
        {
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, processedTex.ID, 0);
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void resize(int _width, int _height)
    {
        vp_width = _width;
        vp_height = _height;
        processedTex.resize(_width, _height);
        contextSetup();
    }
    unsigned int getTextures()
    {
        return processedTex.ID;
    }
    void render(unsigned int screenTex)
    {
        static int toggleGammaCorrection = 1;
        static float gamma = 2.2f;

        ImGui::Begin("RenderUI");
        {
            ImGui::Checkbox("ToggleGamma", (bool *)&toggleGammaCorrection);
            ImGui::DragFloat("Gamma", &gamma, 1e-2f, 0.01f, 5.f);

            ImGui::End();
        }

        glViewport(0, 0, vp_width, vp_height);
        glBindFramebuffer(GL_FRAMEBUFFER, FBO);
        shaders.use();

        // 设置着色器参数
        shaders.setUniform("GammaCorrection", toggleGammaCorrection);
        shaders.setUniform("gamma", gamma);

        shaders.setTextureAuto(screenTex, GL_TEXTURE_2D, 0, "screenTex");
        DrawQuad();
    }
};
