#pragma once

#include "Objects.hpp"

// 全局静态场景类
class Scene
{
public:
    inline static std::vector<std::unique_ptr<Hittable>> Objects;

    inline static void Initialze() // 布置场景
    {
        Objects.push_back(
            std::make_unique<Sphere>(
                point3(2.0f, 0.0f, 2.f),
                1.5f,
                Material{Material::Type::Diffuse, color4(0.7f, 0.2f, 0.3f, 1.0f)}));
        Objects.push_back(
            std::make_unique<Sphere>(
                point3(3.0f, 1.5f, 1.f),
                1.5f,
                Material{Material::Type::Diffuse, color4(0.9f, 0.9f, 0.9f, 1.0f)}));
        Objects.push_back(
            std::make_unique<Sphere>(
                point3(-6.0f, 2.f, 5.f),
                1.5f,
                Material{Material::Type::Diffuse, color4(0.2f, 0.7f, 0.3f, 1.0f)}));
        Objects.push_back(
            std::make_unique<Sphere>(
                point3(0.f, -6.3e2f, 0.f),
                6.3e2f,
                Material{Material::Type::Diffuse, color4(0.8f, 0.9f, 0.7f, 1.0f)})); // 地球
    }
};