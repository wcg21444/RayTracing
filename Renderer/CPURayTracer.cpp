#include "CPURayTracer.hpp"
#include "Trace.hpp"
#include <chrono>
#include "Random.hpp"
#include "SimplifiedData.hpp"
CPURayTracer::~CPURayTracer()
{
}

CPURayTracer::CPURayTracer(int _width, int _height)
    : width(_width), height(_height), tracer(_width, _height)
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
    syncCallbackScheduler.addCallback(
        GENERATE_CALLBACK_UNIQUE_ID(),
        [this, newWidth, newHeight]()
        {
            this->width = newWidth;
            this->height = newHeight;
            this->imageTexture.resize(newWidth, newHeight);
            this->tracer.resize(newWidth, newHeight);
            // this->imageData.resize(newWidth * newHeight, color4(0.0f));
        });
}

void CPURayTracer::resetSamples()
{
    syncCallbackScheduler.addCallback(
        GENERATE_CALLBACK_UNIQUE_ID(),
        [this]()
        {
            tracer.resetSamples();
        });
}

void CPURayTracer::draw(int numThreads)
{
    shadeAsync(numThreads);

    interact();
}

void CPURayTracer::shutdown()
{
    syncBlocking();
}

void CPURayTracer::setScene(const Scene &sceneInput)
{
    tracer.setScene(sceneInput);
}

void CPURayTracer::setSdScene(const sd::Scene &sceneInput)
{
    tracer.setSdScene(sceneInput);
}

// 非阻塞查询任务是否全部完成
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

// 下一次同步将不上传图像数据
void CPURayTracer::discardShadingResults()
{
    discardCurrentImage = true;
}

// 阻塞线程,同步渲染结果;上传渲染结果;清除当前渲染future
void CPURayTracer::syncBlocking()
{
    //  将会同步渲染结果
    if (!shadingFutures.empty())
    {
        for (auto &future : shadingFutures)
        {
            future.get(); // 如果有未完成future,将阻塞线程
        }
        this->shadingFutures.clear();

        // 上传图像
        tracer.sampleCounts++;
        this->imageTexture.setData(tracer.getImageData().data());

        // 同步操作
        syncCallbackScheduler.executeAll();

        // 计算帧周期
        static auto lastTime = std::chrono::high_resolution_clock::now();
        auto currentTime = std::chrono::high_resolution_clock::now();
        secPerSample = static_cast<float>(
            std::chrono::duration<double>(currentTime - lastTime).count());
        lastTime = currentTime;
    }
}

void CPURayTracer::shadeAsync(int numThreads)
{

    if (queryShadingTasksAllDone() == true)
    {
        syncBlocking();

        if (RenderState::SceneDirty || !tracer.isSceneShadingLoaded()) // 场景数据脏 则触发更新
        {
            tracer.uploadScene();
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
                    tracer.shade(x, y);
                }
            } }));
        }
    }
}
void CPURayTracer::sdShadeAsync(int numThreads)
{

    if (queryShadingTasksAllDone() == true)
    {
        syncBlocking();

        if (RenderState::SceneDirty || !tracer.isSdSceneShadingLoaded()) // 场景数据脏 则触发更新
        {
            tracer.uploadSdScene();
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
                    tracer.sdShade(x, y);
                }
            } }));
        }
    }
}

void CPURayTracer::interact()
{
    ImGui::Begin("RenderUI", 0);
    {
        RenderState::Dirty |= ImGui::DragFloat3("CamPosition", glm::value_ptr(Renderer::Cam.position), 0.01f);
        RenderState::Dirty |= ImGui::DragFloat3("LookAtCenter", glm::value_ptr(Renderer::Cam.lookAtCenter), 0.01f);
        RenderState::Dirty |= ImGui::DragFloat("CamFocalLength", &Renderer::Cam.focalLength, 0.01f);
        RenderState::Dirty |= ImGui::DragFloat("PerturbStrength", &tracer.perturbStrength, 1e-4f);

        ImGui::Text(std::format("HFov: {}", Renderer::Cam.getHorizontalFOV()).c_str());
        ImGui::Text(std::format("Count of Samples: {}", tracer.sampleCounts).c_str());
        ImGui::Text(std::format("Seconds per Sample: {}", secPerSample).c_str());

        ImGui::End();
    }
    DebugObjectRenderer::SetCamera(&Renderer::Cam);
}
