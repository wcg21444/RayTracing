
#pragma once
#include "Pass.hpp"
#include "Texture.hpp"
#include <string>
#include <imgui.h>
#include <glm/gtc/type_ptr.hpp>

namespace SimplifiedData
{
    struct DataStorage;
    struct FlatNodeStorage;
    struct FlatTriangleStorage;
}

class GPURayTracer : public Pass
{
private:
    unsigned int FBO1;
    unsigned int FBO2;

    Texture2D raytraceSamplesTex1;
    Texture2D raytraceSamplesTex2;

    SSBO nodeStorageBuf;
    SSBO triangleStorageBuf;

    Texture2D nodeStorageTex;
    Texture2D triangleStorageTex;

    size_t sceneRootIndex = 0;

    int samplesCount = 1;

private:
    void initializeGLResources();
    void shade(TextureID skyTexID, TextureID sceneDataID);

public:
    GPURayTracer();
    GPURayTracer(int _vp_width, int _vp_height, std::string _vs_path, std::string _fs_path);
    ~GPURayTracer();

    void reloadCurrentShaders() override;
    void contextSetup() override;
    void resize(int _width, int _height);
    void resetSamples();
    unsigned int getTextures();
    void render(TextureID skyTexID, TextureID sceneDataID);
    void renderUI();

    void setupSceneBuffers(SimplifiedData::DataStorage *dataStorage);
    
    void setupSceneBuffers(SimplifiedData::FlatNodeStorage *flatNodeStorage, SimplifiedData::FlatTriangleStorage *flatTriangleStorage,size_t rootIndex);
};
