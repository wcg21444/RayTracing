#pragma once
#include <memory>
#include <mutex>
#include <shared_mutex>
#include "Storage.hpp"
#include "Camera.hpp"

// Context在使用期间不应该被外部修改?
struct SdSceneGPUContext // 将被移动注入
{
    std::unique_ptr<Storage::SceneBundle> sceneBundleRendering;
    std::unique_ptr<std::shared_mutex> sceneBundleRenderingMutex; // 必须是指针，不能移动锁
    TextureID& skyboxTextureID;
    Camera &cam;

    // 通过构造函数区别注入的依赖和内部创建的依赖
    SdSceneGPUContext(
        TextureID& _skyboxTextureId,
        Camera &_cam)
        : sceneBundleRendering(std::make_unique<Storage::SceneBundle>()),
          sceneBundleRenderingMutex(std::make_unique<std::shared_mutex>()),
          skyboxTextureID(_skyboxTextureId),
          cam(_cam)
    {
        Storage::InitializeSceneBundle(*sceneBundleRendering);
    }
};