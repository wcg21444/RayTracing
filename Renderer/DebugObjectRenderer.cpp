#include "DebugObjectRenderer.hpp"
#include "Camera.hpp"
#include "DebugObject.hpp"
#include "RenderTarget.hpp"
#include "Shader.hpp"
#include "Texture.hpp"
#include <glad/glad.h>

#define STATICIMPL

void DebugObjectRenderer::Initialize()
{
    if (debugObjectPass)
        throw(std::runtime_error("DebugObjectRenderer already initialized."));
    debugObjectPass = std::make_unique<DebugObjectPass>(width, height, "GLSL/debugRenderer.vs", "GLSL/debugRenderer.fs");
}

void DebugObjectRenderer::Resize(int _width, int _height)
{
    width = _width;
    height = _height;
    debugObjectPass->resize(width, height);
}

void DebugObjectRenderer::ReloadCurrentShaders()
{
    debugObjectPass->reloadCurrentShaders();
}

void DebugObjectRenderer::AddDrawCall(const DebugObjectDrawCall& drawCall)
{
    drawQueue.push(drawCall);
}

// Idea : 顺序上色
void DebugObjectRenderer::Render()
{
    debugObjectPass->render(drawQueue, *camera);
}

unsigned int DebugObjectRenderer::GetRenderOutput()
{
    return debugObjectPass->getTexture();
}

void DebugObjectRenderer::CheckInitialized()
{
    if (!debugObjectPass) {
        throw std::runtime_error("DebugObjectRenderer not initialized. Call Initialize() first.");
    }
}

void DebugObjectRenderer::DrawCube(Shader& shaders, glm::mat4 modelMatrix, glm::vec4 color)
{
    static Cube cube(glm::vec3(1.0f), "CubeTmp");

    shaders.setUniform("color", color);
    cube.draw(modelMatrix, shaders);
}

void DebugObjectRenderer::DrawWireframeCube(Shader& shaders, glm::mat4 modelMatrix, glm::vec4 color)
{
    static WireframeCube wireframeCube(glm::vec3(1.0f), "WireframeCubeTmp");

    shaders.setUniform("color", color);
    wireframeCube.draw(modelMatrix, shaders);
}
void DebugObjectRenderer::DrawWireframeCube(Shader& shaders, glm::vec3 pMin, glm::vec3 pMax, glm::vec4 color)
{
    static WireframeCube wireframeCube(glm::vec3(1.0f), "WireframeCubeTmp");
    glm::vec3 size = pMax - pMin;
    glm::vec3 center = (pMin + pMax) * 0.5f;
    auto modelMatrix = glm::translate(glm::mat4(1.0f), center) * glm::scale(glm::mat4(1.0f), size);
    shaders.setUniform("color", color);
    wireframeCube.draw(modelMatrix, shaders);
}

DebugObjectPass::DebugObjectPass(int _vp_width, int _vp_height, std::string _vs_path, std::string _fs_path)
    : Pass(_vp_width, _vp_height, _vs_path, _fs_path)
{
    renderTarget = std::make_unique<RenderTarget>(_vp_width, _vp_height);
    debugObjectPassTex = std::make_unique<Texture2D>();
    initializeGLResources();
    contextSetup();
}
DebugObjectPass::~DebugObjectPass()
{
    cleanUpGLResources();
}
void DebugObjectPass::initializeGLResources()
{
    debugObjectPassTex->setFilterMin(GL_NEAREST);
    debugObjectPassTex->setFilterMax(GL_NEAREST);
    debugObjectPassTex->generate(vp_width, vp_height, GL_RGBA16F, GL_RGBA, GL_FLOAT, NULL, false);
}

void DebugObjectPass::cleanUpGLResources()
{
}

void DebugObjectPass::contextSetup()
{
    renderTarget->bind();
    renderTarget->attachColorTexture2D(debugObjectPassTex->ID, GL_COLOR_ATTACHMENT0);
    renderTarget->enableColorAttachments();
    renderTarget->unbind();
}
void DebugObjectPass::resize(int _width, int _height)
{
    vp_width = _width;
    vp_height = _height;

    renderTarget->resize(vp_width, vp_height);

    debugObjectPassTex->resize(vp_width, vp_height);

    contextSetup();
}
void DebugObjectPass::render(std::queue<DebugObjectDrawCall>& drawQueue, Camera& cam)
{
    renderTarget->bind();
    renderTarget->setViewport();
    renderTarget->clearBuffer(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT, glm::vec4(0.0f));
    shaders.use();


    shaders.setUniform("view", cam.getViewMatrix());
    shaders.setUniform("projection", cam.getProjectionMatrix());
    while (!drawQueue.empty()) {
        auto& drawCall = drawQueue.front();
        drawCall(shaders);
        drawQueue.pop();
    }
    renderTarget->unbind();
}

unsigned int DebugObjectPass::getTexture()
{
    return debugObjectPassTex->ID;
}