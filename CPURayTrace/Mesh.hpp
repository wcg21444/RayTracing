#pragma once

#include "Objects.hpp"
#include "Materials.hpp"
#include "Bounding.hpp"
#include "BVH.hpp"
#include "BVHUI.hpp"
#include <optional>
#include <vector>

struct Vertex
{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoord;
};

class Triangle : public Hittable
{
public:
    Vertex vertices[3];
    std::unique_ptr<Material> pMaterial;
    BoundingBox boundingBox;

    Triangle(const Vertex &v0, const Vertex &v1, const Vertex &v2, const Material& _material)
        : pMaterial(_material.clone())
    {
        vertices[0] = v0;
        vertices[1] = v1;
        vertices[2] = v2;

        boundingBox.pMin = glm::min(v0.position, glm::min(v1.position, v2.position));
        boundingBox.pMax = glm::max(v0.position, glm::max(v1.position, v2.position));
        // 防止包围盒轴向过小
        boundingBox.pMin -= vec3(1e-6f);
        boundingBox.pMax += vec3(1e-6f);
    }

    std::optional<HitInfos> intersect(const Ray &ray) override
    {
        const float EPSILON = 1e-7f;
        vec3 edge1 = vertices[1].position - vertices[0].position;
        vec3 edge2 = vertices[2].position - vertices[0].position;
        vec3 h = glm::cross(ray.getDirection(), edge2);
        float a = glm::dot(edge1, h);
        if (a > -EPSILON && a < EPSILON)
            return std::nullopt; // This ray is parallel to this triangle.

        float f = 1.0f / a;
        vec3 s = ray.getOrigin() - vertices[0].position;
        float u = f * glm::dot(s, h);
        if (u < 0.0 || u > 1.0)
            return std::nullopt;

        vec3 q = glm::cross(s, edge1);
        float v = f * glm::dot(ray.getDirection(), q);
        if (v < 0.0 || u + v > 1.0)
            return std::nullopt;

        // At this stage we can compute t to find out where the intersection point is on the line.
        float t = f * glm::dot(edge2, q);
        if (t > EPSILON) // ray intersection
        {
            vec3 N = glm::normalize((1 - u - v) * vertices[0].normal + u * vertices[1].normal + v * vertices[2].normal);
            vec3 dir = ray.getDirection();
            return std::make_optional<HitInfos>(
                {t,
                 ray.getOrigin(),
                 dir,
                 vec3(1.f / dir.x, 1.f / dir.y, 1.f / dir.z),
                 ray.at(t),
                 N,
                 pMaterial.get()});
        }
        else // This means that there is a line intersection but not a ray intersection.
            return std::nullopt;
    }

    Material &getMaterial() override
    {
        return *pMaterial;
    }
    BoundingBox getBoundingBox() override
    {
        return boundingBox;
    }

    BVHNode *getInsideBVHRoot() override { return nullptr; }
    ~Triangle() {}
};

class Mesh : public Hittable
{
public:
    // Mesh = 顶点array + 索引array + 材质
    // 或者是 三角形array
    std::vector<std::shared_ptr<Triangle>> triangles;
    std::unique_ptr<Material> pMaterial;
    BoundingBox boundingBox;
    BVH insideBVH;

    Mesh(const std::vector<Vertex> &vertices, const std::vector<unsigned int> &indices, const Material& _material)
        : pMaterial(_material.clone())
    {
        triangles.reserve(indices.size() / 3);
        for (size_t i = 0; i < indices.size(); i += 3)
        {
            triangles.emplace_back(std::make_shared<Triangle>(
                vertices[indices[i]], vertices[indices[i + 1]], vertices[indices[i + 2]], *pMaterial));
        }
        if (!triangles.empty())
        {
            boundingBox = triangles[0]->getBoundingBox();
            for (size_t i = 1; i < triangles.size(); i++)
            {
                boundingBox.pMin = glm::min(boundingBox.pMin, triangles[i]->getBoundingBox().pMin);
                boundingBox.pMax = glm::max(boundingBox.pMax, triangles[i]->getBoundingBox().pMax);
            }
        }

        std::vector<std::shared_ptr<Hittable>> hittablePtrs;
        hittablePtrs.reserve(triangles.size());

        for (const auto &triPtr : triangles)
        {
            std::shared_ptr<Hittable> hittablePtr = triPtr;

            // 或者直接 push_back
            hittablePtrs.push_back(triPtr);
        }
        // build BVH
        insideBVH.build(hittablePtrs);
    }

    Mesh(const Mesh &other)
    {
        triangles = other.triangles;
        pMaterial = other.pMaterial ? other.pMaterial->clone() : nullptr;
        boundingBox = other.boundingBox;

        std::vector<std::shared_ptr<Hittable>> hittablePtrs;
        hittablePtrs.reserve(triangles.size());

        for (const auto &triPtr : triangles)
        {
            std::shared_ptr<Hittable> hittablePtr = triPtr;

            // 或者直接 push_back
            hittablePtrs.push_back(triPtr);
        }
        insideBVH.build(hittablePtrs);
    }

    std::optional<HitInfos> intersect(const Ray &ray) override
    {
        if (BVHSettings::toggleBVHAccel)

        {
            return std::make_optional(insideBVH.intersect(ray));
        }
        else
        {
            HitInfos closestHit;
            for (auto &&tri : triangles)
            {
                auto hitInfos = tri->intersect(ray);
                if (hitInfos && hitInfos->t < closestHit.t)
                {
                    closestHit = *hitInfos;
                }
            }
            return std::make_optional(closestHit);
        }
    }
    ~Mesh()
    {
    }
    Material &getMaterial() override
    {
        return *pMaterial;
    }
    BoundingBox getBoundingBox() override
    {
        return boundingBox;
    }

    BVHNode *getInsideBVHRoot() override { return insideBVH.root; }
};