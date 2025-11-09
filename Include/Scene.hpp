#pragma once
#include <glm/glm.hpp>
#include <vector>
#include <memory>
#include <algorithm>
#include <functional>
#include <optional>

#include "Objects.hpp"
#include "BVH.hpp"
#include "Materials.hpp"
#include "SimplifiedData.hpp"

class SceneConfig;

// 全局静态场景类
class ImplicitScene
{
public:
    std::vector<std::shared_ptr<Hittable>> objects;
    BVH BVHTree;

    ImplicitScene();
    // 拷贝构造 深拷贝
    ImplicitScene(const ImplicitScene &other);
    // 拷贝赋值 深拷贝
    ImplicitScene &operator=(const ImplicitScene &other);

    void initialize(const std::optional<SceneConfig> &config); // 布置场景

    void update();

    void addObject(std::shared_ptr<Hittable> object);

    HitInfos intersectClosestBVH(const Ray &ray) const;
    HitInfos intersectClosest(const Ray &ray) const;
};

namespace SimplifiedData
{
    class Scene
    {
    public:
        std::unique_ptr<sd::DataStorage> pDataStorage = nullptr;
        std::vector<uint32_t> sceneIndices;

        Scene();

        // 拷贝 深拷贝
        Scene(const Scene &other);
        Scene &operator=(const Scene &other);

        void initialize(const std::optional<SceneConfig> &config); // 布置场景 延迟初始化
    };
}