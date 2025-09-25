#pragma once

#include "Utils.hpp"
#include "Random.hpp"

#include "Trace.hpp"

class Material;
color4 castRay(const Ray &ray, int traceDepth = 0);

struct HitInfos
{
    float t;                             // 命中时光线的t
    vec3 origin;                         // 光线起点
    vec3 dir;                            // 命中光线的dir
    vec3 invDir;                         // 光线dir倒数
    vec3 pos;                            // 命中位置
    vec3 normal;                         // 归一化世界法线
    std::shared_ptr<Material> pMaterial; // 材质指针  单一材质命中
};

class Material
{
public:
    virtual ~Material() {}
    virtual color4 getIrradiance(const HitInfos &hitInfos, int traceDepth) const = 0;
};

class Lambertian : public Material
{
public:
    color4 albedo;

    Lambertian(color4 albedo) : albedo(albedo) {}
    ~Lambertian() {}

    color4 getIrradiance(const HitInfos &hitInfos, int traceDepth) const override
    {
        auto &normal = hitInfos.normal;
        auto &pos = hitInfos.pos;

        vec3 bias = normal * 1e-4f; // 防止自相交
        vec3 rndDir = Random::GenerateCosineSemiSphereVector(normal);
        auto bounceRay = Ray(
            pos + bias,
            rndDir);
        color4 irradiance = albedo * castRay(bounceRay, traceDepth + 1);

        return irradiance;
    }
};

class Sky : public Material
{
public:
    ~Sky() {}

    color4 getIrradiance(const HitInfos &hitInfos, int traceDepth) const override
    {
        vec3 unit_direction = normalize(hitInfos.dir);
        auto a = 0.5f * (unit_direction.y + 1.0f);
        return color4((1.0f - a) * vec3(1.0f, 1.0f, 1.0f) + a * vec3(0.5f, 0.7f, 1.0f), 1.0f);
    }
};

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
        color4 irradiance = albedo * castRay(bounceRay, traceDepth + 1);

        return irradiance;
    }
};

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