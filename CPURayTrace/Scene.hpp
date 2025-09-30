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
    std::vector<std::shared_ptr<Hittable>> objects;
    BVH BVHTree;

    inline Scene(){}
    // 拷贝构造
    inline Scene(const Scene &other)
        : objects(other.objects)
    {
        if (other.BVHTree.root)
        {
            BVHTree.build(objects);
        }
    }
    // 拷贝赋值
    inline Scene &operator=(const Scene &other)
    {
        objects = other.objects;
        if (other.BVHTree.root)
        {
            BVHTree.build(objects);
        }
        return *this;
    }

    void initialize(); // 布置场景

    inline void update()
    {
        BVHTree.build(objects);
    }

    inline void addObject(std::shared_ptr<Hittable> object)
    {
        objects.push_back(object);
    }

    inline HitInfos intersectClosestBVH(const Ray &ray) const
    {
        return BVHTree.intersect(ray);
    }
    inline HitInfos intersectClosest(const Ray &ray) const
    {
        HitInfos closestHit;
        for (auto &&object : objects)
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
