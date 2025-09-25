#include "Renderer.hpp"
#include "GPURayTracer.hpp"
#include "PostProcessor.hpp"
#include "CPURayTracer.hpp"
#include "SkyTexPass.hpp"

Renderer::Renderer()
    : screenPass(std::make_unique<ScreenPass>(width, height, "GLSL/screenQuad.vs", "GLSL/screenOutput.fs")),
      postProcessor(std::make_unique<PostProcessor>(width, height, "GLSL/screenQuad.vs", "GLSL/postProcess.fs")),
      gpuRayTracer(std::make_unique<GPURayTracer>(width, height, "GLSL/screenQuad.vs", "GLSL/simpleRayTrace.fs")),
      skyTexPass(std::make_unique<SkyTexPass>("GLSL/cubemapSphere.vs", "GLSL/skyTex.fs", 256)),
      cpuRayTracer(std::make_unique<CPURayTracer>(width, height))
{
    screenPass->contextSetup();
    gpuRayTracer->contextSetup();
    postProcessor->contextSetup();
    skyTexPass->contextSetup();
}

Renderer::~Renderer()
{
}

void Renderer::render()
{
    ImGui::Begin("RenderUI");
    {
        ImGui::Checkbox("UseGPU", &useGPU);
        ImGui::End();
    }

    TextureID raytraceResult;
    if (!useGPU)
    {
        cpuRayTracer->draw();
        raytraceResult = cpuRayTracer->getGLTextureID();
    }
    else
    {
        ImGui::Begin("RenderUI");
        {
            if ((ImGui::Button("Reload")))
            {
                gpuRayTracer->reloadCurrentShaders();
                postProcessor->reloadCurrentShaders();
                skyTexPass->reloadCurrentShaders();
                DebugObjectRenderer::ReloadCurrentShaders();
            }
            ImGui::End();
        }
        skyTexPass->render(gpuRayTracer->getCamera().position);
        auto skyTexID = skyTexPass->getCubemap();
        gpuRayTracer->render(skyTexID);
        raytraceResult = gpuRayTracer->getTextures();
    }
    postProcessor->render(raytraceResult);
    auto postProcessed = postProcessor->getTextures();

    auto debugRendererOutput = DebugObjectRenderer::GetRenderOutput();

    screenPass->render(postProcessed, debugRendererOutput);
}

void Renderer::resize(int newWidth, int newHeight)
{
    width = newWidth;
    height = newHeight;

    cpuRayTracer->resize(width, height);
    gpuRayTracer->resize(width, height);
    screenPass->resize(width, height);
    postProcessor->resize(width, height);

    resetSamples();
}

void Renderer::resetSamples()
{
    if (!useGPU)
    {
        cpuRayTracer->resetSamples();
    }
    else
    {
        gpuRayTracer->resetSamples();
    }
    Dirty = false;
}
