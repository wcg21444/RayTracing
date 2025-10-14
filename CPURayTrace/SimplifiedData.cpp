#include "SimplifiedData.hpp"

#include <exception>
namespace SimplifiedData
{

    // TriangleStorage 成员函数定义
    uint32_t TriangleStorage::addTriangle(const sd::Triangle &_triangle)
    {
        if (nextIndex + 1 > TRIANGLESIZE)
        {
            throw std::runtime_error("TriangleStorage overflow: Exceeded maximum triangle storage size.");
        }
        if (nextIndex < TRIANGLESIZE)
        {
            this->triangles[nextIndex++] = _triangle;
        }
        return nextIndex - 1;
    }

    // move triangles to storage
    uint32_t TriangleStorage::addTriangleArray(std::vector<sd::Triangle> &_triangles)
    {
        uint32_t startIndex = nextIndex;
        if (nextIndex + _triangles.size() > TRIANGLESIZE)
        {
            throw std::runtime_error("TriangleStorage overflow: Exceeded maximum triangle storage size.");
        }
        for (const auto &tri : _triangles)
        {
            this->triangles[this->nextIndex++] = tri;
        }
        return startIndex;
    }

    TriangleStorage::~TriangleStorage()
    {
    }

    // NodeStorage 成员函数定义
    uint32_t NodeStorage::addNode(const sd::Node &_node)
    {
        if (nextIndex + 1 > this->nodes.size())
        {
            throw std::runtime_error("NodeStorage overflow: Exceeded maximum node storage size.");
        }

        if (nextIndex < nodes.size())
        {
            nodes[nextIndex++] = _node;
        }
        return nextIndex - 1;
    }

    uint32_t NodeStorage::addNodeBack(const sd::Node &_node)
    {
        if (nextIndex + 1 > this->nodes.size())
        {
            throw std::runtime_error("NodeStorage overflow: Exceeded maximum node storage size.");
        }
        if (nextIndexBack >= nextIndex)
        {
            nodes[nextIndexBack--] = _node;
        }
        return nextIndexBack + 1;
    }

    uint32_t sd::NodeStorage::addLeafNodeArray(const std::vector<sd::Node> &_nodes)
    {
        uint32_t startIndex = nextIndex;
        if (nextIndex + _nodes.size() > this->nodes.size())
        {
            throw std::runtime_error("NodeStorage overflow: Exceeded maximum node storage size.");
        }
        for (const auto &node : _nodes)
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
            tri.matFlags = LambertianMat; // TODO
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

    uint32_t BVH::BuildBVHFromNodes(NodeStorage &nodeStorage, uint32_t *nodeIndices, size_t start, size_t end)
    {
        auto &nodes = nodeStorage.nodes;
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

        auto compx = [&nodes](const uint32_t &a, const uint32_t &b)
        {
            return nodes[a].box.pMin.x + nodes[a].box.pMax.x < nodes[b].box.pMin.x + nodes[b].box.pMax.x;
        };
        auto compy = [&nodes](const uint32_t &a, const uint32_t &b)
        {
            return nodes[a].box.pMin.y + nodes[a].box.pMax.y < nodes[b].box.pMin.y + nodes[b].box.pMax.y;
        };
        auto compz = [&nodes](const uint32_t &a, const uint32_t &b)
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
        size_t mid = start + (end - start) / 2;
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

    HitInfos BVH::Intersect(DataStorage &dataStorage, const Ray &ray)
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

    HitInfos BVH::IntersectLoop(DataStorage &dataStorage, const Ray &ray)
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

} // namespace SimplifiedData
