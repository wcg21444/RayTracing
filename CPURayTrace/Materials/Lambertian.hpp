#pragma once
#include "Materials.hpp"
#include "Trace.hpp"
#include "Random.hpp"
#include "Scene.hpp"
class Lambertian : public Material
{
public:
    color4 albedo;

    Lambertian(color4 albedo) : albedo(albedo) {}
    ~Lambertian() {}

    Lambertian(const Lambertian &other) = default;

    color4 getIrradiance(const HitInfos &hitInfos, int traceDepth, const Scene &scene) const override
    {
        auto &normal = hitInfos.normal;
        auto &pos = hitInfos.pos;

        vec3 bias = normal * 1e-4f; // 防止自相交
        vec3 rndDir = Random::GenerateCosineSemiSphereVector(normal);
        auto bounceRay = Ray(
            pos + bias,
            rndDir);
        color4 irradiance = albedo * Trace::CastRay(bounceRay, traceDepth + 1, scene);

        return irradiance;
    }
    std::unique_ptr<Material> clone() const override
    {
        return std::move(std::make_unique<Lambertian>(*this));
    }
};
