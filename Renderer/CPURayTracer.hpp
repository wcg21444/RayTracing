
#pragma once
#include "Shader.hpp"
#include "Utils.hpp"
#include "Camera.hpp"
#include "Renderer.hpp"
#include <future>
#include <vector>
#include <unordered_map>
#include <imgui.h>
#include <glm/gtc/type_ptr.hpp>
class Scene;

//Emmmmm  实在没什么好的hash callback function的办法
inline size_t GenerateUniqueCallbackID(int lineNumber, const char *fileName)
{
    return std::hash<std::string>()(std::string(fileName) + std::to_string(lineNumber));
}

#define HASH_CALLBACK \
    GenerateUniqueCallbackID(__LINE__, __FILE__)
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
    std::unique_ptr<Scene> renderScene;
    SyncCallbackScheduler syncCallbackScheduler;
    bool discardCurrentImage = false;

private:
    Texture2D imageTexture;
    std::vector<vec4> imageData;
    std::vector<std::future<void>> shadingFutures;
    int sampleCount = 1;
    float perturbStrength = 1e-3f;
    float secPerSample = 0.f;

    void setPixel(int x, int y, vec4 &value);
    vec4 &pixelAt(int x, int y);
    vec2 uvAt(int x, int y);
    void syncBlocking();
    bool queryShadingTasksAllDone();
    void discardShadingResults();
    void shadeAsync(int numThreads, const Scene &sceneInput);
    void shade(int x, int y);

public:
    CPURayTracer(int _width, int _height);
    ~CPURayTracer();
    unsigned int getGLTextureID();
    void resize(int newWidth, int newHeight);
    void resetSamples();
    void draw(int numThreads, const Scene &sceneInput);
    void shutdown();
};
