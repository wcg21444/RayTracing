#pragma once
#include "Materials.hpp"
class LightEmit : public Material
{
public:
    color4 intensity;

    LightEmit(color4 intensity) : intensity(intensity) {}
    ~LightEmit() {}

    color4 getIrradiance(const HitInfos &hitInfos, int traceDepth, const Scene &scene) const override
    {
        return intensity;
    }

    std::unique_ptr<Material> clone() const override
    {
        return std::move(std::make_unique<LightEmit>(*this));
    }
};