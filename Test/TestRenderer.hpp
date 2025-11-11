#pragma once

#include <thread>

#include "Pass.hpp"
#include "Utils.hpp"
#include "Camera.hpp"
#include "DebugObjectRenderer.hpp"
#include "RenderInterfaces.hpp"
#include "RenderTarget.hpp"

class ScreenPass;
class PostProcessor;
class GPURayTracer;
class SkyTexPass;
class CPURayTracer;
class ImplicitScene;

using ResizeCallback = std::function<void(int, int)>;

namespace Storage
{
    class SceneLoader;
}

namespace SimplifiedData
{
    class Scene;
}

enum class RenderMode
{
    GPU_SdScene,
    GPU_Scene,
    CPU_Scene,
    CPU_SdScene
};

// 将结果输出到Image数组
class ImagePass : public Pass
{
private:
    void initializeGLResources()
    {

        outputTexture.generate(vp_width, vp_height, GL_RGBA32F, GL_RGBA, GL_FLOAT, nullptr, true);
    }

    RenderTarget renderTarget;

    Texture2D outputTexture;

public:
    ImagePass(int _vp_width, int _vp_height, std::string _vs_path,
              std::string _fs_path)
        : Pass(_vp_width, _vp_height, _vs_path, _fs_path), renderTarget(_vp_width, _vp_height)
    {
        initializeGLResources();
        contextSetup();
    }

    void contextSetup()
    {
        renderTarget.bind();
        renderTarget.attachColorTexture2D(outputTexture.ID, GL_COLOR_ATTACHMENT0);
        renderTarget.enableColorAttachments();
        renderTarget.unbind();
    }

    void resize(int _width, int _height)
    {
        if (vp_width == _width &&
            vp_height == _height)
        {
            return;
        }
        vp_width = _width;
        vp_height = _height;

        renderTarget.resize(vp_width, vp_height);
        outputTexture.resize(vp_width, vp_height);

        contextSetup();
    }

    void render(unsigned int outputLayer0)
    {
        renderTarget.bind();
        renderTarget.setViewport();
        renderTarget.clearBuffer(GL_COLOR_BUFFER_BIT);
        // 设置着色器参数
        shaders.use();
        shaders.setTextureAuto(outputLayer0, GL_TEXTURE_2D, 0, "outputLayer0");
        DrawQuad();
        renderTarget.unbind();
    }

    std::vector<float> readOutputToCPU()
    {
        std::vector<float> buf(vp_width * vp_height * 4);
        glBindTexture(GL_TEXTURE_2D, outputTexture.ID);
        glPixelStorei(GL_PACK_ALIGNMENT, 1); // 行对齐，避免 padding
        glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, buf.data());
        glBindTexture(GL_TEXTURE_2D, 0);
        return buf;
    }
};

class TestRenderer
{
private:
    int width = 1600;
    int height = 900;

private:
    RenderMode renderMode = RenderMode::GPU_SdScene;

public:
    std::unique_ptr<ImagePass> imagePass = nullptr;
    std::unique_ptr<PostProcessor> postProcessor = nullptr;
    std::unique_ptr<SkyTexPass> skyTexPass = nullptr;

    std::unique_ptr<ITracer> tracer = nullptr;
    std::unique_ptr<IUpLoader> uploader = nullptr;
    std::unique_ptr<IRenderPipeline> currentPipeline = nullptr;

    TestRenderer();
    TestRenderer(const TestRenderer &) = delete;
    TestRenderer &operator=(const TestRenderer &) = delete;
    ~TestRenderer();

    void changeMode(RenderMode newMode);
    void render();
    void shutdown();
    void resize(int newWidth, int newHeight);

private:
};