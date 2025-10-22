#include "GPURayTracer.hpp"
#include "RenderState.hpp"
#include "DebugObjectRenderer.hpp"
#include "Renderer.hpp"
#include "Camera.hpp"
#include "Random.hpp"
#include "SimplifiedData.hpp"
#include "Utils.hpp"
#include "UI.hpp"

// Implementations for GPURayTracer methods

void GPURayTracer::initializeGLResources()
{
    glGenFramebuffers(1, &FBO1);
    glGenFramebuffers(1, &FBO2);
    raytraceSamplesTex1.generate(vp_width, vp_height, GL_RGBA32F, GL_RGBA, GL_FLOAT, NULL);
    raytraceSamplesTex2.generate(vp_width, vp_height, GL_RGBA32F, GL_RGBA, GL_FLOAT, NULL);

    nodeStorageTex.setFilterMax(GL_NEAREST);
    nodeStorageTex.setFilterMin(GL_NEAREST);
    nodeStorageTex.setWrapMode(GL_CLAMP_TO_EDGE);
    triangleStorageTex.setFilterMax(GL_NEAREST);
    triangleStorageTex.setFilterMin(GL_NEAREST);
    triangleStorageTex.setWrapMode(GL_CLAMP_TO_EDGE);
    nodeStorageTex.generate(1, 1, GL_R32F, GL_RED, GL_FLOAT, NULL, false);
    triangleStorageTex.generate(1, 1, GL_R32F, GL_RED, GL_FLOAT, NULL, false);

    // SSBO Initialization
    // nodeStorageBuf.generate(sd::NODESIZESSBO, GL_DYNAMIC_DRAW);
    // triangleStorageBuf.generate(sd::TRIANGLESIZESSBO, GL_DYNAMIC_DRAW);
}

GPURayTracer::GPURayTracer() {}

GPURayTracer::GPURayTracer(int _vp_width, int _vp_height, std::string _vs_path,
    std::string _fs_path) : Pass(_vp_width, _vp_height, _vs_path, _fs_path)
{
    initializeGLResources();
    contextSetup();
}

GPURayTracer::~GPURayTracer()
{
    glDeleteBuffers(1, &FBO1);
    glDeleteBuffers(1, &FBO2);
}

void GPURayTracer::reloadCurrentShaders()
{
    Pass::reloadCurrentShaders();
}

void GPURayTracer::contextSetup()
{
    glBindFramebuffer(GL_FRAMEBUFFER, FBO1);
    {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, raytraceSamplesTex1.ID, 0);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, FBO2);
    {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, raytraceSamplesTex2.ID, 0);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void GPURayTracer::resize(int _width, int _height)
{
    vp_width = _width;
    vp_height = _height;
    raytraceSamplesTex1.resize(_width, _height);
    raytraceSamplesTex2.resize(_width, _height);
    contextSetup();
}

void GPURayTracer::resetSamples()
{
    glBindFramebuffer(GL_FRAMEBUFFER, FBO1);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glBindFramebuffer(GL_FRAMEBUFFER, FBO2);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    samplesCount = 1;
}

unsigned int GPURayTracer::getTextures()
{
    return raytraceSamplesTex1.ID;
}

void GPURayTracer::render(TextureID skyTexID, TextureID sceneDataID)
{
    glViewport(0, 0, vp_width, vp_height);
    glBindFramebuffer(GL_FRAMEBUFFER, FBO1); // Ping 着色
    shaders.use();
    shaders.setTextureAuto(raytraceSamplesTex2.ID, GL_TEXTURE_2D, 0, "lastSample");
    shade(skyTexID, sceneDataID);
    glBindFramebuffer(GL_FRAMEBUFFER, FBO2); // Pong 着色
    shaders.setTextureAuto(raytraceSamplesTex1.ID, GL_TEXTURE_2D, 0, "lastSample");
    shade(skyTexID, sceneDataID);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void GPURayTracer::renderUI()
{
    static int toggleGammaCorrection = 1;
    static float gamma = 2.2f;
    ImGui::Begin("RenderUI");
    {
        RenderState::Dirty |= ImGui::DragFloat3("CamPosition", glm::value_ptr(Renderer::Cam.position), 0.01f);
        RenderState::Dirty |= ImGui::DragFloat3("LookAtCenter", glm::value_ptr(Renderer::Cam.lookAtCenter), 0.01f);
        RenderState::Dirty |= ImGui::DragFloat("CamFocalLength", &Renderer::Cam.focalLength, 0.01f);

        ImGui::Text(std::format("HFov: {}", Renderer::Cam.getHorizontalFOV()).c_str());
        ImGui::Text(std::format("SamplesCount: {}", samplesCount).c_str());

        ImGui::End();
    }
    SkySettings::RenderUI();
    DebugObjectRenderer::SetCamera(&Renderer::Cam);

}

void GPURayTracer::shade(TextureID skyTexID, TextureID sceneDataID)
{
    // 设置着色器参数
    shaders.setUniform("width", vp_width);
    shaders.setUniform("height", vp_height);
    float rand = Random::randomFloats(Random::generator);
    shaders.setUniform("rand", rand);
    shaders.setUniform("samplesCount", samplesCount);

    Renderer::Cam.setToFragShader(shaders, "cam");
    /***********************************场景设置 *********************************************************/
    // shaders.setTextureAuto(sceneDataID, GL_TEXTURE_2D, 0, "sceneData");

    // glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, nodeStorage.ID);
    // glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, triangleStorage.ID);

    shaders.setTextureAuto(nodeStorageTex.ID, GL_TEXTURE_2D, 0, "nodeStorageTex");
    shaders.setTextureAuto(triangleStorageTex.ID, GL_TEXTURE_2D, 0, "triangleStorageTex");
    shaders.setUniform("sceneRootIndex", static_cast<unsigned int>(sceneRootIndex));

    /****************************************天空设置*****************************************************/
    SkySettings::SetShaderUniforms(shaders);
    shaders.setTextureAuto(skyTexID, GL_TEXTURE_CUBE_MAP, 0, "skybox");

    DrawQuad();
    samplesCount++;
}

//只读取就绪时的CPU场景数据，转换为GPU可用的格式并上传
void GPURayTracer::setupSceneBuffers(SimplifiedData::DataStorage* dataStorage)
{
    static std::unique_ptr<sd::FlatNodeStorage> flatNodeStorage = std::make_unique<sd::FlatNodeStorage>();
    static std::unique_ptr<sd::FlatTriangleStorage> flatTriangleStorage = std::make_unique<sd::FlatTriangleStorage>();

    sd::ConvertToFlatStorage(*dataStorage, *flatNodeStorage.get(), *flatTriangleStorage.get());


    //设置纹理,也就是GPU数据存储
    // 纹理高度不为1
    if (nodeStorageTex.Width * nodeStorageTex.Height != flatNodeStorage->getSizeInBytes())
    {
        if (flatNodeStorage->getSizeInBytes() > GetTextureSizeLimit() * GetTextureSizeLimit())
        {
            throw(std::runtime_error(std::format("Storage Texture width beyond Limit: {} > {}", flatNodeStorage->getSizeInBytes(), GetTextureSizeLimit() * GetTextureSizeLimit())));
        }
        int newWidth = GetTextureSizeLimit();
        int newHeight = static_cast<int>(flatNodeStorage->getSizeInBytes()) / newWidth;
        nodeStorageTex.resize(newWidth, newHeight);
    }
    if (triangleStorageTex.Width * triangleStorageTex.Height != flatTriangleStorage->getSizeInBytes())
    {
        if (flatTriangleStorage->getSizeInBytes() > GetTextureSizeLimit() * GetTextureSizeLimit())
        {
            throw(std::runtime_error(std::format("Storage Texture width beyond Limit: {} > {}", flatTriangleStorage->getSizeInBytes(), GetTextureSizeLimit() * GetTextureSizeLimit())));
        }
        int newWidth = GetTextureSizeLimit();
        int newHeight = static_cast<int>(flatTriangleStorage->getSizeInBytes()) / newWidth;
        triangleStorageTex.resize(newWidth, newHeight);
    }
    nodeStorageTex.setData(flatNodeStorage->nodes.data());
    triangleStorageTex.setData(flatTriangleStorage->triangles.data());

    sceneRootIndex = dataStorage->rootIndex;

}

void GPURayTracer::setupSceneBuffers(sd::FlatNodeStorage* flatNodeStorage, sd::FlatTriangleStorage* flatTriangleStorage, size_t rootIndex)
{
    //设置纹理,也就是GPU数据存储
// 纹理高度不为1
    if (nodeStorageTex.Width * nodeStorageTex.Height != flatNodeStorage->getSizeInBytes())
    {
        if (flatNodeStorage->getSizeInBytes() > GetTextureSizeLimit() * GetTextureSizeLimit())
        {
            throw(std::runtime_error(std::format("Storage Texture width beyond Limit: {} > {}", flatNodeStorage->getSizeInBytes(), GetTextureSizeLimit() * GetTextureSizeLimit())));
        }
        int newWidth = GetTextureSizeLimit();
        int newHeight = static_cast<int>(flatNodeStorage->getSizeInBytes()) / newWidth;
        nodeStorageTex.resize(newWidth, newHeight);
    }
    if (triangleStorageTex.Width * triangleStorageTex.Height != flatTriangleStorage->getSizeInBytes())
    {
        if (flatTriangleStorage->getSizeInBytes() > GetTextureSizeLimit() * GetTextureSizeLimit())
        {
            throw(std::runtime_error(std::format("Storage Texture width beyond Limit: {} > {}", flatTriangleStorage->getSizeInBytes(), GetTextureSizeLimit() * GetTextureSizeLimit())));
        }
        int newWidth = GetTextureSizeLimit();
        int newHeight = static_cast<int>(flatTriangleStorage->getSizeInBytes()) / newWidth;
        triangleStorageTex.resize(newWidth, newHeight);
    }
    nodeStorageTex.setData(flatNodeStorage->nodes.data());
    triangleStorageTex.setData(flatTriangleStorage->triangles.data());

    sceneRootIndex = rootIndex;
}
