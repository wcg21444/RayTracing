#include "Renderer.hpp"
#include "GPURayTracer.hpp"
#include "PostProcessor.hpp"
#include "CPURayTracer.hpp"
#include "SkyTexPass.hpp"
#include "UICommon.hpp"

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
    if (RenderState::Dirty)
    {
        resetSamples();
    }

    ImGui::Begin("RenderUI");
    {
        RenderState::Dirty |= ImGui::Checkbox("UseGPU", &useGPU);
        if ((ImGui::Button("Reload")))
        {
            gpuRayTracer->reloadCurrentShaders();
            postProcessor->reloadCurrentShaders();
            skyTexPass->reloadCurrentShaders();
            DebugObjectRenderer::ReloadCurrentShaders();
            RenderState::Dirty |= true;
        }
        ImGui::End();
    }

    TextureID raytraceResult;
    if (!useGPU)
    {
        cpuRayTracer->draw(CPUThreads);
        raytraceResult = cpuRayTracer->getGLTextureID();
    }
    else
    {
        skyTexPass->render(Renderer::Cam.position);
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

    RenderState::Dirty |= true;
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
    RenderState::Dirty &= false;
}

void Renderer::sync()
{
    if (!useGPU)
    {
        cpuRayTracer->sync();
    }
}
