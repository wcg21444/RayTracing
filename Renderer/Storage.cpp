#include "Storage.hpp"
#include "Utils.hpp"
namespace Storage
{
    SceneBundle SceneBundleRendering;
    std::shared_mutex SceneBundleRenderingMutex;
    sd::Scene SdScene;
    std::shared_mutex SdSceneMutex;
    Scene OldScene;    
    std::shared_mutex OldSceneMutex;

    void InitializeSceneRendering()
    {
        auto &[nodeStorageTexRendering, triangleStorageTexRendering, sceneRootIndexRendering] = Storage::SceneBundleRendering;
        nodeStorageTexRendering.setFilterMax(GL_NEAREST);
        nodeStorageTexRendering.setFilterMin(GL_NEAREST);
        nodeStorageTexRendering.setWrapMode(GL_CLAMP_TO_EDGE);
        triangleStorageTexRendering.setFilterMax(GL_NEAREST);
        triangleStorageTexRendering.setFilterMin(GL_NEAREST);
        triangleStorageTexRendering.setWrapMode(GL_CLAMP_TO_EDGE);
        nodeStorageTexRendering.generate(1, 1, GL_R32F, GL_RED, GL_FLOAT, NULL, false);
        triangleStorageTexRendering.generate(1, 1, GL_R32F, GL_RED, GL_FLOAT, NULL, false);
    }
    void InitializeSceneBundle(SceneBundle &sceneBundle)
    {
        auto &[nodeStorageTex, triangleStorageTex, sceneRootIndex] = sceneBundle;
        nodeStorageTex.setFilterMax(GL_NEAREST);
        nodeStorageTex.setFilterMin(GL_NEAREST);
        nodeStorageTex.setWrapMode(GL_CLAMP_TO_EDGE);
        triangleStorageTex.setFilterMax(GL_NEAREST);
        triangleStorageTex.setFilterMin(GL_NEAREST);
        triangleStorageTex.setWrapMode(GL_CLAMP_TO_EDGE);
        nodeStorageTex.generate(GetTextureSizeLimit(), 1, GL_R32F, GL_RED, GL_FLOAT, NULL, false);
        triangleStorageTex.generate(GetTextureSizeLimit(), 1, GL_R32F, GL_RED, GL_FLOAT, NULL, false);
    }
}
