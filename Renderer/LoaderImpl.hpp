#pragma once
#include "RenderInterfaces.hpp"

//WIP
class SceneUpLoader : public IUpLoader
{
public:
    void upload(ILoadMethod &loadMethod) {
        std::cout << "SceneUpLoader uploading with given load method" << std::endl;
        loadMethod.load();
        std::cout << "SceneLoader finished uploading" << std::endl;
        
    }
    void run(GLFWwindow *mainContext) {
        std::cout << "SceneLoader running with main context" << std::endl;
        // Here you would typically create a new thread and set up the OpenGL context
    }
};
