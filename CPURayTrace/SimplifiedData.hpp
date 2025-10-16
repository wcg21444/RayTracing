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
    struct DataStorage;
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

    enum Matierals
    {
        LambertianMat,
        MetalMat,
        LightEmitMat,
        SkyMat
    };

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

    struct BoundingBox
    {
        vec3 pMin = vec3(FLT_MAX);
        vec3 pMax = vec3(FLT_MIN);
        BoundingBox &operator=(const BoundingBox &other);
    };
    // flag 决定跳转到 NodeStorage 还是 TriangleStorage
    struct Node
    {
        uint32_t left;
        uint32_t right;
        BoundingBox box;
        uint8_t flags; // 节点? 叶子节点? Mesh节点? 三角形节点?...
    };

    // 6xvec3+3xvec2+2bytes = 3*6*16+3*2*8+2 = 96*16+24+2 = 1538 bytes
    //  如果按 float 存储
    //  4*(3*6+2*3)+2 = 4*24+2=98 floats, 392 bytes,  400 bytes对齐
    struct Triangle
    {
        vec3 positions[3];
        vec3 normals[3];
        vec2 texCoords[3];
        uint16_t matFlags;
    };

    inline constexpr uint32_t TRIANGLESIZE = 1 << 20;          // 2^20 = 1048576 个三角形  不要用一个数组分配太大内存 否则 bad alloc
    inline constexpr uint32_t NODESIZE = TRIANGLESIZE * 2 - 1; // 完全二叉树节点数 = 2*n-1,也就是最大节点数

    class TriangleStorage
    {
    public:
        TriangleStorage();

        std::vector<Triangle> triangles;
        uint32_t nextIndex = 0;
        uint32_t addTriangle(const sd::Triangle &triangle);
        uint32_t addTriangleArray(std::vector<sd::Triangle> &triangles);

        ~TriangleStorage();
    };

    // |...三角形叶子节点...|....BVH内部节点....|end
    // |....三角形.........|end
    class NodeStorage
    {
    public:
        NodeStorage();

        std::vector<Node> nodes;
        uint32_t nextIndex = 0;
        uint32_t nextIndexBack = NODESIZE - 1;
        uint32_t addNode(const sd::Node &node);
        uint32_t addNodeBack(const sd::Node &node);
        uint32_t addLeafNodeArray(const std::vector<sd::Node> &nodes);

        ~NodeStorage();
    };

    struct DataStorage
    {
    public:
        TriangleStorage triangleStorage;
        NodeStorage nodeStorage;
        uint32_t rootIndex;
    };

    // 网格到底是什么呢? 网格最终数据结构只是一堆三角形,不是最终实际存储,是一个临时数据结构
    class Mesh
    {
    public:
        uint32_t offsetIndexTriangles = invalidIndex;
        uint32_t offsetIndexNodes = invalidIndex;
        uint32_t meshNodeIndex = invalidIndex;
        Mesh(DataStorage &dataStroage, const std::vector<Vertex> &vertices, const std::vector<unsigned int> &indices, const Material &_material);
    };

    class BVH
    {
    public:
        // 收集所有网格节点 拷贝, 然后排序 划分 最终还是指向存储中的索引[start, end)
        /// nodes: ... ... |TN2|TN1| SceneRoot|...|SI3|SI2|SI1|... ...|*Mesh2|M2I1|M2I2...|*Mesh1|M1I1|M1I2...|
        static uint32_t BuildBVHFromNodes(NodeStorage &nodeStorage, uint32_t *nodeIndices, size_t start, size_t end);
        static HitInfos Intersect(DataStorage &dataStorage, const Ray &ray);
        static HitInfos IntersectLoop(DataStorage &dataStorage, const Ray &ray);
    };

    sd::BoundingBox GetBoundingBox(const sd::Triangle &triangle);
    HitInfos IntersectTriangle(const Triangle &tri, const Ray &ray, float tMin, float tMax);
    bool operator==(const HitInfos &hit1, const HitInfos &hit2);
    bool IntersectBoundingBox(const BoundingBox &box, const Ray &ray, float tMin, float tMax);

    struct FlatNodeStorage
    {
        inline static constexpr size_t kFloatsPerNode = 2 /*indices*/ + 6 /*bbox*/ + 1 /*flags*/;
        inline size_t getSizeInBytes() { return kFloatsPerNode * sizeof(float)*nodes.size(); }
        FlatNodeStorage();

        std::vector<float> nodes;
        uint32_t rootIndex = sd::invalidIndex;
    };
    struct FlatTriangleStorage
    {
        inline static constexpr size_t kFloatsPerTriangle = 3 * 3 /*positions*/ + 3 * 3 /*normals*/ + 3 * 2 /*uv*/ + 1 /*mat flag*/;
        inline size_t getSizeInBytes() { return kFloatsPerTriangle * sizeof(float)*triangles.size(); }
        FlatTriangleStorage();

        std::vector<float> triangles;
    };
    inline constexpr size_t NODESIZESSBO = NODESIZE * FlatNodeStorage::kFloatsPerNode;
    inline constexpr size_t TRIANGLESIZESSBO = TRIANGLESIZE * FlatTriangleStorage::kFloatsPerTriangle;

    void ConvertNodeToFlatStorage(const NodeStorage &nodeStorage, FlatNodeStorage &flatNodeStorage);

    void ConvertTriangleToFlatStorage(const TriangleStorage &triangleStorage, FlatTriangleStorage &flatTriangleStorage);

    void ConvertToFlatStorage(const DataStorage &dataStorage, FlatNodeStorage &flatNodeStorage, FlatTriangleStorage &flatTriangleStorage);

    Node GetNodeFromFlatStorage(const FlatNodeStorage &flatNodeStorage, size_t index);

    Triangle GetTriangleFromFlatStorage(const FlatTriangleStorage &flatTriangleStorage, size_t index);

}
namespace sd = SimplifiedData;