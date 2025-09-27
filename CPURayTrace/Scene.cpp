#pragma once
#include <glm/glm.hpp>
#include <vector>
#include <memory>
#include <algorithm>
#include <functional>
#include "Objects.hpp"
#include "BVH.hpp"
#include "Scene.hpp"
#include "Materials/Metal.hpp"
#include "Materials/Lambertian.hpp"
#include "Materials/LightEmit.hpp"
#include "Materials/Sky.hpp"
#include "Random.hpp"
void Scene::initialize()
{

    Objects.push_back(
        std::make_shared<Sphere>(
            point3(2.0f, 0.0f, 2.f),
            1.5f,
            std::make_shared<Lambertian>(color4(0.7f, 0.1f, 0.15f, 1.0f))));
    Objects.push_back(
        std::make_shared<Sphere>(
            point3(3.0f, 1.5f, 1.f),
            1.5f,
            std::make_shared<Lambertian>(color4(0.2f, 0.7f, 0.1f, 1.0f))));
    Objects.push_back(
        std::make_shared<Sphere>(
            point3(-6.0f, 2.f, 5.f),
            1.5f,
            std::make_shared<Metal>(color4(0.8f, 0.7f, 0.2f, 1.0f), 0.99f)));
    Objects.push_back(
        std::make_shared<Sphere>(
            point3(-2.0f, 4.f, 2.f),
            1.5f,
            std::make_shared<Metal>(color4(0.7f, 0.7f, 0.4f, 1.0f), 0.5f)));
    Objects.push_back(
        std::make_shared<Sphere>(
            point3(-2.0f, 14.f, 7.f),
            1.5f,
            std::make_shared<Metal>(color4(0.7f, 0.7f, 0.4f, 1.0f), 0.5f)));
    Objects.push_back(
        std::make_shared<Sphere>(
            point3(-2.0f, 18.f, 9.f),
            1.5f,
            std::make_shared<Metal>(color4(0.7f, 0.7f, 0.4f, 1.0f), 0.5f)));
    Objects.push_back(
        std::make_shared<Sphere>(
            point3(-2.0f, 15.f, 0.f),
            4.f,
            std::make_shared<LightEmit>(color4(50.0f)))); // 光源
    Objects.push_back(
        std::make_shared<Sphere>(
            point3(0.f, -6.3e3f, 0.f),
            6.3e3f,
            std::make_shared<Lambertian>(color4(0.7f, 0.7f, 0.7f, 1.0f)))); // 地球
    for (size_t i = 0; i < 200; i++)
    {
        point3 center = point3(Random::RandomVector(40.f));
        center.y = glm::length(center) / 40.f + 1.f;
        Objects.push_back(
            std::make_shared<Sphere>(
                center,
                1.f,
                std::make_shared<Metal>(color4((Random::RandomVector(1.0f) + 1.0f) / 2.f, 1.0f), 0.4f)));
    }
}
