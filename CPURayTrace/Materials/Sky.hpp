#pragma once
#include "Materials.hpp"

class Sky : public Material
{
public:
    ~Sky() {}

    color4 getIrradiance(const HitInfos &hitInfos, int traceDepth) const override
    {
        vec3 unit_direction = normalize(hitInfos.dir);
        auto a = 0.5f * (unit_direction.y + 1.0f);
        return color4((1.0f - a) * vec3(1.0f, 1.0f, 1.0f) + a * vec3(0.5f, 0.7f, 1.0f), 1.0f)/1.f;
    }
};


