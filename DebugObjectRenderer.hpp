#pragma once

#include "Pass.hpp"

#include <iostream>
#include <functional>
#include <queue>
#include <memory>

using DebugObjectDrawCall = std::function<void(Shader &debugObjectShaders)>;
class RenderTarget;
class Texture2D;
class Camera;

class DebugObjectPass : public Pass
{
private:
    std::shared_ptr<RenderTarget> renderTarget = nullptr;
    std::shared_ptr<Texture2D> debugObjectPassTex = nullptr;

    void initializeGLResources() override;
    void cleanUpGLResources();

public:
    DebugObjectPass(int _vp_width, int _vp_height, std::string _vs_path, std::string _fs_path);
    ~DebugObjectPass()
    {
        cleanUpGLResources();
    }

    void contextSetup() override;

    void resize(int _width, int _height) override;

    void render(std::queue<DebugObjectDrawCall> &drawQueue, Camera &cam);

    unsigned int getTexture();
};

// 辅助对象渲染器. 单独使用一条渲染管线
// 不应该存储任何渲染参数状态,所有状态都应该在调用时传入
class DebugObjectRenderer
{
private:
    inline static std::queue<DebugObjectDrawCall> drawQueue;
    inline static int width = 1600;
    inline static int height = 900;
    inline static Camera* camera = nullptr;
    inline static std::shared_ptr<DebugObjectPass> debugObjectPass;

public:
    static void AddDrawCall(const DebugObjectDrawCall &drawCall);
    static void Initialize();
    static void Resize(int _width, int _height);
    static void Render();
    static void SetCamera(Camera* cam)
    {
        camera = cam;
    }
    static void ReloadCurrentShaders();
    static unsigned int GetRenderOutput();
    static void CheckInitialized();
    static void DrawCube(Shader &shaders, glm::vec4 color = glm::vec4(1.0f), glm::mat4 modelMatrix = glm::identity<glm::mat4>());
    static void DrawWireframeCube(Shader &shaders, glm::vec4 color = glm::vec4(1.0f), glm::mat4 modelMatrix = glm::identity<glm::mat4>());
};
