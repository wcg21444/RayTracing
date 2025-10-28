#pragma once
#include "RenderInterfaces.hpp"
#include "Storage.hpp"
//WIP
class SceneUploader : public IUpLoader
{
public:
    
    void upload(ILoadMethod &loadMethod) {
        std::cout << "SceneUploader uploading with given load method" << std::endl;
        loadMethod.load();
        std::cout << "SceneUploader finished uploading" << std::endl;

    }
    void waitForCompletion() {
        // Wait for any asynchronous operations to complete
    }
};
