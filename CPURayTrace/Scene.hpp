#pragma once
#include <glm/glm.hpp>
#include <vector>
#include <memory>
#include <algorithm>
#include <functional>
#include "Objects.hpp"
#include "BVH.hpp"
#include "Materials.hpp"
#include "SimplifiedData.hpp"

// 全局静态场景类
class Scene
{
public:
    std::vector<std::shared_ptr<Hittable>> objects;
    BVH BVHTree;

    inline Scene()
    {
        initialize();
    }
    // 拷贝构造
    inline Scene(const Scene &other)
        : objects(other.objects), BVHTree(other.BVHTree)
    {
        if (other.BVHTree.root)
        {
            std::unordered_map<std::shared_ptr<Hittable>, int> indexHash;
            for (int i = 0; i < objects.size(); ++i)
            {
                if (objects[i] != nullptr)
                {
                    indexHash[objects[i]] = i;
                }
            }
            auto remapping = [this, &indexHash](auto &&traverseSelf, BVHNode *node) -> void
            {
                if (!node)
                {
                    return;
                }
                if (node->object) // 叶子节点
                {
                    node->object = objects[indexHash[node->object]];
                }
                traverseSelf(traverseSelf, node->left);
                traverseSelf(traverseSelf, node->right);
            };
            remapping(remapping, BVHTree.root);

            // BVHTree.build(objects);
        }
    }
    // 拷贝赋值
    // 深拷贝场景 BVH
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

namespace SimplifiedData
{
    class Scene
    {
    public:
        std::unique_ptr<sd::DataStorage> pDataStorage = nullptr;
        std::vector<uint32_t> sceneIndices;

        Scene();

        //拷贝
        Scene(const Scene &other) ;
        Scene &operator=(const Scene &other);
        

        void initialize(); // 布置场景
    };
}