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
            std::make_unique<Sphere>(point3(2.0f, 0.0f, 2.f), 1.5f));
        Objects.push_back(
            std::make_unique<Sphere>(point3(1.0f, 8.0f, 3.f), 1.5f));
        Objects.push_back(
            std::make_unique<Sphere>(point3(-12.0f, 3.0f, 5.f), 1.5f));
        Objects.push_back(
            std::make_unique<Sphere>(point3(0.f, -1e3, 0.f), 1e3f)); // 地球
    }
};