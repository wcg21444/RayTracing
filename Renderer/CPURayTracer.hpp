
#pragma once
#include "Shader.hpp"
#include "Utils.hpp"
#include "Camera.hpp"
#include "Renderer.hpp"
#include "Trace.hpp"
#include <future>
#include <vector>
#include <unordered_map>
#include <imgui.h>
#include <glm/gtc/type_ptr.hpp>
class Scene;
namespace SimplifiedData
{
    class Scene;
}
namespace sd = SimplifiedData;

#define GENERATE_CALLBACK_UNIQUE_ID() GenerateUniqueId(__FILE__, __LINE__)

constexpr size_t fnv1a_hash(const char *str, size_t hash = 14695981039346656037u)
{
    return *str ? fnv1a_hash(str + 1, (hash ^ static_cast<size_t>(*str)) * 1099511628211u) : hash;
}
constexpr size_t GenerateUniqueId(const char *file, size_t line)
{
    return fnv1a_hash(file) ^ line;
}

class SyncCallbackScheduler
{
public:
    std::unordered_map<size_t, std::function<void()>> syncCallbacks;

    void addCallback(size_t callbackHash, std::function<void()> func)
    {
        syncCallbacks[callbackHash] = func;
    }

    void executeAll()
    {
        for (auto &[id, func] : syncCallbacks)
        {
            func();
        }
        syncCallbacks.clear();
    }
};

class CPURayTracer
{
public:
    int width;
    int height;

private:
    std::unique_ptr<Scene> renderSceneUploaded = nullptr;
    std::unique_ptr<sd::Scene> sdRenderSceneUploaded = nullptr;

    const Scene *pNewScene;
    const sd::Scene *pNewSdScene;

    SyncCallbackScheduler syncCallbackScheduler;
    bool discardCurrentImage = false;

    bool usingSdScene = true;

private:
    Tracer tracer;
    Texture2D imageTexture;
    std::vector<std::future<void>> shadingFutures;
    float perturbStrength = 1e-3f;
    float secPerSample = 0.f;

    void syncBlocking();
    bool queryShadingTasksAllDone();
    void discardShadingResults();
    void shadeAsync(int numThreads);
    void sdShadeAsync(int numThreads);
    void interact();

public:
    CPURayTracer(int _width, int _height);
    ~CPURayTracer();
    unsigned int getGLTextureID();
    void resize(int newWidth, int newHeight);
    void resetSamples();
    void draw(int numThreads);
    void shutdown();
    void setScene(const Scene &sceneInput);
    void setSdScene(const sd::Scene &sceneInput);
};
