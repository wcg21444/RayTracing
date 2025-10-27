#pragma once

#include <thread>
#include <shared_mutex>
#include <condition_variable>

#include "Utils.hpp"
#include "SimplifiedData.hpp"
#include "Texture.hpp"

#include "Scene.hpp"

namespace sd = SimplifiedData;

namespace Storage
{

    struct SceneBundle
    {
        Texture2D nodeStorageTex;
        Texture2D triangleStorageTex;
        uint32_t sceneRootIndex = 0;
    };

    extern SceneBundle SceneBundleRendering;
    extern std::shared_mutex SceneBundleRenderingMutex;
    extern sd::Scene SdScene;
    extern std::shared_mutex SdSceneMutex;


    void InitializeSceneRendering();

    class SceneLoader
    {
    private:
        std::thread loaderThread;
        GLFWwindow *loadingContext;
        SceneBundle sceneBundleLoading;

        std::mutex MutexNeedUpload;
        std::condition_variable CondNeedUpload;
        bool NeedUpload = true;

        void loading();                 // 加载场景数据到loading纹理
        void swapToRenderingTextures(); // 交换loading和rendering纹理
    public:
        bool running = false;
        SceneLoader();

        void run(GLFWwindow *mainContext); // 发起线程,创建GL上下文

        void upload(); // 提醒挂起的上载线程进行上载

        void swapTextures();

        ~SceneLoader();
    };

    extern SceneLoader SdSceneLoader;
}