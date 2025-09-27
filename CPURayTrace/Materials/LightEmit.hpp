#pragma once
#include "Materials.hpp"
class LightEmit : public Material
{
public:
    color4 intensity;

    LightEmit(color4 intensity) : intensity(intensity) {}
    ~LightEmit() {}

    color4 getIrradiance(const HitInfos &hitInfos, int traceDepth) const override
    {
        return intensity;
    }
};