#pragma once
#include <memory>
#include <mutex>
#include <shared_mutex>
#include "Storage.hpp"
#include "Camera.hpp"

/// Context 使用依赖注入, 数据应当是指针或者引用. 需要注入的依赖通过构造函数传入

// Context在使用期间不应该被外部修改?
struct SdSceneGPUContext : public IRenderContext // 将被移动注入
{
    std::unique_ptr<Storage::SceneBundle> sceneBundleRendering;
    std::unique_ptr<std::shared_mutex> sceneBundleRenderingMutex; // 必须是指针，不能移动锁

    std::unique_ptr<std::shared_mutex> snapshotMutex;
    std::unique_ptr<Camera> camSnapshot;         // 用于保存渲染时的Camera状态
    std::unique_ptr<TextureCube> skyboxSnapShot; // 用于保存渲染时的Skybox纹理

    // 引用外部资源
    TextureCube &skyboxCubemapRef;
    Camera &cameraRef;

    // 通过构造函数区别注入的依赖和内部创建的依赖
    SdSceneGPUContext(
        TextureCube &_skyboxCubemap,
        Camera &_cam)
        : sceneBundleRendering(std::make_unique<Storage::SceneBundle>()),
          sceneBundleRenderingMutex(std::make_unique<std::shared_mutex>()),
          snapshotMutex(std::make_unique<std::shared_mutex>()),
          camSnapshot(std::make_unique<Camera>(_cam)),
          skyboxSnapShot(std::make_unique<TextureCube>()),
          skyboxCubemapRef(_skyboxCubemap),
          cameraRef(_cam)
    {
        Storage::InitializeSceneBundle(*sceneBundleRendering);
        *skyboxSnapShot = (skyboxCubemapRef.clone());
    }

    void snapshot() override
    {
        std::unique_lock<std::shared_mutex> contextLock(*snapshotMutex); // write lock
        *(camSnapshot) = cameraRef;
        skyboxCubemapRef.copyDataTo(*skyboxSnapShot);
    }
};

struct SdSceneCPUContext : public IRenderContext // 将被移动注入
{
    std::unique_ptr<sd::Scene> sceneRendering;              // CPU Context Loader 上传目标, Trace 读取目标
    std::unique_ptr<std::shared_mutex> sceneRenderingMutex; // 必须是指针，不能移动锁

    Camera &cameraRef;

    // 通过构造函数区别注入的依赖和内部创建的依赖
    SdSceneCPUContext(
        Camera &_cam)
        : sceneRendering(std::make_unique<sd::Scene>()),
          sceneRenderingMutex(std::make_unique<std::shared_mutex>()),
          cameraRef(_cam)
    {
    }
    void snapshot() override
    {
        // CPU Context 无需快照
    }
};

struct SceneCPUContext : public IRenderContext // 将被移动注入
{
    std::unique_ptr<Scene> sceneRendering;                  // CPU Context Loader 上传目标, Trace 读取目标
    std::unique_ptr<std::shared_mutex> sceneRenderingMutex; // 必须是指针，不能移动锁

    Camera &cameraRef;

    // 通过构造函数区别注入的依赖和内部创建的依赖
    SceneCPUContext(
        Camera &_cam)
        : sceneRendering(std::make_unique<Scene>()),
          sceneRenderingMutex(std::make_unique<std::shared_mutex>()),
          cameraRef(_cam)
    {
    }
    void snapshot() override
    {
        // CPU Context 无需快照
    }
};
