#pragma once

#include "Ray.hpp"
#include "Materials.hpp"
#include "Bounding.hpp"

#include <optional>
#include <vector>

class BVHNode;

class Hittable
{
public:
    virtual ~Hittable() {}

    virtual std::optional<HitInfos> intersect(const Ray &ray) = 0; // 不命中返回nullopt

    virtual Material &getMaterial() = 0;

    virtual BoundingBox getBoundingBox() = 0;

    virtual BVHNode *getInsideBVHRoot() = 0;
};

class Sphere : public Hittable
{

public:
    float radius;
    point3 center;
    std::unique_ptr<Material> pMaterial;
    BoundingBox boundingBox;

    Sphere(const point3 &center, float radius, const Material& _material)
        : center(center),
          radius(radius),
          pMaterial(_material.clone())
    {
        boundingBox.pMin = center - vec3(radius);
        boundingBox.pMax = center + vec3(radius);
    }
    std::optional<HitInfos> intersect(const Ray &ray) override
    {
        vec3 oc = center - ray.getOrigin();
        float a = dot(ray.getDirection(), ray.getDirection());
        float b = -2.0f * dot(ray.getDirection(), oc);
        float c = dot(oc, oc) - radius * radius;
        float discriminant = b * b - 4 * a * c;
        if (discriminant < 0)
        {
            return std::nullopt;
        }
        else
        {
            float t = (-b - std::sqrt(discriminant)) / (2.0f * a);
            if (t < 0)
            {
                return std::nullopt;
            }
            vec3 N = glm::normalize(ray.at(t) - center);
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
    ~Sphere() {}
};
