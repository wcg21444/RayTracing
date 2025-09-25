#pragma once
#include <glm/glm.hpp>
struct BoundingBox
{   
    glm::vec3 pMin;
    glm::vec3 pMax;

    bool intersect(const Ray &ray, float tMin, float tMax) const
    {
        for (int a = 0; a < 3; a++)
        {
            float invD = ray.getInvDirection()[a];
            float t0 = (pMin[a] - ray.getOrigin()[a]) * invD;
            float t1 = (pMax[a] - ray.getOrigin()[a]) * invD;
            if (invD < 0.0f)
                std::swap(t0, t1);
            tMin = t0 > tMin ? t0 : tMin;
            tMax = t1 < tMax ? t1 : tMax;
            if (tMax <= tMin)
                return false;
        }
        return true;
    }
};