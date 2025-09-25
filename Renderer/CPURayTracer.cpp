#include "CPURayTracer.hpp"
#include <chrono>
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
    syncAndUploadShadingResult();
    this->width = newWidth;
    this->height = newHeight;
    this->camera.resize(newWidth, newHeight);
    this->imageTexture.resize(newWidth, newHeight);
    this->imageData.resize(newWidth * newHeight, color4(0.0f));
}

void CPURayTracer::resetSamples()
{
    syncAndUploadShadingResult();

    if (this->sampleCount > 1)
    {
        this->sampleCount = 1;
        for (auto &pixel : imageData)
        {
            pixel = color4(0.0f);
        }
    }
}

void CPURayTracer::draw(int numThreads)
{
    shadingAsync(numThreads);
    ImGui::Begin("RenderUI", 0);
    {
        RenderState::Dirty |= ImGui::DragFloat3("CamPosition", glm::value_ptr(camera.position), 0.01f);
        RenderState::Dirty |= ImGui::DragFloat3("LookAtCenter", glm::value_ptr(camera.lookAtCenter), 0.01f);
        RenderState::Dirty |= ImGui::DragFloat("CamFocalLength", &camera.focalLength, 0.01f);
        RenderState::Dirty |= ImGui::DragFloat("PerturbStrength", &perturbStrength, 1e-4f);

        ImGui::Text(std::format("HFov: {}", camera.getHorizontalFOV()).c_str());
        ImGui::Text(std::format("Count of Samples: {}", sampleCount).c_str());
        ImGui::Text(std::format("Seconds per Sample: {}", secPerSample ).c_str());

        ImGui::End();
    }
    DebugObjectRenderer::SetCamera(&camera);
}

void CPURayTracer::sync()
{
    syncAndUploadShadingResult();
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

void CPURayTracer::syncAndUploadShadingResult()
{
    if (!shadingFutures.empty() && shadingFutures[0].valid())
    {
        for (auto &future : shadingFutures)
        {
            future.get();
        }
        this->sampleCount++;
        this->imageTexture.setData(imageData.data());

        static auto lastTime = std::chrono::high_resolution_clock::now();
        auto currentTime = std::chrono::high_resolution_clock::now();
        auto elapsed = std::chrono::duration<double>(currentTime - lastTime).count();
        lastTime = currentTime;
        secPerSample = elapsed;
    }
}

void CPURayTracer::shadingAsync(int numThreads)
{
    if (!shadingFutures.empty() && shadingFutures[0].valid() && shadingFutures[0].wait_for(std::chrono::seconds(0)) != std::future_status::ready)
    {
        return;
    }
    syncAndUploadShadingResult();
    this->shadingFutures.clear();
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

void CPURayTracer::shade(int x, int y)
{
    vec2 uv = uvAt(x, y);
    vec4 &pixelColor = pixelAt(x, y);
    Ray ray(
        camera.position,
        camera.getRayDirction(uv) + Random::RandomVector(perturbStrength));
    auto newColor = castRay(ray);
    pixelColor = (pixelColor * static_cast<float>(sampleCount - 1.f) + newColor) / static_cast<float>(sampleCount);
}
