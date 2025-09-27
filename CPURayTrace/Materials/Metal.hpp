#pragma once
#include "Materials.hpp"
#include "Trace.hpp"
#include "Random.hpp"
class Metal : public Material
{

public:
    color4 albedo;
    float gross;

    Metal(color4 albedo, float gross) : albedo(albedo), gross(gross) {}
    ~Metal() {}

    color4 getIrradiance(const HitInfos &hitInfos, int traceDepth) const override
    {
        auto &normal = hitInfos.normal;
        auto &pos = hitInfos.pos;
        auto &dir = hitInfos.dir;

        vec3 bias = normal * 1e-4f; // 防止自相交
        vec3 rndDir = Random::GenerateCosineSemiSphereVector(normal);
        vec3 reflectDir = dir - normal * dot(dir, normal) * 2.f;
        vec3 rayDir = normalize(rndDir * gross + reflectDir * (1 - gross));
        auto bounceRay = Ray(
            pos + bias,
            rayDir);
        color4 irradiance = albedo * Trace::CastRay(bounceRay, traceDepth + 1);

        return irradiance;
    }
};