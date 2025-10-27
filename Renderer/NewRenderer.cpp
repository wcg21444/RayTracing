#include "NewRenderer.hpp"
#include "TracerImpl.hpp"
#include "LoaderImpl.hpp"

NewRenderer::NewRenderer()
    : screenPass(std::make_unique<ScreenPass>(width, height, "GLSL/screenQuad.vs", "GLSL/screenOutput.fs")),
      postProcessor(std::make_unique<PostProcessor>(width, height, "GLSL/screenQuad.vs", "GLSL/postProcess.fs")),
      skyTexPass(std::make_unique<SkyTexPass>("GLSL/cubemapSphere.vs", "GLSL/skyTex.fs", 256)),
      tracer(std::make_unique<TracerAsync>()),
      uploader(std::make_unique<SceneUpLoader>())
{
}

NewRenderer::~NewRenderer()
{
}

void NewRenderer::render()
{
}

void NewRenderer::resize(int newWidth, int newHeight)
{
}

void NewRenderer::shutdown()
{
}

void NewRenderer::renderUI()
{
}
