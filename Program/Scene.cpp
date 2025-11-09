#pragma once
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/euler_angles.hpp>
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
#include "Config.hpp"

// Scene copy constructor
ImplicitScene::ImplicitScene(const ImplicitScene &other)
    : objects(other.objects), BVHTree(other.BVHTree)
{
    if (other.BVHTree.root)
    {
        std::unordered_map<std::shared_ptr<Hittable>, int> indexHash;
        for (int i = 0; i < objects.size(); ++i)
        {
            if (objects[i] != nullptr)
            {
                indexHash[objects[i]] = i;
            }
        }
        auto remapping = [this, &indexHash](auto &&traverseSelf, BVHNode *node) -> void
        {
            if (!node)
            {
                return;
            }
            if (node->object) // 叶子节点
            {
                node->object = objects[indexHash[node->object]];
            }
            traverseSelf(traverseSelf, node->left);
            traverseSelf(traverseSelf, node->right);
        };
        remapping(remapping, BVHTree.root);
        // BVHTree.build(objects);
    }
}

// Scene default constructor
ImplicitScene::ImplicitScene()
{
}

// Scene::update
void ImplicitScene::update()
{
    BVHTree.build(objects);
}

// Scene::addObject
void ImplicitScene::addObject(std::shared_ptr<Hittable> object)
{
    objects.push_back(object);
}

// Scene::intersectClosestBVH
HitInfos ImplicitScene::intersectClosestBVH(const Ray &ray) const
{
    return BVHTree.intersect(ray);
}

// Scene::intersectClosest
HitInfos ImplicitScene::intersectClosest(const Ray &ray) const
{
    HitInfos closestHit;
    for (auto &&object : objects)
    {
        auto hitInfos = object->intersect(ray);
        if (hitInfos && hitInfos->t < closestHit.t)
        {
            closestHit = *hitInfos;
        }
    }
    return closestHit;
}

// Scene copy assignment operator
ImplicitScene &ImplicitScene::operator=(const ImplicitScene &other)
{
    objects = other.objects;
    if (other.BVHTree.root)
    {
        BVHTree.build(objects);
    }
    return *this;
}

void ImplicitScene::initialize(const std::optional<SceneConfig> &config)
{
    objects.clear();
    if(!config)
        return;
    try
    {
        for (auto &object : config->jsonConfig["objects"])
        {
            if (object["type"] == "sphere")
            {
                glm::vec3 position = {object["position"][0], object["position"][1], object["position"][2]};
                float radius = object.contains("scale") ? static_cast<float>(object["scale"][0]) : 1.0f;
                glm::vec3 scale = object.contains("scale") ? glm::vec3(object["scale"][0], object["scale"][1], object["scale"][2]) : glm::vec3(1.0f);
                glm::vec3 rotationEuler = object.contains("rotation_euler") ? glm::vec3(object["rotation_euler"][0], object["rotation_euler"][1], object["rotation_euler"][2]) : glm::vec3(0.0f);
                // 材质解析
                std::shared_ptr<Material> mat;
                const auto &matJson = object["material"];
                std::string matType = matJson["type"];
                glm::vec4 color = matJson.contains("color") ? glm::vec4(matJson["color"][0], matJson["color"][1], matJson["color"][2], matJson["color"][3]) : glm::vec4(1.0f);
                if (matType == "Lambertian")
                    mat = std::make_shared<Lambertian>(color);
                else if (matType == "Metal")
                    mat = std::make_shared<Metal>(color, matJson.value("fuzz", 0.0f));
                else if (matType == "LightEmit")
                    mat = std::make_shared<LightEmit>(color);
                else
                    mat = std::make_shared<Lambertian>(color);
                objects.push_back(std::make_shared<Sphere>(position, radius, *mat));
            }
            else if (object["type"] == "mesh")
            {
                // Mesh 反序列化
                std::vector<Vertex> vertices;
                for (const auto &v : object["vertices"])
                {
                    Vertex vert;
                    vert.position = glm::vec3(v["position"][0], v["position"][1], v["position"][2]);
                    vert.normal = glm::vec3(v["normal"][0], v["normal"][1], v["normal"][2]);
                    vert.texCoord = glm::vec2(v["uv"][0], v["uv"][1]);
                    vertices.push_back(vert);
                }
                std::vector<unsigned int> indices;
                for (const auto &idx : object["indices"])
                    indices.push_back(idx);
                // 材质
                const auto &matJson = object["material"];
                std::string matType = matJson["type"];
                glm::vec4 color = matJson.contains("color") ? glm::vec4(matJson["color"][0], matJson["color"][1], matJson["color"][2], matJson["color"][3]) : glm::vec4(1.0f);
                std::shared_ptr<Material> mat;
                if (matType == "Lambertian")
                    mat = std::make_shared<Lambertian>(color);
                else if (matType == "Metal")
                    mat = std::make_shared<Metal>(color, matJson.value("fuzz", 0.0f));
                else if (matType == "LightEmit")
                    mat = std::make_shared<LightEmit>(color);
                else
                    mat = std::make_shared<Lambertian>(color);
                // 变换
                glm::vec3 position = {object["position"][0], object["position"][1], object["position"][2]};
                glm::vec3 rotationEuler = object.contains("rotation_euler") ? glm::vec3(object["rotation_euler"][0], object["rotation_euler"][1], object["rotation_euler"][2]) : glm::vec3(0.0f);
                glm::vec3 scale = object.contains("scale") ? glm::vec3(object["scale"][0], object["scale"][1], object["scale"][2]) : glm::vec3(1.0f);
                glm::mat4 transform = glm::mat4(1.0f);
                transform = glm::translate(transform, position);
                transform = glm::eulerAngleXYZ(rotationEuler.x, rotationEuler.y, rotationEuler.z) * transform;
                transform = glm::scale(transform, scale);
                objects.push_back(std::make_shared<Mesh>(vertices, indices, *mat, transform));
            }
        }
        update();
    }
    catch (std::exception &e)
    {
        std::cout << e.what() << std::endl;
        std::abort();
    }
}

namespace SimplifiedData
{
    Scene::Scene()
    {
        pDataStorage = std::make_unique<sd::DataStorage>();
    }

    Scene::Scene(const Scene &other)
    {
        pDataStorage = std::make_unique<sd::DataStorage>(*other.pDataStorage.get());
        pDataStorage->rootIndex = other.pDataStorage->rootIndex;
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

    void Scene::initialize(const std::optional<SceneConfig> &config)
    {
        ModelLoader::SetDataStorage(pDataStorage.get());

        uint32_t root;

        try
        {
            if (!config)
            {
                return;
            }
            for (auto &object : config->jsonConfig["objects"])
            {
                if (object["type"] != "mesh")
                    continue;

                glm::vec3 position = {object["position"][0], object["position"][1], object["position"][2]};
                glm::vec3 rotationEuler = {object["rotation_euler"][0], object["rotation_euler"][1], object["rotation_euler"][2]};
                glm::vec3 scale = {object["scale"][0], object["scale"][1], object["scale"][2]};
                glm::vec4 color = {object["color"][0], object["color"][1], object["color"][2], object["color"][3]};
                glm::mat4 transform = glm::mat4(1.0f);
                transform = glm::translate(transform, position);
                transform = glm::eulerAngleXYZ(rotationEuler.y, rotationEuler.x, rotationEuler.z) * transform;
                transform = glm::scale(transform, scale);
                root = sd::ModelLoader::LoadModelFileSync(object["path"].get<std::string>(), transform, color);
                sceneIndices.push_back(root);
            }
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