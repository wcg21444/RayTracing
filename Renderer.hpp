#pragma once

#include "GLShader/Shader.hpp"
#include "GLShader/Pass.hpp"
#include "Utils.hpp"
#include "Trace.hpp"
#include "Camera.hpp"
#include "Random.hpp"

#include <thread>
#include <future>

using namespace glm;

class Image
{
public:
    int width;
    int height;

private:
    Texture imageTexture;
    std::vector<vec4> imageData;
    std::vector<std::future<void>> shadingFutures; // 存储多个future对象
    float perturbStrength = 1e-3f;

    Camera camera = Camera(1.0f, point3(0.0f, 0.0f, 1.0f), 2.0f, float(16) / float(9));

    int sampleCount = 1;

private:
    void setPixel(int x, int y, vec4 &value)
    {
        imageData[y * width + x] = value;
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
            sampleCount++;
            imageTexture.SetData(imageData.data());
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

        shadingFutures.clear();

        int rowsPerThread = height / numThreads;

        // 启动异步任务
        for (int i = 0; i < numThreads; ++i)
        {
            int startY = i * rowsPerThread;
            int endY = (i == numThreads - 1) ? height : startY + rowsPerThread;

            // 启动一个异步任务，处理指定的行范围
            shadingFutures.push_back(std::async(std::launch::async, [this, startY, endY]()
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
        imageTexture.Generate(_width, _height, GL_RGBA16F, GL_RGBA, GL_FLOAT, NULL);
        imageTexture.SetFilterMax(GL_NEAREST);
        imageTexture.SetFilterMin(GL_NEAREST);
    }
    unsigned int getGLTextureID()
    {
        return imageTexture.ID;
    }

    void resize(int newWidth, int newHeight)
    {
        syncResetSamples();

        width = newWidth;
        height = newHeight;

        camera.resize(newWidth, newHeight); // 适应image比例
        imageTexture.Resize(newWidth, newHeight);
        imageData.resize(newWidth * newHeight, color4(0.0f));
    }

    void syncResetSamples()
    {
        syncAndUploadShadingResult();
        sampleCount = 1;
        for (auto &pixel : imageData)
        {
            pixel = color4(0.0f);
        }
    }

    void draw()
    {
        launchAsyncShadingTasks(12);

        ImGui::Begin("RenderUI", 0);
        {
            if (
                ImGui::DragFloat3("CamPosition", glm::value_ptr(camera.position), 0.01f) ||
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

class Renderer
{
private:
    int width = 1600;
    int height = 900;

    Texture screenTexture;

private:
public:
    ScreenPass screenPass;
    Image image;
    // Camera camera;
    // Scene

    Renderer()
        : screenPass(ScreenPass(width, height, "GLSL/screenQuad.vs", "GLSL/texture.fs")),
          image(Image(width, height))
    {
        screenPass.contextSetup();
    }
    void render()
    {
        image.draw();
        auto imageTextureID = image.getGLTextureID();
        screenPass.render(imageTextureID);
    }
    void resize(int newWidth, int newHeight)
    {
        width = newWidth;
        height = newHeight;

        image.resize(width, height);
        screenPass.resize(width, height);
    }
};