#include "TraceMethods.hpp"
#include "Pass.hpp"
#include <future>
#include <stdexcept>
#include <iostream>
#include <shared_mutex>
#include <algorithm>
#include <memory>

// TraceSdSceneGPU
TraceSdSceneGPU::TraceSdSceneGPU(SdSceneGPUContext &context)
    : DIContext(context),
      traceRenderTarget(1, 1),
      traceShader("GLSL/screenQuad.vs", "GLSL/simpleRayTrace.fs") {}

void TraceSdSceneGPU::trace(const Texture2D &traceInput, Texture2D &traceOutput, int sampleCount) {
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
        SkySettings::SetShaderUniforms(traceShader);
        traceShader.setTextureAuto(DIContext.skyboxTextureID, GL_TEXTURE_CUBE_MAP, 0, "skybox");
        DrawQuad();
    }
    traceRenderTarget.unbind();
}

// TraceSdSceneCPU
TraceSdSceneCPU::TraceSdSceneCPU(SdSceneCPUContext &context) : DIContext(context) {}
void TraceSdSceneCPU::trace(const Texture2D &traceInput, Texture2D &traceOutput, int sampleCount) {
    traceImageData.resize(traceInput.Width, traceInput.Height);
    auto shade = [this, sampleCount](CPUImageData &imageData, size_t x, size_t y) {
        const float perturbStrength = 0.001f;
        auto &pixelColor = imageData.pixelAt(x, y);
        auto uv = imageData.uvAt(x, y);
        Ray ray(
            DIContext.cam.position,
            DIContext.cam.getRayDirction(uv) + Random::RandomVector(perturbStrength));
        if (!DIContext.sceneRendering) {
            throw std::runtime_error("Scene is not loaded.");
        }
        std::shared_lock<std::shared_mutex> sceneLock(*DIContext.sceneRenderingMutex);
        auto newColor = Trace::CastRay(ray, 0, *DIContext.sceneRendering->pDataStorage);
        pixelColor = (pixelColor * static_cast<float>(sampleCount - 1.f) + newColor) / static_cast<float>(sampleCount);
    };
    size_t rowsPerThread = traceImageData.height / numThreads;
    for (int i = 0; i < numThreads; ++i) {
        size_t startY = i * rowsPerThread;
        size_t endY = (i == numThreads - 1) ? traceImageData.height : startY + rowsPerThread;
        this->shadingFutures.push_back(std::async(std::launch::async, [this, startY, endY, shade]() {
            for (size_t y = startY; y < endY; ++y) {
                for (size_t x = 0; x < traceImageData.width; ++x) {
                    shade(this->traceImageData, x, y);
                }
            }
        }));
    }
    if (!shadingFutures.empty()) {
        for (auto &future : shadingFutures) {
            future.get();
        }
        this->shadingFutures.clear();
    }
    traceOutput.setData(traceImageData.data());
}

// TraceSceneCPU
TraceSceneCPU::TraceSceneCPU(SceneCPUContext &context) : DIContext(context) {}
void TraceSceneCPU::trace(const Texture2D &traceInput, Texture2D &traceOutput, int sampleCount) {
    traceImageData.resize(traceInput.Width, traceInput.Height);
    auto shade = [this, sampleCount](CPUImageData &imageData, size_t x, size_t y) {
        const float perturbStrength = 0.001f;
        auto &pixelColor = imageData.pixelAt(x, y);
        auto uv = imageData.uvAt(x, y);
        Ray ray(
            DIContext.cam.position,
            DIContext.cam.getRayDirction(uv) + Random::RandomVector(perturbStrength));
        if (!DIContext.sceneRendering) {
            throw std::runtime_error("Scene is not loaded.");
        }
        std::shared_lock<std::shared_mutex> sceneLock(*DIContext.sceneRenderingMutex);
        auto newColor = Trace::CastRay(ray, 0, *DIContext.sceneRendering);
        pixelColor = (pixelColor * static_cast<float>(sampleCount - 1.f) + newColor) / static_cast<float>(sampleCount);
    };
    size_t rowsPerThread = traceImageData.height / numThreads;
    for (int i = 0; i < numThreads; ++i) {
        size_t startY = i * rowsPerThread;
        size_t endY = (i == numThreads - 1) ? traceImageData.height : startY + rowsPerThread;
        this->shadingFutures.push_back(std::async(std::launch::async, [this, startY, endY, shade]() {
            for (size_t y = startY; y < endY; ++y) {
                for (size_t x = 0; x < traceImageData.width; ++x) {
                    shade(this->traceImageData, x, y);
                }
            }
        }));
    }
    if (!shadingFutures.empty()) {
        for (auto &future : shadingFutures) {
            future.get();
        }
        this->shadingFutures.clear();
    }
    traceOutput.setData(traceImageData.data());
}
