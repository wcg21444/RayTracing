#pragma once
#include <glm/glm.hpp>

#include "Utils.hpp"
#include "Ray.hpp"
#include "Objects.hpp"
#include "Scene.hpp"
#include "Random.hpp"

#include <limits>

inline int bouanceLimit = 4;

inline color4 castRayDirectionLight(const Ray &ray, const color4 &light)
{
    HitInfos closestHit;
    closestHit.t = std::numeric_limits<float>::infinity();
    for (auto &&object : Scene::Objects)
    {
        auto hitInfos = object->intersect(ray);
        if (hitInfos)
        {
            return color4(0.0f);
        }
    }
    return light;
}

inline color4 castRay(const Ray &ray, int traceDepth)
{
    if (traceDepth > bouanceLimit)
    {
        return color4(0.0f);
    }
    // 深度测试
    // auto closestHit = Scene::IntersectClosest(ray);
    auto closestHit = Scene::IntersectClosestBVH(ray);
    // 命中
    if (closestHit.t != std::numeric_limits<float>::infinity())
    {
        return closestHit.pMaterial->getIrradiance(closestHit, traceDepth);
    }
    // 未命中
    Sky sky;
    HitInfos hitSky;
    hitSky.dir = ray.getDirection();
    return sky.getIrradiance(hitSky, traceDepth);
}