#pragma once
#include "RenderInterfaces.hpp"
#include "Storage.hpp"

#include <thread>
// WIP
class SceneUploader : public IUpLoader
{
public:
    void upload(ILoadMethod &loadMethod) override;
    void waitForCompletion() override;
};

class SceneAsyncLoader : public IUpLoader
{
private:
    GLFWwindow *loaderContext = nullptr;
    std::thread loaderThread;
    bool running = false;
    enum Status { Idle, Uploading, Completed };
    Status currentStatus = Status::Idle;
    std::mutex statusMutex;
    std::condition_variable statusCondVar;
    ILoadMethod *currentLoadMethod = nullptr;
public:
    SceneAsyncLoader();
    ~SceneAsyncLoader();
    void upload(ILoadMethod &loadMethod) override;
    void waitForCompletion() override;
};
