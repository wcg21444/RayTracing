#pragma once
#include "RenderInterfaces.hpp"
#include "RenderContexts.hpp"
#include "SimplifiedData.hpp"
#include "RenderTarget.hpp"
#include "Shader.hpp"
#include "Random.hpp"
#include "UI.hpp"
#include "Trace.hpp"

#include <future>
class TraceSdSceneGPU : public ITraceMethod // 产生对应Context的引用依赖
{
    SdSceneGPUContext &DIContext; // DI 必须

    RenderTarget traceRenderTarget;

    Shader traceShader;

public:
    TraceSdSceneGPU(SdSceneGPUContext &context)
        : DIContext(context),
          traceRenderTarget(1, 1),
          traceShader("GLSL/screenQuad.vs", "GLSL/simpleRayTrace.fs") // 直接使用simpleRayTrace 会导致nvogl卡死
    {
    }

    void trace(const Texture2D &traceInput, Texture2D &traceOutput, int sampleCount) override
    {
        traceRenderTarget.bind();
        traceRenderTarget.attachColorTexture2D(traceOutput.ID, GL_COLOR_ATTACHMENT0);
        traceRenderTarget.enableColorAttachments();

        traceRenderTarget.resize(traceInput.Width, traceInput.Height);
        traceRenderTarget.setViewport();

        traceShader.use();
        traceShader.setTextureAuto(traceInput.ID, GL_TEXTURE_2D, 0, "lastSample");

        traceShader.setUniform("width", traceInput.Width);
        traceShader.setUniform("height", traceInput.Height);
        float rand = Random::randomFloats(Random::generator);
        traceShader.setUniform("rand", rand);
        traceShader.setUniform("samplesCount", sampleCount);

        DIContext.cam.setToFragShader(traceShader, "cam");
        {
            std::shared_lock<std::shared_mutex> sceneLock(*DIContext.sceneBundleRenderingMutex);
            auto &[NodeStorageTexRendering, TriangleStorageTexRendering, SceneRootIndexRendering] = *DIContext.sceneBundleRendering;
            traceShader.setTextureAuto(NodeStorageTexRendering.ID, GL_TEXTURE_2D, 0, "nodeStorageTex");
            traceShader.setTextureAuto(TriangleStorageTexRendering.ID, GL_TEXTURE_2D, 0, "triangleStorageTex");
            traceShader.setUniform("sceneRootIndex", static_cast<unsigned int>(SceneRootIndexRendering));

            /****************************************天空设置**************************************************/
            SkySettings::SetShaderUniforms(traceShader);
            traceShader.setTextureAuto(DIContext.skyboxTextureID, GL_TEXTURE_CUBE_MAP, 0, "skybox");

            DrawQuad();
        }
        traceRenderTarget.unbind();
    }
};

class CPUImageData
{
    std::vector<glm::vec4> pixels; // RGBA format
public:
    size_t width;
    size_t height;
    inline void setPixel(size_t x, size_t y, glm::vec4 &value) { this->pixels[y * width + x] = value; }
    inline glm::vec4 &pixelAt(size_t x, size_t y) { return pixels[y * width + x]; }
    inline glm::vec2 uvAt(size_t x, size_t y) { return glm::vec2(x / float(width), y / float(height)); }
    inline void resize(size_t w, size_t h)
    {
        width = w;
        height = h;
        pixels.resize(w * h, glm::vec4(0.0f));
    }
    inline glm::vec4 *data() { return pixels.data(); }
};

class TraceSdSceneCPU : public ITraceMethod
{
    SdSceneCPUContext &DIContext;

    CPUImageData traceImageData;

    size_t numThreads = 16;
    std::vector<std::future<void>> shadingFutures;

public:
    TraceSdSceneCPU(SdSceneCPUContext &context)
        : DIContext(context)
    {
    }

    void trace(const Texture2D &traceInput, Texture2D &traceOutput, int sampleCount) override
    {
        traceImageData.resize(traceInput.Width, traceInput.Height);

        auto shade = [this, sampleCount](CPUImageData &imageData, size_t x, size_t y)
        {
            const float perturbStrength = 0.001f; // Placeholder
            auto &pixelColor = imageData.pixelAt(x, y);
            auto uv = imageData.uvAt(x, y);
            Ray ray(
                DIContext.cam.position,
                DIContext.cam.getRayDirction(uv) + Random::RandomVector(perturbStrength));
            if (!DIContext.sceneRendering)
            {
                throw std::runtime_error("Scene is not loaded.");
            }
            std::shared_lock<std::shared_mutex> sceneLock(*DIContext.sceneRenderingMutex);
            {
                auto newColor = Trace::CastRay(ray, 0, *DIContext.sceneRendering->pDataStorage);
                pixelColor = (pixelColor * static_cast<float>(sampleCount - 1.f) + newColor) / static_cast<float>(sampleCount);
            }
        };

        size_t rowsPerThread = traceImageData.height / numThreads;
        for (int i = 0; i < numThreads; ++i)
        {
            size_t startY = i * rowsPerThread;
            size_t endY = (i == numThreads - 1) ? traceImageData.height : startY + rowsPerThread;
            this->shadingFutures.push_back(std::async(std::launch::async, [this, startY, endY, shade]()
                                                      { 
                    for (size_t y = startY; y < endY; ++y)
                    {
                        for (size_t x = 0; x < traceImageData.width; ++x)
                        {
                            shade(this->traceImageData, x, y);
                        }
                    } }));
        }

        // 等待所有线程完成
        if (!shadingFutures.empty())
        {
            for (auto &future : shadingFutures)
            {
                future.get(); // 如果有未完成future,将阻塞线程
            }
            this->shadingFutures.clear();
        }
        traceOutput.setData(traceImageData.data());
    }
};