#pragma once
#include <glm/glm.hpp>
#include <vector>
#include <memory>
#include <algorithm>
#include <functional>
#include "Objects.hpp"

struct BVHNode
{
    BoundingBox box;
    BVHNode *left = nullptr;
    BVHNode *right = nullptr;
    std::shared_ptr<Hittable> object = nullptr; // 叶子节点存储物体
};

class BVHAccelerator
{
public:
    inline static BVHNode *BuildBVH(std::vector<std::shared_ptr<Hittable>> &objects, int start, int end) // [start, end)
    {
        if (end - start <= 0)
            return nullptr;
        if (end - start == 1)
        {
            BVHNode *leaf = new BVHNode();
            leaf->box = objects[start]->getBoundingBox();
            leaf->left = nullptr;
            leaf->right = nullptr;
            leaf->object = objects[start];
            return leaf;
        }
        if (end - start == 2)
        {
            BVHNode *node = new BVHNode();
            node->left = BuildBVH(objects, start, start + 1);
            node->right = BuildBVH(objects, start + 1, end);
            node->box.pMin = glm::min(node->left->box.pMin, node->right->box.pMin);
            node->box.pMax = glm::max(node->left->box.pMax, node->right->box.pMax);
            return node;
        }
        BVHNode *node = new BVHNode();
        for (int i = start; i < end; i++)
        {
            node->box.pMin = glm::min(node->box.pMin, objects[i]->getBoundingBox().pMin);
            node->box.pMax = glm::max(node->box.pMax, objects[i]->getBoundingBox().pMax);
        }
        float xExtent = node->box.pMax.x - node->box.pMin.x;
        float yExtent = node->box.pMax.y - node->box.pMin.y;
        float zExtent = node->box.pMax.z - node->box.pMin.z;
        // 选择最长轴进行划分
        if (xExtent >= yExtent && xExtent >= zExtent) // x轴最长
        {
            std::sort(objects.begin() + start, objects.begin() + end, [](const std::shared_ptr<Hittable> &a, const std::shared_ptr<Hittable> &b)
                      { return a->getBoundingBox().pMin.x + a->getBoundingBox().pMax.x < b->getBoundingBox().pMin.x + b->getBoundingBox().pMax.x; });
        }
        else if (yExtent >= xExtent && yExtent >= zExtent) // y轴最长
        {
            std::sort(objects.begin() + start, objects.begin() + end, [](const std::shared_ptr<Hittable> &a, const std::shared_ptr<Hittable> &b)
                      { return a->getBoundingBox().pMin.y + a->getBoundingBox().pMax.y < b->getBoundingBox().pMin.y + b->getBoundingBox().pMax.y; });
        }
        else // z轴最长
        {
            std::sort(objects.begin() + start, objects.begin() + end, [](const std::shared_ptr<Hittable> &a, const std::shared_ptr<Hittable> &b)
                      { return a->getBoundingBox().pMin.z + a->getBoundingBox().pMax.z < b->getBoundingBox().pMin.z + b->getBoundingBox().pMax.z; });
        }
        int mid = start + (end - start) / 2;
        node->left = BuildBVH(objects, start, mid);
        node->right = BuildBVH(objects, mid, end);
        return node;
    }
};

class BVH
{
public:
    BVHNode *root = nullptr;

    BVH() {}

    BVH(BVHNode *root) : root(root) {}

    ~BVH()
    {
        // 递归删除节点
        auto deleteNode = [&](auto &&self, BVHNode *node) -> void
        {
            if (!node)
                return;
            self(self, node->left);
            self(self, node->right);
            delete node;
        };
        deleteNode(deleteNode, root);
    }

    void build(std::vector<std::shared_ptr<Hittable>> &objects)
    {
        std::vector<std::shared_ptr<Hittable>> objsCopy = objects; // 复制一份,不改变原始顺序
        root = BVHAccelerator::BuildBVH(objsCopy, 0, objsCopy.size());
    }

    HitInfos intersect(const Ray &ray)
    {
        HitInfos closestHit;
        auto traverse = [&ray, &closestHit](auto &&traverseSelf, BVHNode *node) -> void
        {
            if (!node)
                return;
            if (!node->box.intersect(ray, 0.001f, closestHit.t)) // closestHit.t 是当前最近的命中距离，避免不必要的遍历
            {
                return;
            }
            if (node->object) // 叶子节点
            {
                // 展开求交
                auto hitInfos = node->object->intersect(ray);
                if (hitInfos && hitInfos->t < closestHit.t) // 代替原来的命中物体收集
                {
                    closestHit = *hitInfos;
                }
                return;
            }
            traverseSelf(traverseSelf, node->left);
            traverseSelf(traverseSelf, node->right);
        };
        traverse(traverse, root);
        return closestHit;
    }
};

// 全局静态场景类
class Scene
{
public:
    inline static std::vector<std::shared_ptr<Hittable>> Objects;
    inline static BVH BVHTree;

    inline static void Initialze() // 布置场景
    {
        Objects.push_back(
            std::make_shared<Sphere>(
                point3(2.0f, 0.0f, 2.f),
                1.5f,
                std::make_shared<Lambertian>(color4(0.7f, 0.1f, 0.15f, 1.0f))));
        Objects.push_back(
            std::make_shared<Sphere>(
                point3(3.0f, 1.5f, 1.f),
                1.5f,
                std::make_shared<Lambertian>(color4(0.2f, 0.7f, 0.1f, 1.0f))));
        Objects.push_back(
            std::make_shared<Sphere>(
                point3(-6.0f, 2.f, 5.f),
                1.5f,
                std::make_shared<Metal>(color4(0.8f, 0.7f, 0.2f, 1.0f), 0.99f)));
        Objects.push_back(
            std::make_shared<Sphere>(
                point3(-2.0f, 4.f, 2.f),
                1.5f,
                std::make_shared<Metal>(color4(0.7f, 0.7f, 0.4f, 1.0f), 0.5f)));
        Objects.push_back(
            std::make_shared<Sphere>(
                point3(-2.0f, 14.f, 7.f),
                1.5f,
                std::make_shared<Metal>(color4(0.7f, 0.7f, 0.4f, 1.0f), 0.5f)));
        Objects.push_back(
            std::make_shared<Sphere>(
                point3(-2.0f, 18.f, 9.f),
                1.5f,
                std::make_shared<Metal>(color4(0.7f, 0.7f, 0.4f, 1.0f), 0.5f)));
        Objects.push_back(
            std::make_shared<Sphere>(
                point3(-2.0f, 15.f, 0.f),
                4.f,
                std::make_shared<LightEmit>(color4(50.0f)))); // 光源
        Objects.push_back(
            std::make_shared<Sphere>(
                point3(0.f, -6.3e3f, 0.f),
                6.3e3f,
                std::make_shared<Lambertian>(color4(0.7f, 0.7f, 0.7f, 1.0f)))); // 地球
        for (size_t i = 0; i < 100; i++)
        {
            point3 center = point3(Random::RandomVector(40.f));
            center.y = glm::length(center) / 40.f+1.f ;
            Objects.push_back(
                std::make_shared<Sphere>(
                    center,
                    1.f,
                    std::make_shared<Lambertian>(color4((Random::RandomVector(1.0f) + 1.0f) / 2.f, 1.0f)

                                                     )));
        }
    }

    inline static void Update()
    {
        BVHTree.build(Objects);
    }

    inline static HitInfos IntersectClosestBVH(const Ray &ray)
    {
        return BVHTree.intersect(ray);
    }
    inline static HitInfos IntersectClosest(const Ray &ray)
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