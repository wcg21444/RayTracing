#pragma once
#include <glm/glm.hpp>

#include "Utils.hpp"
#include "Ray.hpp"
#include "Objects.hpp"
#include "Scene.hpp"
#include <limits>

inline color4 castRay(Ray &ray)
{
    // 深度测试
    HitInfos closestHit;
    closestHit.t = std::numeric_limits<float>::infinity();
    for (auto &&object : Scene::Objects)
    {
        auto hitInfos = object->intersect(ray);
        if (hitInfos && hitInfos->t < closestHit.t)
        {
            closestHit = *hitInfos;
        }
    }
    // 命中
    if (closestHit.t != std::numeric_limits<float>::infinity())
    {
        auto N = closestHit.normal;
        return 0.5f * color4(N.x + 1, N.y + 1, N.z + 1, 1.0f);
    }
    // 未命中
    float a = 0.5f * (ray.getDirection().y + 1.0f);
    return (1.0f - a) * vec4(1.0f) + a * vec4(0.0f, 0.0f, 1.0f, 1.0f);
}