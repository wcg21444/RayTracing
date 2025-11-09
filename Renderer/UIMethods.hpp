#pragma once
#include "RenderInterfaces.hpp"
#include "RenderContexts.hpp"
#include "BVHUI.hpp"

class SdSceneGPUUI : public IUIMethod
{
    SdSceneGPUContext &DIContext; // DI 必须
public:
    SdSceneGPUUI(SdSceneGPUContext &context) : DIContext(context) {}

    void renderUI() override
    {
        ImGui::Begin("BVH Debug");
        {
            if (ImGui::Button("Print BVH Tree"))
            {
                Output2DVisualization(*Storage::SdSceneInstance.pDataStorage, "BVHOutput.txt");
            }
        }
        ImGui::End();
        InteractableVisualization(*Storage::SdSceneInstance.pDataStorage); // 跟渲染的具体场景类型有关,应该交由pipeline提供的方法处理
        BVHDebugSettings::RenderVisualization(*Storage::SdSceneInstance.pDataStorage);
        BVHDebugSettings::RenderUI();
        RayVisualizer::Render(*Storage::SdSceneInstance.pDataStorage);
    }
};
class SdSceneCPUUI : public IUIMethod
{
    SdSceneCPUContext &DIContext; // DI 必须
public:
    SdSceneCPUUI(SdSceneCPUContext &context) : DIContext(context) {}

    void renderUI() override
    {
        ImGui::Begin("BVH Debug");
        {
            if (ImGui::Button("Print BVH Tree"))
            {
                Output2DVisualization(*Storage::SdSceneInstance.pDataStorage, "BVHOutput.txt");
            }
        }
        ImGui::End();
        InteractableVisualization(*Storage::SdSceneInstance.pDataStorage); // 跟渲染的具体场景类型有关,应该交由pipeline提供的方法处理
        BVHDebugSettings::RenderVisualization(*Storage::SdSceneInstance.pDataStorage);
        BVHDebugSettings::RenderUI();
        RayVisualizer::Render(*Storage::SdSceneInstance.pDataStorage);
    }
};
class ImSceneCPUUI : public IUIMethod
{
    ImSceneCPUContext &DIContext; // DI 必须
public:
    ImSceneCPUUI(ImSceneCPUContext &context) : DIContext(context) {}

    void renderUI() override
    {
        BVHDebugSettings::RenderVisualization(DIContext.implicitSceneRef.BVHTree.root);
        BVHDebugSettings::RenderUI();
        RayVisualizer::Render(DIContext.implicitSceneRef.BVHTree.root);

    }
};