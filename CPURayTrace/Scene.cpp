#pragma once
#include <glm/glm.hpp>
#include <vector>
#include <memory>
#include <algorithm>
#include <functional>
#include "Objects.hpp"
#include "Mesh.hpp"
#include "BVH.hpp"
#include "Scene.hpp"
#include "Materials/Metal.hpp"
#include "Materials/Lambertian.hpp"
#include "Materials/LightEmit.hpp"
#include "Materials/Sky.hpp"
#include "Random.hpp"
#include "ModelLoader.hpp"

void Scene::initialize()
{

    /*     objects.push_back(
            std::make_shared<Sphere>(
                point3(2.0f, 0.0f, 2.f),
                1.5f,
                std::make_shared<Lambertian>(color4(0.7f, 0.1f, 0.15f, 1.0f))));
        objects.push_back(
            std::make_shared<Sphere>(
                point3(3.0f, 1.5f, 1.f),
                1.5f,
                std::make_shared<Lambertian>(color4(0.2f, 0.7f, 0.1f, 1.0f))));
        objects.push_back(
            std::make_shared<Sphere>(
                point3(-6.0f, 2.f, 5.f),
                1.5f,
                std::make_shared<Metal>(color4(0.8f, 0.7f, 0.2f, 1.0f), 0.99f)));
        objects.push_back(
            std::make_shared<Sphere>(
                point3(-2.0f, 4.f, 2.f),
                1.5f,
                std::make_shared<Metal>(color4(0.7f, 0.7f, 0.4f, 1.0f), 0.5f)));
        objects.push_back(
            std::make_shared<Sphere>(
                point3(-2.0f, 14.f, 7.f),
                1.5f,
                std::make_shared<Metal>(color4(0.7f, 0.7f, 0.4f, 1.0f), 0.5f)));
        objects.push_back(
            std::make_shared<Sphere>(
                point3(-2.0f, 18.f, 9.f),
                1.5f,
                std::make_shared<Metal>(color4(0.7f, 0.7f, 0.4f, 1.0f), 0.5f)));
        objects.push_back(
            std::make_shared<Sphere>(
                point3(-2.0f, 15.f, 0.f),
                4.f,
                std::make_shared<LightEmit>(color4(50.0f)))); // 光源
        objects.push_back(
            std::make_shared<Sphere>(
                point3(0.f, -6.3e3f, 0.f),
                6.3e3f,
                std::make_shared<Lambertian>(color4(0.7f, 0.7f, 0.7f, 1.0f)))); // 地球
        for (size_t i = 0; i < 50; i++)
        {
            point3 center = point3(Random::RandomVector(10.f));
            center.y = glm::length(center) / 10.f -0.5f;
            objects.push_back(
                std::make_shared<Sphere>(
                    center,
                    0.4f,
                    std::make_shared<Metal>(color4((Random::RandomVector(0.7f) + 1.0f) / 2.f, 1.0f), 0.9f)));
        } */

    // for (size_t i = 0; i < 50; i++)
    // {
    //     point3 center = point3(Random::RandomVector(10.f));
    //     center.y = glm::length(center) / 10.f - 0.5f;
    //     objects.push_back(
    //         std::make_shared<Sphere>(
    //             center,
    //             0.4f,
    //             Metal(color4((Random::RandomVector(0.7f) + 1.0f) / 2.f, 1.0f), 0.9f)));
    // }

    // 搭建立方体（每个面4个顶点，共24个顶点，保证法线与UV正确）
    // objects.push_back(
    //     std::make_shared<Mesh>(
    //         std::vector<Vertex>{
    //             // Front (-Z)
    //             {{-1.0f, 0.0f, -1.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f}},
    //             {{1.0f, 0.0f, -1.0f}, {0.0f, 0.0f, -1.0f}, {1.0f, 0.0f}},
    //             {{1.0f, 2.0f, -1.0f}, {0.0f, 0.0f, -1.0f}, {1.0f, 1.0f}},
    //             {{-1.0f, 2.0f, -1.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f}},
    //             // Back (+Z)
    //             {{-1.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
    //             {{1.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
    //             {{1.0f, 2.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
    //             {{-1.0f, 2.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
    //             // Top (+Y)
    //             {{-1.0f, 2.0f, -1.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
    //             {{1.0f, 2.0f, -1.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
    //             {{1.0f, 2.0f, 1.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}},
    //             {{-1.0f, 2.0f, 1.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}},
    //             // Bottom (-Y)
    //             {{-1.0f, 0.0f, 1.0f}, {0.0f, -1.0f, 0.0f}, {0.0f, 0.0f}},
    //             {{1.0f, 0.0f, 1.0f}, {0.0f, -1.0f, 0.0f}, {1.0f, 0.0f}},
    //             {{1.0f, 0.0f, -1.0f}, {0.0f, -1.0f, 0.0f}, {1.0f, 1.0f}},
    //             {{-1.0f, 0.0f, -1.0f}, {0.0f, -1.0f, 0.0f}, {0.0f, 1.0f}},
    //             // Left (-X)
    //             {{-1.0f, 0.0f, 1.0f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
    //             {{-1.0f, 0.0f, -1.0f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
    //             {{-1.0f, 2.0f, -1.0f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},
    //             {{-1.0f, 2.0f, 1.0f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},
    //             // Right (+X)
    //             {{1.0f, 0.0f, -1.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
    //             {{1.0f, 0.0f, 1.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
    //             {{1.0f, 2.0f, 1.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},
    //             {{1.0f, 2.0f, -1.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},
    //         },
    //         std::vector<unsigned int>{
    //             // Front
    //             0,
    //             1,
    //             2,
    //             2,
    //             3,
    //             0,
    //             // Back
    //             4,
    //             5,
    //             6,
    //             6,
    //             7,
    //             4,
    //             // Top
    //             8,
    //             9,
    //             10,
    //             10,
    //             11,
    //             8,
    //             // Bottom
    //             12,
    //             13,
    //             14,
    //             14,
    //             15,
    //             12,
    //             // Left
    //             16,
    //             17,
    //             18,
    //             18,
    //             19,
    //             16,
    //             // Right
    //             20,
    //             21,
    //             22,
    //             22,
    //             23,
    //             20,
    //         },
    //         LightEmit(color4(8.f, 6.f, 5.f, 1.f))));

    ModelLoader::LoadModelFileSync("Resources/TheStanfordDragon2426.obj", *this);
}

namespace SimplifiedData
{
    Scene::Scene()
    {
        pDataStorage = std::make_unique<sd::DataStorage>();

        initialize();
    }

    Scene::Scene(const Scene &other)
    {
        pDataStorage = std::make_unique<sd::DataStorage>(*other.pDataStorage.get());
    }

    Scene &Scene::operator=(const Scene &other)
    {
        if (this != &other)
        {
            pDataStorage = std::make_unique<sd::DataStorage>(*other.pDataStorage.get());
            sceneIndices = other.sceneIndices;
        }
        return *this;
    }

    void Scene::initialize()
    {
        ModelLoader::SetDataStorage(pDataStorage.get());

        uint32_t root;

        try
        {
            // root = sd::ModelLoader::LoadModelFileSync("Resources/Sphere.obj");
            // sceneIndices.push_back(root);
            root = sd::ModelLoader::LoadModelFileSync("Resources/MultiHighCube.obj");
            // root = sd::ModelLoader::LoadModelFileSync("Resources/TheStanfordDragon2426.obj");
            sceneIndices.push_back(root);

            auto sceneRoot = sd::BVH::BuildBVHFromNodes(pDataStorage->nodeStorage, sceneIndices.data(), 0, sceneIndices.size());
            pDataStorage->rootIndex = sceneRoot;
        }
        catch (std::exception &e)
        {
            std::cout << e.what() << std::endl;
            std::abort(); // 临时措施,直接终止
        }
    }
}