#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#include "TestRenderer.hpp"
#include "TestRenderState.hpp"
#include "TracerImpl.hpp"
#include "LoaderImpl.hpp"
#include "PostProcessor.hpp"
#include "SkyTexPass.hpp"
#include "Storage.hpp"
#include "RenderContexts.hpp"
#include "LoadMethods.hpp"
#include "TraceMethods.hpp"
#include "UIMethods.hpp"


#include "TestProfiler.hpp"

bool SaveFloatRGBAtoPNG(const float *floatRGBA, int width, int height, const char *filename, bool flipY = true)
{
    std::vector<unsigned char> out;
    out.resize(width * height * 4);
    auto clamp01 = [](float v)
    { return std::max(0.0f, std::min(1.0f, v)); };

    // 简单 gamma 校正 (sRGB)，并把值压缩到 [0,1]
    const float invGamma = 1.0f / 2.2f;

    for (int y = 0; y < height; ++y)
    {
        int srcY = flipY ? (height - 1 - y) : y;
        const float *row = floatRGBA + (size_t)srcY * width * 4;
        unsigned char *dst = out.data() + (size_t)y * width * 4;
        for (int x = 0; x < width; ++x)
        {
            float r = row[0], g = row[1], b = row[2], a = row[3];
            // 简单 tone-map: 使用 clamp( x / (1 + x) ) 或其它高级方法
            auto tone = [](float v)
            { return v / (1.0f + v); };
            r = tone(r);
            g = tone(g);
            b = tone(b);

            // gamma -> 8-bit
            dst[0] = (unsigned char)(std::round(std::pow(clamp01(r), invGamma) * 255.0f));
            dst[1] = (unsigned char)(std::round(std::pow(clamp01(g), invGamma) * 255.0f));
            dst[2] = (unsigned char)(std::round(std::pow(clamp01(b), invGamma) * 255.0f));
            dst[3] = (unsigned char)(std::round(clamp01(a) * 255.0f)); // alpha 线性
            row += 4;
            dst += 4;
        }
    }

    // 写 PNG，行对齐默认无问题；第四参数 stride = width*4
    int success = stbi_write_png(filename, width, height, 4, out.data(), width * 4);
    return success != 0;
}

TestRenderer::TestRenderer()
    : imagePass(std::make_unique<ImagePass>(width, height, "GLSL/screenQuad.vs", "GLSL/screenOutput.fs")),
      postProcessor(std::make_unique<PostProcessor>(width, height, "GLSL/screenQuad.vs", "GLSL/postProcess.fs")),
      skyTexPass(std::make_unique<SkyTexPass>("GLSL/cubemapSphere.vs", "GLSL/skyTex.fs", 256)),
      tracer(std::make_unique<TracerAsync>(width, height)),
      uploader(std::make_unique<SceneUploader>())

{
    changeMode(RenderMode::CPU_SdScene);
}

TestRenderer::~TestRenderer()
{
}

void TestRenderer::changeMode(RenderMode newMode)
{
    renderMode = newMode;

    tracer->waitForCompletion();
    uploader->waitForCompletion();
    tracer->resetSamples();
    switch (renderMode)
    {
    case RenderMode::GPU_SdScene:
        currentPipeline = std::make_unique<RenderPipeline<SdSceneGPUContext, LoadSdSceneGPU, TraceSdSceneGPU, SdSceneGPUUI>>(
            SdSceneGPUContext{
                skyTexPass->getCubemapRef(),
                RenderState::CameraInstance,
                Storage::SdSceneInstance,
                Storage::SdSceneInstanceMutex});
        break;
    case RenderMode::CPU_SdScene:
        currentPipeline = std::make_unique<RenderPipeline<SdSceneCPUContext, LoadSdSceneCPU, TraceSdSceneCPU, SdSceneCPUUI>>(
            SdSceneCPUContext{
                RenderState::CameraInstance,
                Storage::SdSceneInstance,
                Storage::SdSceneInstanceMutex});
        break;
        // case RenderMode::CPU_Scene:
        //     currentPipeline = std::make_unique<RenderPipeline<ImSceneCPUContext, LoadImSceneCPU, TraceImSceneCPU, ImSceneCPUUI>>(
        //         ImSceneCPUContext{
        //             RenderState::CameraInstance,
        //             Storage::ImplicitSceneInstance,
        //             Storage::ImplicitSceneInstanceMutex});
        //     break;

    default:
        assert(false && "Unknown RenderMode");
    }
    RenderState::Dirty |= true;
}

void TestRenderer::render()
{
    assert(currentPipeline && "RenderPipeline not set in TestRenderer::render");
    // Preprocessing
    skyTexPass->render(RenderState::CameraInstance.position);

    auto loadMethod = currentPipeline->getLoadMethod();
    auto traceMethod = currentPipeline->getTraceMethod();
    assert(loadMethod && "ILoadMethod not set in Renderer::render");
    assert(traceMethod && "ITraceMethod not set in Renderer::render");

    if (RenderState::SceneDirty)
    {
        tracer->resetSamples();
        uploader->upload(*loadMethod);
        RenderState::SceneDirty = false;
    }
    if (RenderState::Dirty)
    {
        tracer->waitForCompletion();
        uploader->waitForCompletion();
        // 拍摄context快照
        currentPipeline->snapshotContext();
        tracer->resetSamples();
        RenderState::Dirty = false;
    }
    int sample = 0;
    {
        Profiler::Test::ScopedTimeBlock tb("Total Raytracing Time");
        while (sample++ < RenderState::Test::SampleTimes)
        {
            tracer->render(*traceMethod);
        }
        glFinish();
    }
    // postprocessing

    auto raytraceResultID = tracer->getTraceOutputTextureID();
    // postProcessor->render(raytraceResultID);
    // auto postProcessed = postProcessor->getTextures();

    imagePass->render(raytraceResultID);

    auto outputData = imagePass->readOutputToCPU();

    SaveFloatRGBAtoPNG(outputData.data(), width, height, "./Test/TestOutput.png", true);
}

void TestRenderer::shutdown()
{
    // 同步等待所有任务完成
    tracer->waitForCompletion();
    uploader->waitForCompletion();
}

void TestRenderer::resize(int newWidth, int newHeight)
{
    width = newWidth;
    height = newHeight;

    imagePass->resize(width, height);
    postProcessor->resize(width, height);
    if (tracer)
    {
        tracer->resize(width, height);
    }

    RenderState::Dirty |= true;
}
