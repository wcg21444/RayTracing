#include "LoaderImpl.hpp"
#include <stdexcept>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <GLFW/glfw3.h>

// SceneUploader
void SceneUploader::upload(ILoadMethod &loadMethod) {
    loadMethod.load();
}
void SceneUploader::waitForCompletion() {
    // Wait for any asynchronous operations to complete
}

// SceneAsyncLoader
SceneAsyncLoader::SceneAsyncLoader() {
    auto mainContext = glfwGetCurrentContext();
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    this->loaderContext = glfwCreateWindow(1, 1, "Loader", NULL, mainContext); // Shared with main context
    if (this->loaderContext == NULL) {
        throw std::runtime_error("Failed to create GLFW loader context");
    }
    auto loadingTask = [this, mainContext]() {
        glfwMakeContextCurrent(this->loaderContext);
        running = true;
        currentStatus = Status::Uploading;
        while (running) {
            std::unique_lock<std::mutex> lock(statusMutex);
            statusCondVar.wait(lock, [this]() { return currentStatus == Status::Uploading || !running; });
            if (!running) {
                break;
            }
            currentLoadMethod ? currentLoadMethod->load() : void();
            currentStatus = Status::Completed;
            statusCondVar.notify_all();
        }
        glfwMakeContextCurrent(NULL);
    };
    loaderThread = std::thread(loadingTask);
}

SceneAsyncLoader::~SceneAsyncLoader() {
    {
        std::unique_lock<std::mutex> lock(statusMutex);
        running = false;
        statusCondVar.notify_all();
    }
    if (loaderThread.joinable()) {
        loaderThread.join();
    }
    if (loaderContext) {
        glfwDestroyWindow(loaderContext);
        loaderContext = nullptr;
    }
}

void SceneAsyncLoader::upload(ILoadMethod &loadMethod) {
    std::unique_lock<std::mutex> lock(statusMutex);
    currentLoadMethod = &loadMethod;
    currentStatus = Status::Uploading;
    statusCondVar.notify_one();
}
void SceneAsyncLoader::waitForCompletion() {
    {
        std::unique_lock<std::mutex> lock(statusMutex);
        statusCondVar.wait(lock, [this]() { return currentStatus == Status::Completed || !running; });
    }
}
