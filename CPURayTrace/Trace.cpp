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
#include "SimplifiedData.hpp"
#include <limits>

color4 Trace::CastRayDirectionLight(const Ray &ray, const color4 &light, const Scene &scene)
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

color4 Trace::CastRay(const Ray &ray, int traceDepth, const Scene &scene)
{
    float rr = traceDepth <= 1 ? 1.0f : Random::RussianRoulette(0.8f);

    if (traceDepth > bounceLimit || rr == 0.0f)
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
        return closestHit.pMaterial->getIrradiance(closestHit, traceDepth, scene) * rr;
    }
    // 未命中
    Sky sky;
    HitInfos hitSky;
    hitSky.dir = ray.getDirection();
    return sky.getIrradiance(hitSky, traceDepth, scene) * rr;
}

color4 Trace::CastRay(const Ray &ray, int traceDepth, sd::DataStorage &dataStorage)
{
    vec4 color = vec4(0.0f);
    vec3 throughout = vec3(1.f);
    Ray tracingRay = ray;
    while (traceDepth < bounceLimit)
    {

        traceDepth++;
        // 场景测试
        sd::HitInfos closestHit;
        closestHit = sd::BVH::IntersectLoop(dataStorage, tracingRay);
        // 命中场景
        if (closestHit.hit)
        {
            // color+= lambertianIrradiance(closestHit);
            // vec3 rndDir = sampleCosineHemisphere(closestHit.normal, TexCoord * (rand + 1.f));
            vec3 rndDir = Random::GenerateCosineSemiSphereVector(closestHit.normal);

            tracingRay = Ray(closestHit.pos, rndDir);
            throughout *= vec3(0.9f, 0.4f, 0.7f);
            // color = vec4(1.0f,0.0f,0.0f,0.0f);
            continue;
        }
        // 未命中
        // color.rgb += throughout * hitSky(tracingRay.ori, tracingRay.dir).rgb;
        color += vec4(throughout * vec3(0.4f), 1.0f);
        break;
    }

    return color;
}
