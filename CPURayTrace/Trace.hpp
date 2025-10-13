#pragma once
#include <glm/glm.hpp>

#include "Utils.hpp"
#include "Ray.hpp"
#include "Objects.hpp"
#include "Scene.hpp"

#include <limits>
namespace SimplifiedData
{
    class DataStorage;
}
namespace Trace
{
    inline size_t bounceLimit = 10;

    color4 CastRayDirectionLight(const Ray &ray, const color4 &light, const Scene &scene);

    color4 CastRay(const Ray &ray, int traceDepth, const Scene &scene);

    color4 CastRay(const Ray &ray, int traceDepth, SimplifiedData::DataStorage &dataStorage);
}

class Tracer
{
private:
    int width;
    int height;

    std::unique_ptr<Scene> pSceneTracing = nullptr;
    std::unique_ptr<SimplifiedData::Scene> pSdSceneTracing = nullptr;

    const Scene *pNewScene = nullptr;
    const SimplifiedData::Scene *pNewSdScene = nullptr;
    std::vector<vec4> imageData;


private:
    void setPixel(int x, int y, vec4 &value);
    vec4 &pixelAt(int x, int y);
    vec2 uvAt(int x, int y);

public:
    size_t sampleCounts = 1;
    float perturbStrength=1e-3f;

    size_t bounceLimit = 10;

    Tracer(int _width, int _height);

    void resize(int newWidth, int newHeight);

    void resetSamples();

    void setScene(const Scene &scene);
    void setSdScene(const SimplifiedData::Scene &sdScene);

    void shade(int x, int y);
    void sdShade(int x, int y);

    void uploadSdScene();
    void uploadScene();

    inline bool isSdSceneShadingLoaded(){
        return pSdSceneTracing != nullptr;
    }
    inline bool isSceneShadingLoaded(){
        return pSceneTracing !=nullptr;
    }

    std::vector<vec4> getImageData();
};
