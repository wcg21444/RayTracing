#pragma once
#include <glm/glm.hpp>
#include <vector>
#include <memory>
#include <algorithm>
#include <functional>
#include "Objects.hpp"
#include "BVH.hpp"
#include "Materials.hpp"

// 全局静态场景类
class Scene
{
public:
    inline static std::vector<std::shared_ptr<Hittable>> Objects;
    inline static BVH BVHTree;

    static void initialize(); // 布置场景

    inline static void update()
    {
        BVHTree.build(Objects);
    }

    inline static HitInfos intersectClosestBVH(const Ray &ray)
    {
        return BVHTree.intersect(ray);
    }
    inline static HitInfos intersectClosest(const Ray &ray)
    {
        HitInfos closestHit;
        for (auto &&object : Objects)
        {
            auto hitInfos = object->intersect(ray);
            if (hitInfos && hitInfos->t < closestHit.t)
            {
                closestHit = *hitInfos;
            }
        }
        return closestHit;
    }
};
