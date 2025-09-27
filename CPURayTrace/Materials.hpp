#pragma once

class Material;

struct HitInfos
{
    float t = std::numeric_limits<float>::infinity(); // 命中时光线的t
    vec3 origin;                                      // 光线起点
    vec3 dir;                                         // 命中光线的dir
    vec3 invDir;                                      // 光线dir倒数
    vec3 pos;                                         // 命中位置
    vec3 normal;                                      // 归一化世界法线
    std::shared_ptr<Material> pMaterial;              // 材质指针  单一材质命中
};

class Material
{
public:
    virtual ~Material() {}
    virtual color4 getIrradiance(const HitInfos &hitInfos, int traceDepth) const = 0;
};
