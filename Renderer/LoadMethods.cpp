#include "LoadMethods.hpp"
#include <format>
#include <iostream>
#include <shared_mutex>
#include <stdexcept>
#include <string>
#include <memory>
#include <algorithm>

// LoadSdSceneGPU
LoadSdSceneGPU::LoadSdSceneGPU(SdSceneGPUContext &context)
    : DIContext(context),
      flatNodeStorage(std::make_unique<sd::FlatNodeStorage>()),
      flatTriangleStorage(std::make_unique<sd::FlatTriangleStorage>()) {}

void LoadSdSceneGPU::load() {
    uint32_t rootIndex = 0;
    {
        std::shared_lock<std::shared_mutex> sdSceneLock(Storage::SdSceneMutex); // read lock
        sd::ConvertToFlatStorage(*Storage::SdScene.pDataStorage, *flatNodeStorage.get(), *flatTriangleStorage.get());
        rootIndex = Storage::SdScene.pDataStorage->rootIndex;
    }
    {
        std::unique_lock<std::shared_mutex> sceneRenderingLock(*DIContext.sceneBundleRenderingMutex); // write lock
        auto &[NodeStorageTexRendering, TriangleStorageTexRendering, SceneRootIndexRendering] = *DIContext.sceneBundleRendering;
        if (NodeStorageTexRendering.ID == 0 || TriangleStorageTexRendering.ID == 0) {
            throw std::runtime_error("Error: SceneBundleRendering Textures not initialized!");
        }
        resizeTextureStroage(TriangleStorageTexRendering, *flatTriangleStorage.get(), "Triangle");
        resizeTextureStroage(NodeStorageTexRendering, *flatNodeStorage.get(), "Node");
        TriangleStorageTexRendering.setData(flatTriangleStorage->triangles.data());
        NodeStorageTexRendering.setData(flatNodeStorage->nodes.data());
        SceneRootIndexRendering = rootIndex;
    }
}

template <typename TextureStorage, typename FlatStorage>
void LoadSdSceneGPU::resizeTextureStroage(TextureStorage &texLoading, const FlatStorage &flatStorage, const std::string &storageName) {
    size_t requiredSize = flatStorage.getSizeInFloats();
    size_t currentCapacity = (size_t)texLoading.Width * (size_t)texLoading.Height;
    size_t limit = (size_t)GetTextureSizeLimit() * (size_t)GetTextureSizeLimit();
    if (currentCapacity < requiredSize) {
        if (requiredSize > limit) {
            throw std::runtime_error(std::format("{} Storage Texture width beyond Limit: {} > {}", storageName, requiredSize, limit));
        }
        int newWidth = GetTextureSizeLimit();
        int newHeight = static_cast<int>((requiredSize + newWidth - 1) / newWidth);
        newHeight = std::max(1, newHeight);
        texLoading.resize(newWidth, newHeight);
    }
}

// LoadSdSceneCPU
LoadSdSceneCPU::LoadSdSceneCPU(SdSceneCPUContext &context) : DIContext(context) {}
void LoadSdSceneCPU::load() {
    {
        std::unique_lock<std::shared_mutex> sceneWriteLock(*DIContext.sceneRenderingMutex); // write lock
        std::shared_lock<std::shared_mutex> sceneReadLock(Storage::SdSceneMutex);           // read lock
        DIContext.sceneRendering = std::make_unique<sd::Scene>(Storage::SdScene);           // 拷贝上传数据
    }
}

// LoadSceneCPU
LoadSceneCPU::LoadSceneCPU(SceneCPUContext &context) : DIContext(context) {}
void LoadSceneCPU::load() {
    try {
        std::unique_lock<std::shared_mutex> sceneWriteLock(*DIContext.sceneRenderingMutex); // write lock
        std::shared_lock<std::shared_mutex> sceneReadLock(Storage::OldSceneMutex);          // read lock
        DIContext.sceneRendering = std::make_unique<Scene>(Storage::OldScene);              // 拷贝上传数据
    } catch (std::exception &e) {
        std::cerr << "Error loading Old Scene: " << e.what() << std::endl;
    }
}
