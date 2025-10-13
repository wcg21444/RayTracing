#pragma once

#include "Materials.hpp"
#include "Ray.hpp"
#include "Utils.hpp"

#include <optional>
#include <vector>
#include <array>
#include <memory>
#include <stdexcept>
#include <stack>
#include <algorithm>
namespace SimplifiedData
{
    namespace sd = SimplifiedData;
    // fwd declaration
    struct Node;
    struct BoundingBox;
    struct Triangle;
    struct HitInfos;
    class TriangleStorage;
    class NodeStorage;
    class Mesh;
    class DataStorage;
    class BVH;

    inline const uint32_t invalidIndex = uint32_t(-1);

    enum NodeFlags : uint8_t
    {
        NODE_INTERNAL = 0,
        NODE_LEAF = 1,
        NODE_MESH = 2,
        NODE_TRIANGLE = 3
    };
    struct Vertex
    {
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec2 texCoord;
    };
    struct BoundingBox
    {
        vec3 pMin = vec3(FLT_MAX);
        vec3 pMax = vec3(FLT_MIN);
        BoundingBox &operator=(const BoundingBox &other);
    };
    bool IntersectBoundingBox(const BoundingBox &box, const Ray &ray, float tMin, float tMax);

    struct HitInfos
    {
        bool hit = false;
        float t = std::numeric_limits<float>::infinity(); // 命中时光线的t
        glm::vec3 origin;                                 // 光线起点
        glm::vec3 dir;                                    // 命中光线的dir
        glm::vec3 invDir;                                 // 光线dir倒数
        glm::vec3 pos;                                    // 命中位置
        glm::vec3 normal;                                 // 归一化世界法线
        uint16_t matFlags;                                // 材质
    };

    bool operator==(const HitInfos &hit1, const HitInfos &hit2);

    // flag 决定跳转到 NodeStorage 还是 TriangleStorage
    struct Node
    {
        uint32_t left;
        uint32_t right;
        BoundingBox box;
        uint8_t flags; // 节点? 叶子节点? Mesh节点? 三角形节点?...
    };
    struct Triangle
    {
        vec3 positions[3];
        vec3 normals[3];
        vec2 texCoords[3];
        uint16_t matFlags;
    };
    HitInfos IntersectTriangle(const Triangle &tri, const Ray &ray, float tMin, float tMax);

    // 网格到底是什么呢? 网格最终数据结构只是一堆三角形,不是最终实际存储,是一个临时数据结构
    class Mesh
    {
    public:
        uint32_t offsetIndexTriangles = invalidIndex;
        uint32_t offsetIndexNodes = invalidIndex;
        uint32_t meshNodeIndex = invalidIndex;
        Mesh(DataStorage &dataStroage, const std::vector<Vertex> &vertices, const std::vector<unsigned int> &indices, const Material &_material);
    };

    const uint32_t TRIANGLESIZE = 1 << 16; // 假设最多有65536个三角形
    class TriangleStorage
    {
    public:
        std::array<Triangle, TRIANGLESIZE> triangles;
        uint32_t nextIndex = 0;
        uint32_t addTriangle(const sd::Triangle &triangle);
        uint32_t addTriangleArray(std::vector<sd::Triangle> &triangles);

        ~TriangleStorage();
    };

    // |...三角形叶子节点...|....BVH内部节点....|end
    // |....三角形.........|end
    const uint32_t NODESIZE = TRIANGLESIZE * 2 - 1; // 完全二叉树节点数 = 2*n-1,也就是最大节点数
    class NodeStorage
    {
    public:
        std::array<Node, NODESIZE> nodes;
        uint32_t nextIndex = 0;
        uint32_t nextIndexBack = NODESIZE - 1;
        uint32_t addNode(const sd::Node &node);
        uint32_t addNodeBack(const sd::Node &node);
        uint32_t addLeafNodeArray(const std::vector<sd::Node> &nodes);

        ~NodeStorage();
    };

    sd::BoundingBox GetBoundingBox(const sd::Triangle &triangle);

    class DataStorage
    {
    public:
        TriangleStorage triangleStorage;
        NodeStorage nodeStorage;
        uint32_t rootIndex;
    };

    class BVH
    {
    public:
        // 收集所有网格节点 拷贝, 然后排序 划分 最终还是指向存储中的索引[start, end)
        /// nodes: ... ... |TN2|TN1| SceneRoot|...|SI3|SI2|SI1|... ...|*Mesh2|M2I1|M2I2...|*Mesh1|M1I1|M1I2...|
        inline static uint32_t BuildBVHFromNodes(NodeStorage &nodeStorage, uint32_t *nodeIndices, size_t start, size_t end)
        {
            auto &nodes = nodeStorage.nodes;
            // ISSUE 叶子节点重定向
            if (end - start <= 0)
                // return sd::invalidIndex;
                throw std::runtime_error("Build Failed. end - start <= 0 ");
            if (end - start == 1)
            {
                return nodeIndices[start]; // start 是相对inputNodes 的位置 ,不一定是strorage索引值
            }
            if (end - start == 2)
            {
                uint32_t leftIndex = nodeIndices[start];
                uint32_t rightIndex = nodeIndices[start + 1];

                auto &leftNode = nodes[leftIndex];
                auto &rightNode = nodes[rightIndex];
                BoundingBox box;
                box.pMin = glm::min(leftNode.box.pMin, rightNode.box.pMin);
                box.pMax = glm::max(leftNode.box.pMax, rightNode.box.pMax);
                return nodeStorage.addNodeBack(
                    Node{
                        .left = leftIndex,
                        .right = rightIndex,
                        .box = box,
                        .flags = NODE_INTERNAL,
                    });
            }
            // 分治
            BoundingBox nodeBox;

            for (uint32_t i = start; i < end; i++)
            {
                auto &subNode = nodes[nodeIndices[i]];
                auto subBox = subNode.box;
                nodeBox.pMin = glm::min(nodeBox.pMin, subBox.pMin);
                nodeBox.pMax = glm::max(nodeBox.pMax, subBox.pMax);
            }
            float xExtent = nodeBox.pMax.x - nodeBox.pMin.x;
            float yExtent = nodeBox.pMax.y - nodeBox.pMin.y;
            float zExtent = nodeBox.pMax.z - nodeBox.pMin.z;

            static auto compx = [&nodes](const uint32_t &a, const uint32_t &b)
            {
                return nodes[a].box.pMin.x + nodes[a].box.pMax.x < nodes[b].box.pMin.x + nodes[b].box.pMax.x;
            };
            static auto compy = [&nodes](const uint32_t &a, const uint32_t &b)
            {
                return nodes[a].box.pMin.y + nodes[a].box.pMax.y < nodes[b].box.pMin.y + nodes[b].box.pMax.y;
            };
            static auto compz = [&nodes](const uint32_t &a, const uint32_t &b)
            {
                return nodes[a].box.pMin.z + nodes[a].box.pMax.z < nodes[b].box.pMin.z + nodes[b].box.pMax.z;
            };
            // 选择最长轴进行划分
            if (xExtent >= yExtent && xExtent >= zExtent) // x轴最长
            {
                std::sort(nodeIndices + start, nodeIndices + end, compx);
            }
            else if (yExtent >= xExtent && yExtent >= zExtent) // y轴最长
            {
                std::sort(nodeIndices + start, nodeIndices + end, compy);
            }
            else // z轴最长
            {
                std::sort(nodeIndices + start, nodeIndices + end, compz);
            }
            int mid = start + (end - start) / 2;
            uint32_t leftIndex = BuildBVHFromNodes(nodeStorage, nodeIndices, start, mid);
            uint32_t rightIndex = BuildBVHFromNodes(nodeStorage, nodeIndices, mid, end);
            uint32_t nodeIndex = nodeStorage.addNodeBack(Node{
                .left = leftIndex,
                .right = rightIndex,
                .box = nodeBox,
                .flags = NODE_INTERNAL,
            });
            return nodeIndex;
        }

        inline static HitInfos Intersect(DataStorage &dataStorage, const Ray &ray)
        {

            HitInfos closestHit;
            auto traverse = [&ray, &closestHit, &dataStorage](auto &&traverseSelf, uint32_t nodeIndex) -> void
            {
                Node node = dataStorage.nodeStorage.nodes[nodeIndex];
                if (nodeIndex == sd::invalidIndex || !sd::IntersectBoundingBox(node.box, ray, 1e-6f, closestHit.t))
                {
                    return;
                }
                if (node.flags == NODE_LEAF) // 叶子节点
                {
                    // 展开求交
                    auto tri = dataStorage.triangleStorage.triangles[node.left];
                    auto hitInfos = sd::IntersectTriangle(tri, ray, 1e-6f, closestHit.t);
                    if (hitInfos.hit && hitInfos.t < closestHit.t) // 代替原来的命中物体收集
                    {
                        closestHit = hitInfos;
                    }
                    return;
                }
                traverseSelf(traverseSelf, node.left);
                traverseSelf(traverseSelf, node.right);
            };
            traverse(traverse, dataStorage.rootIndex);
            return closestHit;
        }

        inline static HitInfos IntersectLoop(DataStorage &dataStorage, const Ray &ray)
        {
            static thread_local std::array<uint32_t, 32> callStack; // 假设栈深度不会超过32
            static thread_local size_t top = 0;
            HitInfos closestHit;

            callStack[top++] = dataStorage.rootIndex;

            while (top > 0)
            {
                uint32_t index = callStack[--top];
                const Node &node = dataStorage.nodeStorage.nodes[index];

                if (index == sd::invalidIndex || !sd::IntersectBoundingBox(node.box, ray, 1e-6f, closestHit.t))
                {
                    continue;
                }
                if (node.flags == NODE_LEAF) // 叶子节点
                {
                    // 展开求交
                    const auto &tri = dataStorage.triangleStorage.triangles[node.left];
                    auto hitInfos = sd::IntersectTriangle(tri, ray, 1e-6f, closestHit.t);
                    if (hitInfos.hit && hitInfos.t < closestHit.t) // 代替原来的命中物体收集
                    {
                        closestHit = hitInfos;
                    }
                    continue;
                }
                callStack[top++] = node.left;
                callStack[top++] = node.right;
            }
            return closestHit;
        }
    };

}
namespace sd = SimplifiedData;