#include "SkyTexPass.hpp"
#include "Cubemap.hpp"
#include "UI.hpp"
#include "Camera.hpp"
SkyTexPass::SkyTexPass(std::string _vs_path, std::string _fs_path, int _cubemapSize)
    : Pass(0, 0, _vs_path, _fs_path),
      cubemapSize(_cubemapSize)
{
    cubemapParam = std::make_unique<CubemapParameters>(0.1f, 1e4f, glm::vec3(0.f));
    initializeGLResources();
    contextSetup();
}

SkyTexPass::~SkyTexPass()
{
    cleanUpGLResources();
}

inline void SkyTexPass::initializeGLResources()
{
    glGenFramebuffers(1, &FBO);
    skyCubemapTex.setWrapMode(GL_CLAMP_TO_EDGE);
    skyCubemapTex.generate(cubemapSize, cubemapSize, GL_RGBA32F, GL_RGBA, GL_FLOAT, GL_LINEAR, GL_LINEAR, false);
}

void SkyTexPass::cleanUpGLResources()
{
    glDeleteFramebuffers(1, &FBO);
}

inline void SkyTexPass::contextSetup()
{
}

inline void SkyTexPass::resize(int _width, int _height)
{
    contextSetup();
}

// 输入光源的Tex对象,绑定Tex对象到FBO,结果输出到Tex.
void SkyTexPass::render(const glm::vec3 & camPos)
{
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);
    glViewport(0, 0, cubemapSize, cubemapSize);

    shaders.use();
    if (!shaders.used)
        throw(std::exception("Shader failed to setup."));

    /****************************************视口设置***************************************************/
    shaders.setUniform("width", cubemapSize);
    shaders.setUniform("height", cubemapSize);
    /****************************************天空设置*****************************************************/
    SkySettings::SetShaderUniforms(shaders);
    /****************************************方向光源输入**************************************************/

    shaders.setUniform3fv("dirLightPos", SkySettings::sunlightDir);
    shaders.setUniform3fv("dirLightIntensity", SkySettings::sunlightIntensity);

    cubemapParam->update(camPos);
    for (unsigned int i = 0; i < 6; ++i)
    {
        shaders.setMat4("view", cubemapParam->viewMatrices[i]);
        shaders.setUniform3fv("eyePos", cubemapParam->viewPosition);
        shaders.setMat4("projection", cubemapParam->projectionMartix);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                               TextureCube::FaceTargets[i], skyCubemapTex.ID, 0);
        DrawSphere();
    }
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

unsigned int SkyTexPass::getCubemap()
{
    return skyCubemapTex.ID;
}
