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
                std::make_shared<Lambertian>(color4(0.7f, 0.1f, 0.15f, 1.0f))));
        Objects.push_back(
            std::make_unique<Sphere>(
                point3(3.0f, 1.5f, 1.f),
                1.5f,
                std::make_shared<Lambertian>(color4(0.2f, 0.7f, 0.1f, 1.0f))));
        Objects.push_back(
            std::make_unique<Sphere>(
                point3(-6.0f, 2.f, 5.f),
                1.5f,
                std::make_shared<Metal>(color4(0.8f, 0.7f, 0.2f, 1.0f), 0.9f)));
        Objects.push_back(
            std::make_unique<Sphere>(
                point3(-2.0f, 4.f, 2.f),
                1.5f,
                std::make_shared<Metal>(color4(0.7f, 0.7f, 0.4f, 1.0f), 0.2f)));
        Objects.push_back(
            std::make_unique<Sphere>(
                point3(-2.0f, 10.f, 0.f),
                4.f,
                std::make_shared<LightEmit>(color4(5.0f)))); // 光源
        Objects.push_back(
            std::make_unique<Sphere>(
                point3(0.f, -6.3e2f, 0.f),
                6.3e2f,
                std::make_shared<Lambertian>(color4(0.7f, 0.7f, 0.7f, 1.0f)))); // 地球
    }
};