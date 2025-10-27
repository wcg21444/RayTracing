#include "Storage.hpp"
#include "Utils.hpp"
namespace Storage
{
    SceneBundle SceneBundleRendering;
    std::shared_mutex SceneBundleRenderingMutex;
    sd::Scene SdScene;
    std::shared_mutex SdSceneMutex;

    SceneLoader SdSceneLoader;

    SceneLoader::SceneLoader()
    {
    }

    // In Loader Thread
    void SceneLoader::loading()
    {
        glfwMakeContextCurrent(loadingContext);
        static auto &[nodeStorageTexLoading, triangleStorageTexLoading, rootIndex] = sceneBundleLoading;
        nodeStorageTexLoading.setFilterMax(GL_NEAREST);
        nodeStorageTexLoading.setFilterMin(GL_NEAREST);
        nodeStorageTexLoading.setWrapMode(GL_CLAMP_TO_EDGE);
        triangleStorageTexLoading.setFilterMax(GL_NEAREST);
        triangleStorageTexLoading.setFilterMin(GL_NEAREST);
        triangleStorageTexLoading.setWrapMode(GL_CLAMP_TO_EDGE);
        nodeStorageTexLoading.generate(GetTextureSizeLimit(), 1, GL_R32F, GL_RED, GL_FLOAT, NULL, false);
        triangleStorageTexLoading.generate(GetTextureSizeLimit(), 1, GL_R32F, GL_RED, GL_FLOAT, NULL, false);

        static thread_local std::unique_ptr<sd::FlatNodeStorage> flatNodeStorage = std::make_unique<sd::FlatNodeStorage>();
        static thread_local std::unique_ptr<sd::FlatTriangleStorage> flatTriangleStorage = std::make_unique<sd::FlatTriangleStorage>();

        std::cout << "SceneLoader initialized." << std::endl;

        while (running)
        {
            // Load scene data into loading textures

            // Wait for upload signal
            {
                std::unique_lock<std::mutex> lock(MutexNeedUpload);
                CondNeedUpload.wait(lock, [this]()
                                    { return NeedUpload || !running; });
                if (!running)
                {
                    break;
                }
                NeedUpload = false;
            }
            // Load scene data into Converted Flat Storage Buffers
            {
                std::shared_lock<std::shared_mutex> sdSceneLock(SdSceneMutex); // read lock
                sd::ConvertToFlatStorage(*SdScene.pDataStorage, *flatNodeStorage.get(), *flatTriangleStorage.get());
                rootIndex = SdScene.pDataStorage->rootIndex;
            }

            std::cout << "Uploading Scene: Nodes float=" << flatNodeStorage->nodes.size() << " Triangles float=" << flatTriangleStorage->triangles.size() << std::endl;
            std::cout << "Scene Root Index: " << rootIndex << std::endl;

            // Resize loading textures if needed
            if (nodeStorageTexLoading.Width * nodeStorageTexLoading.Height < flatNodeStorage->getSizeInFloats())
            {
                if (flatNodeStorage->getSizeInFloats() > GetTextureSizeLimit() * GetTextureSizeLimit())
                {
                    throw(std::runtime_error(std::format("Storage Texture width beyond Limit: {} > {}", flatNodeStorage->getSizeInFloats(), GetTextureSizeLimit() * GetTextureSizeLimit())));
                }
                int newWidth = GetTextureSizeLimit();
                int newHeight = static_cast<int>(flatNodeStorage->getSizeInFloats()) / newWidth + 1; // 上取整
                nodeStorageTexLoading.resize(newWidth, newHeight);

                std::cout << "Resized Node Storage Texture to: " << nodeStorageTexLoading.Width << "x" << nodeStorageTexLoading.Height << std::endl;
            }
            // Resizing Triangle Storage Texture if needed
            if (triangleStorageTexLoading.Width * triangleStorageTexLoading.Height < flatTriangleStorage->getSizeInFloats())
            {
                if (flatTriangleStorage->getSizeInFloats() > GetTextureSizeLimit() * GetTextureSizeLimit())
                {
                    throw(std::runtime_error(std::format("Storage Texture width beyond Limit: {} > {}", flatTriangleStorage->getSizeInFloats(), GetTextureSizeLimit() * GetTextureSizeLimit())));
                }
                int newWidth = GetTextureSizeLimit();
                int newHeight = static_cast<int>(flatTriangleStorage->getSizeInFloats()) / newWidth + 1;
                triangleStorageTexLoading.resize(newWidth, newHeight);

                std::cout << "Resized Triangle Storage Texture to: " << triangleStorageTexLoading.Width << "x" << triangleStorageTexLoading.Height << std::endl;
            }

            nodeStorageTexLoading.setData(flatNodeStorage->nodes.data());
            triangleStorageTexLoading.setData(flatTriangleStorage->triangles.data());

            swapToRenderingTextures();
        }

        glfwMakeContextCurrent(NULL);
    }

    void SceneLoader::swapToRenderingTextures()
    {
        static auto &[nodeStorageTexLoading, triangleStorageTexLoading, rootIndex] = sceneBundleLoading;
        // Upload data to rendering textures By swapping
        {
            std::unique_lock<std::shared_mutex> sceneRenderingLock(SceneBundleRenderingMutex); // write lock
            auto &[NodeStorageTexRendering, TriangleStorageTexRendering, SceneRootIndexRendering] = SceneBundleRendering;

            std::cout << "Loading Data set." << std::endl;

            if (triangleStorageTexLoading.Width != TriangleStorageTexRendering.Width ||
                triangleStorageTexLoading.Height != TriangleStorageTexRendering.Height)
            {
                TriangleStorageTexRendering.resize(triangleStorageTexLoading.Width, triangleStorageTexLoading.Height);
                std::cout << "Resized Rendering Triangle Storage Texture to: " << TriangleStorageTexRendering.Width << "x" << TriangleStorageTexRendering.Height << std::endl;
            }
            if (nodeStorageTexLoading.Width != NodeStorageTexRendering.Width ||
                nodeStorageTexLoading.Height != NodeStorageTexRendering.Height)
            {
                NodeStorageTexRendering.resize(nodeStorageTexLoading.Width, nodeStorageTexLoading.Height);
                std::cout << "Resized Rendering Node Storage Texture to: " << NodeStorageTexRendering.Width << "x" << NodeStorageTexRendering.Height << std::endl;
            }
            std::cout << "Loading Node Texture ID: " << nodeStorageTexLoading.ID << std::endl;
            std::cout << "Rendering Node Texture ID: " << NodeStorageTexRendering.ID << std::endl;

            // std::swap(nodeStorageTexLoading, NodeStorageTexRendering);
            // std::swap(triangleStorageTexLoading, TriangleStorageTexRendering);

            std::swap(this->sceneBundleLoading, SceneBundleRendering);

            std::cout << "Loading Node Texture ID: " << nodeStorageTexLoading.ID << std::endl;
            std::cout << "Rendering Node Texture ID: " << NodeStorageTexRendering.ID << std::endl;

            std::cout << "Scene Upload Completed." << std::endl;
        }
    }

    void SceneLoader::run(GLFWwindow *mainContext)
    {
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
        loadingContext = glfwCreateWindow(1, 1, "loaderContext", NULL, mainContext);
        running = true;
        loaderThread = std::thread(&SceneLoader::loading, this);
    }
    void SceneLoader::upload()
    {
        {
            std::unique_lock<std::mutex> lock(MutexNeedUpload);
            NeedUpload = true;
        }
        CondNeedUpload.notify_all();
    }
    void SceneLoader::swapTextures()
    {
        std::swap(this->sceneBundleLoading, SceneBundleRendering);
    }
    SceneLoader::~SceneLoader()
    {
        running = false;
        CondNeedUpload.notify_all();
        if (loaderThread.joinable())
        {
            loaderThread.join();
        }
        if (loadingContext)
        {
            glfwDestroyWindow(loadingContext);
        }
    }
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
}
