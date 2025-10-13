#include "SimplifiedData.hpp"

#include <exception>
namespace SimplifiedData
{

    // TriangleStorage 成员函数定义
    uint32_t TriangleStorage::addTriangle(const sd::Triangle &triangle)
    {
        if (nextIndex < TRIANGLESIZE)
        {
            this->triangles[nextIndex++] = triangle;
        }
        return nextIndex - 1;
    }

    // move triangles to storage
    uint32_t TriangleStorage::addTriangleArray(std::vector<sd::Triangle> &triangles)
    {
        uint32_t startIndex = nextIndex;
        if (nextIndex + triangles.size() > TRIANGLESIZE)
        {
            throw std::runtime_error("TriangleStorage overflow: Exceeded maximum triangle storage size.");
        }
        for (const auto &tri : triangles)
        {
            this->triangles[this->nextIndex++] = tri;
        }
        return startIndex;
    }

    TriangleStorage::~TriangleStorage()
    {
    }

    // NodeStorage 成员函数定义
    uint32_t NodeStorage::addNode(const sd::Node &node)
    {
        if (nextIndex < nodes.size())
        {
            nodes[nextIndex++] = node;
        }
        return nextIndex - 1;
    }

    uint32_t NodeStorage::addNodeBack(const sd::Node &node)
    {
        if (nextIndexBack >= nextIndex)
        {
            nodes[nextIndexBack--] = node;
        }
        return nextIndexBack + 1;
    }

    uint32_t sd::NodeStorage::addLeafNodeArray(const std::vector<sd::Node> &nodes)
    {
        uint32_t startIndex = nextIndex;
        if (nextIndex + nodes.size() > this->nodes.size())
        {
            throw std::runtime_error("NodeStorage overflow: Exceeded maximum node storage size.");
        }
        for (const auto &node : nodes)
        {
            this->nodes[nextIndex++] = node;
        }
        return startIndex;
    }

    NodeStorage::~NodeStorage()
    {
    }

    // Mesh 构造函数定义
    // 构造叶子节点
    // 将三角形数据加入存储区
    Mesh::Mesh(DataStorage &dataStroage, const std::vector<Vertex> &vertices, const std::vector<unsigned int> &indices, const Material &_material)
    {
        auto &triangleStorage = dataStroage.triangleStorage;
        auto &nodeStorage = dataStroage.nodeStorage;

        std::vector<uint32_t> nodeIndices;
        for (uint32_t i = 0; i < indices.size(); i += 3)
        {
            Triangle tri;
            auto &v0 = vertices[indices[i]];
            auto &v1 = vertices[indices[i + 1]];
            auto &v2 = vertices[indices[i + 2]];
            tri.positions[0] = v0.position;
            tri.positions[1] = v1.position;
            tri.positions[2] = v2.position;
            tri.normals[0] = v0.normal;
            tri.normals[1] = v1.normal;
            tri.normals[2] = v2.normal;
            tri.texCoords[0] = v0.texCoord;
            tri.texCoords[1] = v1.texCoord;
            tri.texCoords[2] = v2.texCoord;
            tri.matFlags = 0; // TODO
            uint32_t triangleIndex = triangleStorage.addTriangle(tri);

            Node leafNode;
            leafNode.left = triangleIndex;
            leafNode.right = triangleIndex;
            leafNode.box = GetBoundingBox(tri);
            leafNode.flags = NODE_LEAF;
            uint32_t leafNodeIndex = nodeStorage.addNode(leafNode);
            nodeIndices.push_back(leafNodeIndex);

            // 更新Mesh 包围盒
            // meshNode.box.pMin = glm::min(meshNode.box.pMin, glm::min(v0.position, glm::min(v1.position, v2.position)));
            // meshNode.box.pMax = glm::max(meshNode.box.pMax, glm::max(v0.position, glm::max(v1.position, v2.position)));
        }

        meshNodeIndex = sd::BVH::BuildBVHFromNodes(nodeStorage, nodeIndices.data(), 0, nodeIndices.size());
    }

    // BoundingBox 拷贝赋值
    BoundingBox &BoundingBox::operator=(const BoundingBox &other)
    {
        this->pMin = other.pMin;
        this->pMax = other.pMax;
        return *this;
    }

    bool IntersectBoundingBox(const BoundingBox &box, const Ray &ray, float tMin, float tMax)
    {
        for (int a = 0; a < 3; a++)
        {
            float invD = ray.getInvDirection()[a];
            float t0 = (box.pMin[a] - ray.getOrigin()[a]) * invD;
            float t1 = (box.pMax[a] - ray.getOrigin()[a]) * invD;
            if (invD < 0.0f)
                std::swap(t0, t1);
            tMin = t0 > tMin ? t0 : tMin;
            tMax = t1 < tMax ? t1 : tMax;
            if (tMax <= tMin)
                return false;
        }
        return true;
    }
    bool operator==(const HitInfos &hit1, const HitInfos &hit2)
    {
        bool eq = true;
        eq &= (hit1.hit == hit2.hit);
        eq &= (hit1.t == hit2.t);
        eq &= (hit1.origin == hit2.origin);
        eq &= (hit1.dir == hit2.dir);
        eq &= (hit1.invDir == hit2.invDir);
        eq &= (hit1.pos == hit2.pos);
        eq &= (hit1.normal == hit2.normal);
        eq &= (hit1.matFlags == hit2.matFlags);
        return eq;
    }
    HitInfos IntersectTriangle(const Triangle &tri, const Ray &ray, float tMin, float tMax)
    {
        const float EPSILON = 1e-7f;
        vec3 edge1 = tri.positions[1] - tri.positions[0];
        vec3 edge2 = tri.positions[2] - tri.positions[0];
        vec3 h = glm::cross(ray.getDirection(), edge2);
        float a = glm::dot(edge1, h);
        if (a > -EPSILON && a < EPSILON)
            return HitInfos{}; // This ray is parallel to this triangle.
        float f = 1.0f / a;
        vec3 s = ray.getOrigin() - tri.positions[0];
        float u = f * glm::dot(s, h);
        if (u < 0.0 || u > 1.0)
            return HitInfos{};

        vec3 q = glm::cross(s, edge1);
        float v = f * glm::dot(ray.getDirection(), q);
        if (v < 0.0 || u + v > 1.0)
            return HitInfos{};

        // At this stage we can compute t to find out where the intersection point is on the line.
        float t = f * glm::dot(edge2, q);
        if (t > EPSILON) // ray intersection
        {
            vec3 N = glm::normalize((1 - u - v) * tri.normals[0] + u * tri.normals[1] + v * tri.normals[2]);
            vec3 dir = ray.getDirection();
            return HitInfos{
                .hit = true,
                .t = t,
                .origin = ray.getOrigin(),
                .dir = dir,
                .invDir = vec3(1.f / dir.x, 1.f / dir.y, 1.f / dir.z),
                .pos = ray.at(t),
                .normal = N,
                .matFlags = tri.matFlags};
        }
        else // This means that there is a line intersection but not a ray intersection.
            return HitInfos{};
    }
    // GetBoundingBox
    BoundingBox GetBoundingBox(const Triangle &triangle)
    {
        BoundingBox box;
        box.pMin = glm::min(triangle.positions[0], glm::min(triangle.positions[1], triangle.positions[2]));
        box.pMax = glm::max(triangle.positions[0], glm::max(triangle.positions[1], triangle.positions[2]));
        box.pMin -= vec3(1e-6f);
        box.pMax += vec3(1e-6f);
        return box;
    }

} // namespace SimplifiedData
