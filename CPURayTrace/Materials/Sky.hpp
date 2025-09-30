#pragma once
#include "Materials.hpp"

class Sky : public Material
{
public:
    ~Sky() {}
    Sky() = default;
    Sky(const Sky& other) = default;
    color4 getIrradiance(const HitInfos &hitInfos, int traceDepth, const Scene &scene) const override
    {
        vec3 unit_direction = normalize(hitInfos.dir);
        auto a = 0.5f * (unit_direction.y + 1.0f);
        return color4((1.0f - a) * vec3(1.0f, 1.0f, 1.0f) + a * vec3(0.5f, 0.7f, 1.0f), 1.0f) / 1.f;
    }
    std::unique_ptr<Material> clone() const override
    {
        return std::move(std::make_unique<Sky>(*this));
    }
};
