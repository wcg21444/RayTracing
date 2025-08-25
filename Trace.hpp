#pragma once
#include <glm/glm.hpp>

#include "Utils.hpp"
#include "Ray.hpp"

inline float hitSphere(const point3 &center, float radius, const Ray &r)
{
    vec3 oc = center - r.getOrigin();
    float a = dot(r.getDirection(), r.getDirection());
    float b = -2.0f * dot(r.getDirection(), oc);
    float c = dot(oc, oc) - radius * radius;
    float discriminant = b * b - 4 * a * c;
    if (discriminant < 0)
    {
        return -1.0f;
    }
    else
    {
        return (-b - std::sqrt(discriminant)) / (2.0 * a);
    }
}

inline color4 castRay(Ray &ray)
{
    auto t = hitSphere(point3(0.0f, 0.0f, 2.f), 1.5f, ray);
    if (t > 0.0f)
    {
        vec3 N = glm::normalize(ray.at(t) - vec3(0.0f, 0.f, -1.f));
        return 0.5f * color4(N.x + 1, N.y + 1, N.z + 1, 1.0f);
    }

    float a = 0.5f * (ray.getDirection().y + 1.0f);

    return (1.0f - a) * vec4(1.0f) + a * vec4(0.5f, 0.7f, 1.0f, 1.0f);
}