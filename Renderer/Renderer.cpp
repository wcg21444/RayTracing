#include "Renderer.hpp"
#include "GPURayTracer.hpp"
#include "PostProcessor.hpp"
#include "CPURayTracer.hpp"
#include "SkyTexPass.hpp"
#include "RenderState.hpp"
#include "Storage.hpp"

Renderer::Renderer()
    : screenPass(std::make_unique<ScreenPass>(width, height, "GLSL/screenQuad.vs", "GLSL/screenOutput.fs")),
      postProcessor(std::make_unique<PostProcessor>(width, height, "GLSL/screenQuad.vs", "GLSL/postProcess.fs")),
      gpuRayTracer(std::make_unique<GPURayTracer>(width, height, "GLSL/screenQuad.vs", "GLSL/simpleRayTrace.fs")),
      skyTexPass(std::make_unique<SkyTexPass>("GLSL/cubemapSphere.vs", "GLSL/skyTex.fs", 256)),
      cpuRayTracer(std::make_unique<CPURayTracer>(width, height))
{

    skyTexPass->contextSetup();
    gpuRayTracer->contextSetup();
    postProcessor->contextSetup();
}

Renderer::~Renderer()
{
}

void Renderer::render(const Scene &scene)
{
    renderUI();

    if (RenderState::Dirty)
    {
        resetSamples();
    }

    TextureID raytraceResult;
    if (!useGPU)
    {
        cpuRayTracer->setScene(scene);
        cpuRayTracer->draw(CPUThreads);
        raytraceResult = cpuRayTracer->getTraceResult();
    }
    else
    {
        skyTexPass->render(Renderer::Cam.position);
        auto skyTexID = skyTexPass->getCubemap();
        gpuRayTracer->render(skyTexID, 0);
        raytraceResult = gpuRayTracer->getTraceResult();
    }
    postProcessor->render(raytraceResult);
    auto postProcessed = postProcessor->getTextures();

    auto debugRendererOutput = DebugObjectRenderer::GetRenderOutput();

    screenPass->render(postProcessed, debugRendererOutput);
}
void Renderer::render(const sd::Scene &scene)
{

    renderUI();

    if (RenderState::Dirty)
    {
        resetSamples();
    }

    TextureID raytraceResult;

    // TODO 使用策略模式或桥接器模式重构
    if (!useGPU)
    {
        if (RenderState::SceneDirty)
        {
            cpuRayTracer->setSdScene(Storage::SdScene);
            RenderState::SceneDirty = false;
        }
        cpuRayTracer->draw(CPUThreads);
        raytraceResult = cpuRayTracer->getTraceResult();
    }
    else
    {

        skyTexPass->render(Renderer::Cam.position);
        auto skyTexID = skyTexPass->getCubemap();

        gpuRayTracer->renderUI();

        try
        {
            if (RenderState::SceneDirty)
            {
                Storage::SdSceneLoader.upload();

                RenderState::SceneDirty = false;
            }
        }
        catch (std::exception &e)
        {
            std::cerr << e.what() << std::endl;
        }
        gpuRayTracer->render(skyTexID, 0);

        raytraceResult = gpuRayTracer->getTraceResult();
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

void Renderer::shutdown()
{
    if (!useGPU)
    {
        cpuRayTracer->shutdown();
    }
}

void Renderer::renderUI()
{

    ImGui::Begin("RenderUI");
    {
        // Toggle GPU/CPU Raytracing
        if (ImGui::Checkbox("UseGPU", &useGPU))
        {
            RenderState::Dirty = true;
            RenderState::SceneDirty = true;
        }
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
}
