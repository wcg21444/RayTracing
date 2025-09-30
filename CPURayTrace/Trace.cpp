#pragma once
#include <glm/glm.hpp>

#include "Utils.hpp"
#include "Ray.hpp"
#include "Objects.hpp"
#include "Random.hpp"
#include "Materials/Sky.hpp"
#include "BVHUI.hpp"
#include "Scene.hpp"
#include "Trace.hpp"

#include <limits>

color4 Trace::CastRayDirectionLight(const Ray &ray, const color4 &light,const Scene& scene)
{
    HitInfos closestHit;
    closestHit.t = std::numeric_limits<float>::infinity();
    for (auto &&object : scene.objects)
    {
        auto hitInfos = object->intersect(ray);
        if (hitInfos)
        {
            return color4(0.0f);
        }
    }
    return light;
}

color4 Trace::CastRay(const Ray &ray, int traceDepth, const Scene& scene)
{
    float rr = traceDepth <= 1 ? 1.0f : Random::RussianRoulette(0.8f);

    if (traceDepth > bouanceLimit || rr == 0.0f)
    {
        return color4(0.0f);
    }
    // 深度测试
    // auto closestHit = scene.IntersectClosest(ray);
    HitInfos closestHit;
    if (BVHSettings::toggleBVHAccel)
        closestHit = scene.intersectClosestBVH(ray);
    else
        closestHit = scene.intersectClosest(ray);
    // 命中
    if (closestHit.t != std::numeric_limits<float>::infinity())
    {
        return closestHit.pMaterial->getIrradiance(closestHit, traceDepth,scene) * rr;
    }
    // 未命中
    Sky sky;
    HitInfos hitSky;
    hitSky.dir = ray.getDirection();
    return sky.getIrradiance(hitSky, traceDepth,scene) * rr;
}