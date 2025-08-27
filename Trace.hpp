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

inline color4 castRay(const Ray &ray, int depth = 0)
{
    if (depth > bouanceLimit)
    {
        return color4(0.0f);
    }
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
        auto &[t, origin, dir, invDir, pos, normal, material] = closestHit;

        // 默认当前Material 为Diffuse
        if (closestHit.material.type == Material::Type::Diffuse)
        {
            vec3 bias = normal * 1e-4f; // 防止自相交
            vec3 rndDir = Random::GenerateCosineSemiSphereVector(normal);
            vec3 sunDir = DirectionOf(vec3(0.4f, 0.6f, 0.1f) + rndDir / 8.f);
            auto bounceRay = Ray(
                pos + bias,
                rndDir); // 半球随机方向
            color4 irradiance = material.albedo * castRay(bounceRay, depth + 1);
            color4 lightIrradiance = dot(normal, sunDir) * castRayDirectionLight(Ray(pos, sunDir), color4(1.2f));
            irradiance = (irradiance + lightIrradiance) / 2.f;

            return irradiance;
        }
        else
        {
            return color4(0.f); // TODO 其他材质
        }
    }
    // 未命中

    vec3 unit_direction = normalize(ray.getDirection());
    auto a = 0.5f * (unit_direction.y + 1.0f);
    return color4((1.0f - a) * vec3(1.0f, 1.0f, 1.0f) + a * vec3(0.5f, 0.7f, 1.0f), 1.0f);
}