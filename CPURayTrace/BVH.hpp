#pragma once
#include "Bounding.hpp"
#include "Materials.hpp"
#include "Objects.hpp"

struct BVHNode
{
    BoundingBox box;
    BVHNode *left = nullptr;
    BVHNode *right = nullptr;
    std::shared_ptr<Hittable> object = nullptr; // 叶子节点存储物体
};

class BVH
{
public:
    BVHNode *root = nullptr;

    // TODO 并行化构建
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

    inline BVH() {}

    inline BVH(BVHNode *root) : root(root) {}

    // 拷贝构造
    inline BVH(const BVH &other)
    {
        root = new BVHNode(*other.root);
        auto copy = [](auto &&copySelf, BVHNode *copyNode, BVHNode *otherNode)
        {
            if (!otherNode)
                return;
            *copyNode = *otherNode;
            if (otherNode->left)
            {
                copyNode->left = new BVHNode();
                copySelf(copySelf, copyNode->left, otherNode->left);
            }
            if (otherNode->right)
            {
                copyNode->right = new BVHNode();
                copySelf(copySelf, copyNode->right, otherNode->right);
            }
        };
        copy(copy, root, other.root);
    }

    inline ~BVH()
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

    inline void build(std::vector<std::shared_ptr<Hittable>> &objects) // 构建的BVH叶子节点指向实际存储
    {
        root = BVH::BuildBVH(objects, 0, static_cast<int>(objects.size()));
    }

    inline HitInfos intersect(const Ray &ray) const
    {
        HitInfos closestHit;
        auto traverse = [&ray, &closestHit](auto &&traverseSelf, BVHNode *node) -> void
        {
            if (!node || !node->box.intersect(ray, 1e-6f, closestHit.t))
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
