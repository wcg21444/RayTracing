#pragma once

#include "Ray.hpp"
#include "Materials.hpp"
#include <optional>

class Hittable
{
public:
    virtual ~Hittable() {}

    virtual std::optional<HitInfos> intersect(const Ray &ray) = 0; // 不命中可以返回nullopt
};

class Sphere : public Hittable
{

public:
    float radius;
    point3 center;
    std::shared_ptr<Material> pMaterial;

    Sphere(const point3 &center, float radius, std::shared_ptr<Material> pMaterial)
        : center(center),
          radius(radius),
          pMaterial(pMaterial)
    {
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
                 pMaterial});
        }
    }
    ~Sphere() {}
};