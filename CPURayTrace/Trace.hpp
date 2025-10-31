#pragma once
#include <glm/glm.hpp>

#include "Utils.hpp"
#include "Ray.hpp"
#include "Objects.hpp"
#include "Scene.hpp"

#include <limits>
namespace SimplifiedData
{
    struct DataStorage;
}
namespace Trace
{
    inline size_t bounceLimit = 4;

    color4 CastRayDirectionLight(const Ray &ray, const color4 &light, const Scene &scene);

    color4 CastRay(const Ray &ray, int traceDepth, const Scene &scene);

    color4 CastRay(const Ray &ray, int traceDepth, SimplifiedData::DataStorage &dataStorage);
}
