#include "CPURayTracer.hpp"
#include "Trace.hpp"
#include <chrono>
#include "Random.hpp"
CPURayTracer::~CPURayTracer() {}

CPURayTracer::CPURayTracer(int _width, int _height)
    : width(_width), height(_height), imageData(_width * _height)
{
    this->imageTexture.generate(_width, _height, GL_RGBA16F, GL_RGBA, GL_FLOAT, NULL);
    this->imageTexture.setFilterMax(GL_NEAREST);
    this->imageTexture.setFilterMin(GL_NEAREST);
}

unsigned int CPURayTracer::getGLTextureID()
{
    return imageTexture.ID;
}

void CPURayTracer::resize(int newWidth, int newHeight)
{
    if (queryShadingTasksAllDone() == false)
    {
        discardShadingResults();
    }
    // 如果同步未完成,应该将下面操作加入到发起渲染任务之前
    this->width = newWidth;
    this->height = newHeight;
    this->imageTexture.resize(newWidth, newHeight);
    this->imageData.resize(newWidth * newHeight, color4(0.0f));
}

void CPURayTracer::resetSamples()
{
    if (queryShadingTasksAllDone() == false)//渲染任务未完成,则丢弃结果
    {
        discardShadingResults();//下一次阻塞同步将不上传图像
    }
    if (this->sampleCount > 1)
    {
        this->sampleCount = 1;
        for (auto &pixel : imageData)
        {
            pixel = color4(0.0f);
        }
    }
}

void CPURayTracer::draw(int numThreads, const Scene &sceneInput)
{
    shadeAsync(numThreads, sceneInput);

    ImGui::Begin("RenderUI", 0);
    {
        RenderState::Dirty |= ImGui::DragFloat3("CamPosition", glm::value_ptr(Renderer::Cam.position), 0.01f);
        RenderState::Dirty |= ImGui::DragFloat3("LookAtCenter", glm::value_ptr(Renderer::Cam.lookAtCenter), 0.01f);
        RenderState::Dirty |= ImGui::DragFloat("CamFocalLength", &Renderer::Cam.focalLength, 0.01f);
        RenderState::Dirty |= ImGui::DragFloat("PerturbStrength", &perturbStrength, 1e-4f);

        ImGui::Text(std::format("HFov: {}", Renderer::Cam.getHorizontalFOV()).c_str());
        ImGui::Text(std::format("Count of Samples: {}", sampleCount).c_str());
        ImGui::Text(std::format("Seconds per Sample: {}", secPerSample).c_str());

        ImGui::End();
    }
    DebugObjectRenderer::SetCamera(&Renderer::Cam);
}

void CPURayTracer::setPixel(int x, int y, vec4 &value)
{
    this->imageData[y * width + x] = value;
}

vec4 &CPURayTracer::pixelAt(int x, int y)
{
    return imageData[y * width + x];
}

vec2 CPURayTracer::uvAt(int x, int y)
{
    return vec2(x / float(width), y / float(height));
}

//非阻塞查询任务是否全部完成
bool CPURayTracer::queryShadingTasksAllDone()
{
    if (shadingFutures.empty())
    {
        return true;
    }
    bool allDone = true;
    for (auto &future : shadingFutures)
    {
        if (future.valid() && future.wait_for(std::chrono::seconds(0)) != std::future_status::ready)
        {
            allDone = false;
            break;
        }
    }
    return allDone;
}

//下一次同步将不上传图像数据
void CPURayTracer::discardShadingResults()
{
    discardCurrentImage = true;
}

// 阻塞线程,同步渲染结果;上传渲染结果;清除当前渲染future
void CPURayTracer::syncAndUploadShadingResult()
{
    // 这个方法是非阻塞的
    //  将会同步渲染结果
    if (!shadingFutures.empty())
    {
        for (auto &future : shadingFutures)
        {
            future.get(); // 如果有未完成future,将阻塞线程
        }

        if (!discardCurrentImage)
        {
            this->sampleCount++;
            this->imageTexture.setData(imageData.data()); // 上传纹理
        }
        discardCurrentImage = false;

        static auto lastTime = std::chrono::high_resolution_clock::now();
        auto currentTime = std::chrono::high_resolution_clock::now();
        secPerSample = static_cast<float>(
            std::chrono::duration<double>(currentTime - lastTime).count());
        lastTime = currentTime;
        this->shadingFutures.clear();
    }
}

void CPURayTracer::shadeAsync(int numThreads, const Scene &scene)
{

    if (queryShadingTasksAllDone() == true)
    {
        syncAndUploadShadingResult();

        if (!renderScene || RenderState::SceneDirty) // 场景数据脏 则触发更新
        {
            renderScene = std::make_unique<Scene>(scene); // 拷贝一份Scene  更新渲染场景
            RenderState::SceneDirty &= false;
        }

        int rowsPerThread = height / numThreads;
        for (int i = 0; i < numThreads; ++i)
        {
            int startY = i * rowsPerThread;
            int endY = (i == numThreads - 1) ? height : startY + rowsPerThread;
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
}

void CPURayTracer::shade(int x, int y)
{
    vec2 uv = uvAt(x, y);
    vec4 &pixelColor = pixelAt(x, y);
    Ray ray(
        Renderer::Cam.position,
        Renderer::Cam.getRayDirction(uv) + Random::RandomVector(perturbStrength));
    if (!renderScene)
    {
        throw std::runtime_error("Scene is not loaded.");
    }
    auto newColor = Trace::CastRay(ray, 0, *renderScene);
    pixelColor = (pixelColor * static_cast<float>(sampleCount - 1.f) + newColor) / static_cast<float>(sampleCount);
}
