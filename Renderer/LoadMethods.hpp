#pragma once
#include "RenderInterfaces.hpp"
#include "RenderContexts.hpp"

class LoadSdSceneGPU : public ILoadMethod // 产生对应Context的引用依赖
{
    SdSceneGPUContext &DIContext; // DI 必须
    std::unique_ptr<sd::FlatNodeStorage> flatNodeStorage;
    std::unique_ptr<sd::FlatTriangleStorage> flatTriangleStorage;
public:
    LoadSdSceneGPU(SdSceneGPUContext &context);
    void load() override;
private:
    template <typename TextureStorage, typename FlatStorage>
    void resizeTextureStroage(TextureStorage &texLoading, const FlatStorage &flatStorage, const std::string &storageName);
};

class LoadSdSceneCPU : public ILoadMethod
{
    SdSceneCPUContext &DIContext;
public:
    LoadSdSceneCPU(SdSceneCPUContext &context);
    void load() override;
};

class LoadImSceneCPU : public ILoadMethod
{
    ImSceneCPUContext &DIContext;
public:
    LoadImSceneCPU(ImSceneCPUContext &context);
    void load() override;
};