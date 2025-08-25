#pragma once
#include <glm/glm.hpp>

#include "Utils.hpp"
#include "Ray.hpp"
#include "Objects.hpp"
#include "Scene.hpp"
#include "Random.hpp"

#include <limits>

inline int bouanceLimit = 4;

inline color4 castRay(Ray &ray, int depth = 0)
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
        // Material处理
        // auto N = closestHit.normal;
        // return 0.5f * color4(N.x + 1, N.y + 1, N.z + 1, 1.0f);

        // 默认当前Material 为Diffuse
        if (closestHit.material.type == Material::Type::Diffuse)
        {
            auto &[t, origin, dir, invDir, pos, normal, material] = closestHit;
            Ray bounceRay = Ray(
                pos + normal * 1e-3f,
                Random::GenerateCosineSemiSphereVector(normal)); // 半球随机方向
            auto irradiance = material.albedo * castRay(bounceRay, depth + 1);
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
    // return color4(0.6f, 0.7f, 1.0f, 1.0f);
}