#pragma once
#include "Utils.hpp"
class Ray
{
public:
    Ray() {}

    Ray(const point3 &origin, const vec3 &direction) : orig(origin), dir(direction), invDir(1.0f / direction) {}

    const point3 &getOrigin() const { return orig; }
    const vec3 &getDirection() const { return dir; }
    const vec3 &getInvDirection() const { return invDir; }

    point3 at(float t) const
    {
        return orig + t * dir;
    }

private:
    point3 orig;
    vec3 dir;
    vec3 invDir;
};
