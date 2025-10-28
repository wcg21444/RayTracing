#pragma once
#include "RenderInterfaces.hpp"
#include "RenderContexts.hpp"
class LoadSdSceneGPU : public ILoadMethod // 产生对应Context的引用依赖
{
    SdSceneGPUContext &DIContext; // DI 必须

    Storage::SceneBundle sceneBundleLoading;
    std::unique_ptr<sd::FlatNodeStorage> flatNodeStorage = nullptr;
    std::unique_ptr<sd::FlatTriangleStorage> flatTriangleStorage = nullptr;

public:
    LoadSdSceneGPU(SdSceneGPUContext &context) : DIContext(context)
    {
        Storage::InitializeSceneBundle(sceneBundleLoading);
        flatNodeStorage = std::make_unique<sd::FlatNodeStorage>();
        flatTriangleStorage = std::make_unique<sd::FlatTriangleStorage>();
    }

    void load() override
    {
        auto &[nodeStorageTexLoading, triangleStorageTexLoading, rootIndex] = sceneBundleLoading;
        {
            std::shared_lock<std::shared_mutex> sdSceneLock(Storage::SdSceneMutex); // read lock
            sd::ConvertToFlatStorage(*Storage::SdScene.pDataStorage, *flatNodeStorage.get(), *flatTriangleStorage.get());
            rootIndex = Storage::SdScene.pDataStorage->rootIndex;
        }

        resizeTextureStroage(nodeStorageTexLoading, *flatNodeStorage.get(), "Node");
        resizeTextureStroage(triangleStorageTexLoading, *flatTriangleStorage.get(), "Triangle");

        nodeStorageTexLoading.setData(flatNodeStorage->nodes.data());
        triangleStorageTexLoading.setData(flatTriangleStorage->triangles.data());

        {
            std::unique_lock<std::shared_mutex> sceneRenderingLock(*DIContext.sceneBundleRenderingMutex); // write lock
            auto &[NodeStorageTexRendering, TriangleStorageTexRendering, SceneRootIndexRendering] = *DIContext.sceneBundleRendering;

            TriangleStorageTexRendering.resize(triangleStorageTexLoading.Width, triangleStorageTexLoading.Height);
            NodeStorageTexRendering.resize(nodeStorageTexLoading.Width, nodeStorageTexLoading.Height);

            std::swap(this->sceneBundleLoading, *DIContext.sceneBundleRendering);
        }
    }

private:
    template <typename TextureStorage, typename FlatStorage>
    void resizeTextureStroage(TextureStorage &texLoading, const FlatStorage &flatStorage, const std::string &storageName)
    {
        // 获取所需最小的浮点数总数
        size_t requiredSize = flatStorage.getSizeInFloats();
        size_t currentCapacity = (size_t)texLoading.Width * (size_t)texLoading.Height;
        size_t limit = (size_t)GetTextureSizeLimit() * (size_t)GetTextureSizeLimit();

        // 检查是否需要调整大小
        if (currentCapacity < requiredSize)
        {
            // 检查是否超出纹理大小限制
            if (requiredSize > limit)
            {
                throw std::runtime_error(std::format("{} Storage Texture width beyond Limit: {} > {}", storageName, requiredSize, limit));
            }

            // 重新计算新的高度，确保上取整： (a + b - 1) / b
            int newWidth = GetTextureSizeLimit();
            // C++中整数除法默认截断，使用 (requiredSize + newWidth - 1) / newWidth 实现上取整
            int newHeight = static_cast<int>((requiredSize + newWidth - 1) / newWidth);

            // 确保新高度至少为 1
            newHeight = std::max(1, newHeight);

            texLoading.resize(newWidth, newHeight);

            std::cout << std::format("Resized {} Storage Texture to: {}x{} (Capacity: {})",
                                     storageName, texLoading.Width, texLoading.Height, (size_t)texLoading.Width * (size_t)texLoading.Height)
                      << std::endl;
        }
    }
};
