#pragma once
#include <glm/glm.hpp>
#include <vector>
#include <memory>
#include <algorithm>
#include <functional>
#include "Objects.hpp"
#include "BVH.hpp"
#include "Materials.hpp"
#include "SimplifiedData.hpp"

// 全局静态场景类
class ImplicitScene
{
public:
    std::vector<std::shared_ptr<Hittable>> objects;
    BVH BVHTree;

    ImplicitScene();
    // 拷贝构造
    ImplicitScene(const ImplicitScene &other);
    // 拷贝赋值
    ImplicitScene &operator=(const ImplicitScene &other);

    void initialize(); // 布置场景

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

        //拷贝
        Scene(const Scene &other) ;
        Scene &operator=(const Scene &other);
        

        void initialize(); // 布置场景 延迟初始化
    };
}