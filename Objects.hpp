#pragma once

#include "Ray.hpp"
#include <optional>

struct HitInfos
{
    float t;     // 命中时光线的t
    vec3 origin; // 光线起点
    vec3 dir;    // 命中光线的dir
    vec3 invDir; // 光线dir倒数
    vec3 pos;    // 命中位置
    vec3 normal; // 世界法线
};

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

    Sphere(const point3 &center, float radius) : center(center), radius(radius) {}
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
            return std::make_optional<HitInfos>({t,
                                                 ray.getOrigin(),
                                                 dir,
                                                 vec3(1.f / dir.x, 1.f / dir.y, 1.f / dir.z),
                                                 ray.at(t),
                                                 N});
        }
    }
    ~Sphere() {}
};