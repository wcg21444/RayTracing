#pragma once
#include "RenderInterfaces.hpp"
#include "RenderContexts.hpp"
#include "SimplifiedData.hpp"
#include "RenderTarget.hpp"
#include "Shader.hpp"
#include "Random.hpp"
#include "UI.hpp"
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
