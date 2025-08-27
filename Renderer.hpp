#pragma once

#include "GLShader/Shader.hpp"
#include "GLShader/Pass.hpp"
#include "Utils.hpp"
#include "Trace.hpp"
#include "Camera.hpp"
#include "Random.hpp"

#include <thread>
#include <future>

class Image
{
public:
    int width;
    int height;

private:
    Texture imageTexture;
    std::vector<vec4> imageData;
    std::vector<std::future<void>> shadingFutures; // 存储多个future对象

    Camera camera = Camera(1.0f, point3(0.0f, 0.0f, 1.0f), 2.0f, float(16) / float(9));

    int sampleCount = 1;
    float perturbStrength = 1e-3f;

private:
    void setPixel(int x, int y, vec4 &value)
    {
        this->imageData[y * width + x] = value;
    }
    vec4 &pixelAt(int x, int y)
    {
        return imageData[y * width + x];
    }
    vec2 uvAt(int x, int y)
    {
        return vec2(x / float(width), y / float(height));
    }
    void syncAndUploadShadingResult()
    {
        // 检查是否有任何有效的 future 对象
        if (!shadingFutures.empty() && shadingFutures[0].valid())
        {
            // 等待所有异步任务完成
            for (auto &future : shadingFutures)
            {
                future.get();
            }

            this->sampleCount++;
            this->imageTexture.SetData(imageData.data());
        }
    }

    void launchAsyncShadingTasks(int numThreads)
    {

        if (
            !shadingFutures.empty() &&
            shadingFutures[0].valid() &&
            shadingFutures[0].wait_for(std::chrono::seconds(0)) != std::future_status::ready)
        {
            return;
        }

        syncAndUploadShadingResult();

        this->shadingFutures.clear();

        int rowsPerThread = height / numThreads;

        // 启动异步任务
        for (int i = 0; i < numThreads; ++i)
        {
            int startY = i * rowsPerThread;
            int endY = (i == numThreads - 1) ? height : startY + rowsPerThread;

            // 启动一个异步任务，处理指定的行范围
            this->shadingFutures.push_back(std::async(std::launch::async, [this, startY, endY]()
                                                      {
                for (int y = startY; y < endY; ++y) 
                {

                    for (int x = 0; x < width; ++x) 
                    {
                        this->shade(x, y);
                    }
                } }));
        }
    }

public:
    Image(int _width,
          int _height) : width(_width), height(_height),
                         imageData(_width * _height)
    {
        this->imageTexture.Generate(_width, _height, GL_RGBA16F, GL_RGBA, GL_FLOAT, NULL);
        this->imageTexture.SetFilterMax(GL_NEAREST);
        this->imageTexture.SetFilterMin(GL_NEAREST);
    }
    unsigned int getGLTextureID()
    {
        return imageTexture.ID;
    }

    void resize(int newWidth, int newHeight)
    {
        syncResetSamples();

        this->width = newWidth;
        this->height = newHeight;

        this->camera.resize(newWidth, newHeight); // 适应image比例
        this->imageTexture.Resize(newWidth, newHeight);
        this->imageData.resize(newWidth * newHeight, color4(0.0f));
    }

    void syncResetSamples()
    {
        syncAndUploadShadingResult();
        this->sampleCount = 1;
        for (auto &pixel : imageData)
        {
            pixel = color4(0.0f);
        }
    }

    void draw()
    {
        launchAsyncShadingTasks(16);
        ImGui::Begin("RenderUI", 0);
        {
            if (
                ImGui::DragFloat3("CamPosition", glm::value_ptr(camera.position), 0.01f) ||
                ImGui::DragFloat3("LookAtCenter", glm::value_ptr(camera.lookAtCenter), 0.01f) ||
                ImGui::DragFloat("CamFocalLength", &camera.focalLength, 0.01f) ||
                ImGui::DragFloat("PerturbStrength", &perturbStrength, 1e-4f))
            {
                syncResetSamples();
            }
            ImGui::Text(std::format("HFov: {}", camera.getHorizontalFOV()).c_str());
            ImGui::Text(std::format("Count of Samples: {}", sampleCount).c_str());
            ImGui::End();
        }
    }
    // 采样积累 + gamma矫正 使得画面灰度不正确累积
    // static color4 GammaCorrection(const color4 &color, float gamma = 2.f)
    // {
    //     return color4(
    //         pow(color.r, 1 / (gamma)),
    //         pow(color.g, 1 / (gamma)),
    //         pow(color.b, 1 / (gamma)),
    //         color.a);
    // }

private:
    void shade(int x, int y)
    {
        vec2 uv = uvAt(x, y);
        vec4 &pixelColor = pixelAt(x, y);

        Ray ray(
            camera.position,
            camera.getRayDirction(uv) + Random::randomVector(perturbStrength)); // 每一个像素,打出一根光线进行追踪,然后着色
        auto newColor = castRay(ray);

        pixelColor = (pixelColor * static_cast<float>(sampleCount - 1.f) + newColor) / static_cast<float>(sampleCount); // 线性平均累积
    }
};

class PostProcessor : public Pass
{
private:
    Texture processedTex;

private:
    void initializeGLResources()
    {
        glGenFramebuffers(1, &FBO);
        processedTex.Generate(vp_width, vp_height, GL_RGBA16F, GL_RGBA, GL_FLOAT, NULL);
    }

public:
    PostProcessor() {}
    PostProcessor(int _vp_width, int _vp_height, std::string _vs_path,
                  std::string _fs_path) : Pass(_vp_width, _vp_height, _vs_path, _fs_path)
    {
        initializeGLResources();
        contextSetup();
    }

    void contextSetup()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, FBO);
        {
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, processedTex.ID, 0);
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void resize(int _width, int _height)
    {
        vp_width = _width;
        vp_height = _height;
        processedTex.Resize(_width, _height);
        contextSetup();
    }
    unsigned int getTextures()
    {
        return processedTex.ID;
    }
    void render(unsigned int screenTex)
    {
        static int toggleGammaCorrection = 1;
        static float gamma = 2.2f;

        ImGui::Begin("RenderUI");
        {
            ImGui::Checkbox("ToggleGamma", (bool *)&toggleGammaCorrection);
            ImGui::DragFloat("Gamma", &gamma, 1e-2f, 0.01f, 5.f);

            ImGui::End();
        }

        glViewport(0, 0, vp_width, vp_height);
        glBindFramebuffer(GL_FRAMEBUFFER, FBO);
        shaders.use();

        // 设置着色器参数
        shaders.setUniform("GammaCorrection", toggleGammaCorrection);
        shaders.setUniform("gamma", gamma);

        shaders.setTextureAuto(screenTex, GL_TEXTURE_2D, 0, "screenTex");
        DrawQuad();
    }
};

class Renderer
{
private:
    int width = 1600;
    int height = 900;

    Texture screenTexture;

private:
public:
    ScreenPass screenPass;
    PostProcessor postProcessor;
    Image image;
    // Camera camera;
    // Scene

    Renderer()
        : screenPass(ScreenPass(width, height, "GLSL/screenQuad.vs", "GLSL/texture.fs")),
          postProcessor(PostProcessor(width, height, "GLSL/screenQuad.vs", "GLSL/postProcess.fs")),
          image(Image(width, height))
    {
        screenPass.contextSetup();
        postProcessor.contextSetup();
    }
    void render()
    {
        image.draw();
        auto imageTextureID = image.getGLTextureID();

        postProcessor.render(imageTextureID);
        auto postProcessed = postProcessor.getTextures();

        screenPass.render(postProcessed);
    }
    void resize(int newWidth, int newHeight)
    {
        width = newWidth;
        height = newHeight;

        image.resize(width, height);
        screenPass.resize(width, height);
        postProcessor.resize(width, height);
    }
};