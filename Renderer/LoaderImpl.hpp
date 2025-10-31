#pragma once
#include "RenderInterfaces.hpp"
#include "Storage.hpp"

#include <thread>
// WIP
class SceneUploader : public IUpLoader
{
public:
    void upload(ILoadMethod &loadMethod)
    {
        loadMethod.load();
    }
    void waitForCompletion()
    {
        // Wait for any asynchronous operations to complete
    }
};

class SceneAsyncLoader : public IUpLoader
{
private:
    GLFWwindow *loaderContext = nullptr;
    std::thread loaderThread;

    bool running = false;

    enum Status
    {
        Idle,
        Uploading,
        Completed
    };
    Status currentStatus = Status::Idle;
    std::mutex statusMutex;
    std::condition_variable statusCondVar;
    ILoadMethod *currentLoadMethod = nullptr;

public:
    SceneAsyncLoader()
    {
        auto mainContext = glfwGetCurrentContext();
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
        this->loaderContext = glfwCreateWindow(1, 1, "Loader", NULL, mainContext); // Shared with main context
        if (this->loaderContext == NULL)
        {
            throw std::runtime_error("Failed to create GLFW loader context");
        }
        auto loadingTask = [this, mainContext]()
        {
            glfwMakeContextCurrent(this->loaderContext);

            running = true;
            currentStatus = Status::Uploading;
            while (running)
            {
                std::unique_lock<std::mutex> lock(statusMutex);
                statusCondVar.wait(lock, [this]()
                                   { return currentStatus == Status::Uploading || !running; });
                if (!running)
                {
                    break;
                }

                currentLoadMethod ? currentLoadMethod->load() : void();

                // Notify upload completion
                currentStatus = Status::Completed;
                statusCondVar.notify_all();
            }
            glfwMakeContextCurrent(NULL);
        };

        loaderThread = std::thread(loadingTask);
    }

    ~SceneAsyncLoader()
    {
        {
            std::unique_lock<std::mutex> lock(statusMutex);
            running = false;
            statusCondVar.notify_all();
        }
        if (loaderThread.joinable())
        {
            loaderThread.join();
        }
        if (loaderContext)
        {
            glfwDestroyWindow(loaderContext);
            loaderContext = nullptr;
        }
    }

    void upload(ILoadMethod &loadMethod)
    {

        std::unique_lock<std::mutex> lock(statusMutex);
        currentLoadMethod = &loadMethod;
        currentStatus = Status::Uploading;
        statusCondVar.notify_one();
    }
    void waitForCompletion()
    {
        {
            std::unique_lock<std::mutex> lock(statusMutex);
            statusCondVar.wait(lock, [this]()
                               { return currentStatus == Status::Completed || !running; });
        }
    }
};
