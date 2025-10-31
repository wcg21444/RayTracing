#include "NewRenderer.hpp"
#include "TracerImpl.hpp"
#include "LoaderImpl.hpp"
#include "Renderer.hpp"
#include "PostProcessor.hpp"
#include "SkyTexPass.hpp"
#include "RenderState.hpp"
#include "Storage.hpp"
#include "RenderContexts.hpp"
#include "LoadMethods.hpp"
#include "TraceMethods.hpp"

NewRenderer::NewRenderer()
    : screenPass(std::make_unique<ScreenPass>(width, height, "GLSL/screenQuad.vs", "GLSL/screenOutput.fs")),
      postProcessor(std::make_unique<PostProcessor>(width, height, "GLSL/screenQuad.vs", "GLSL/postProcess.fs")),
      skyTexPass(std::make_unique<SkyTexPass>("GLSL/cubemapSphere.vs", "GLSL/skyTex.fs", 256)),
      tracer(std::make_unique<TracerAsync>(width, height)),
      uploader(std::make_unique<SceneAsyncLoader>()),
      onResize(std::make_shared<ResizeCallback>(
          [this](int newWidth, int newHeight)
          {
              this->resize(newWidth, newHeight);
          }))
{
    changeMode(RenderMode::CPU_SdScene);
}

NewRenderer::~NewRenderer()
{
}

void NewRenderer::changeMode(RenderMode newMode)
{
    renderMode = newMode;
    // tracer->wait(); //确保上一次渲染完成
    // sceneLoader->wait(); //确保上一次上传完成
    tracer->waitForCompletion();
    uploader->waitForCompletion();
    tracer->resetSamples();
    switch (renderMode)
    {
    case RenderMode::GPU_SdScene:
        currentPipeline = std::make_unique<RenderPipeline<SdSceneGPUContext, LoadSdSceneGPU, TraceSdSceneGPU>>(
            SdSceneGPUContext{
                skyboxTextureID,
                cam});
        break;
    case RenderMode::CPU_SdScene:
        currentPipeline = std::make_unique<RenderPipeline<SdSceneCPUContext, LoadSdSceneCPU, TraceSdSceneCPU>>(
            SdSceneCPUContext{
                cam});
        break;
    case RenderMode::CPU_Scene:
        currentPipeline = std::make_unique<RenderPipeline<SceneCPUContext, LoadSceneCPU, TraceSceneCPU>>(
            SceneCPUContext{
                cam});
        break;

    default:
        assert(false && "Unknown RenderMode");
    }
    RenderState::Dirty |= true;
}

void NewRenderer::render()
{
    assert(currentPipeline && "RenderPipeline not set in NewRenderer::render");
    // 对pipeline context的绑定修改不需要同步?句柄引用?
    tracer->waitForCompletion(); // 帧同步
    uploader->waitForCompletion();
    // Preprocessing
    skyTexPass->render(cam.position);
    skyboxTextureID = skyTexPass->getCubemap();
    // 需要注入到GPU渲染管线

    auto loadMethod = currentPipeline->getLoadMethod();
    auto traceMethod = currentPipeline->getTraceMethod();
    assert(loadMethod && "ILoadMethod not set in NewRenderer::render");
    assert(traceMethod && "ITraceMethod not set in NewRenderer::render");

    if (RenderState::SceneDirty)
    {
        tracer->resetSamples();
        uploader->upload(*loadMethod);
        RenderState::SceneDirty = false;
    }
    if (RenderState::Dirty)
    {
        tracer->resetSamples();
        RenderState::Dirty = false;
    }
    tracer->render(*traceMethod);
    // postprocessing
    auto raytraceResultID = tracer->getTraceOutputTextureID();
    postProcessor->render(raytraceResultID);
    auto postProcessed = postProcessor->getTextures();

    auto debugRendererOutput = DebugObjectRenderer::GetRenderOutput();

    screenPass->render(postProcessed, debugRendererOutput);

    renderUI();
}

void NewRenderer::resize(int newWidth, int newHeight)
{
    width = newWidth;
    height = newHeight;

    screenPass->resize(width, height);
    postProcessor->resize(width, height);
    if (tracer)
    {
        tracer->resize(width, height);
    }

    RenderState::Dirty |= true;
}

void NewRenderer::shutdown()
{
    // 同步等待所有任务完成
    tracer->waitForCompletion();
    uploader->waitForCompletion();
}

void NewRenderer::renderUI()
{
    ImGui::Begin("RenderUI");
    {
        if ((ImGui::Button("Reload")))
        {
            Shader::ReloadAll();
            RenderState::Dirty |= true;
        }

        // Render mode selection (checkbox style, mutually exclusive)
        {
            struct ModeItem
            {
                const char *label;
                RenderMode mode;
            };
            static const ModeItem modeItems[] = {
                {"CPU SD Scene", RenderMode::CPU_SdScene},
                {"GPU SD Scene", RenderMode::GPU_SdScene},
                {"CPU Old Scene", RenderMode::CPU_Scene}};
            static int selectedMode = 0; // 选中状态
            for (int i = 0; i < std::size(modeItems); ++i)
            {
                if (renderMode == modeItems[i].mode)
                    selectedMode = i; // 初始化
            }
            if (ImGui::BeginListBox("Select Mode"))
            {
                for (int i = 0; i < std::size(modeItems); ++i)
                {
                    bool checked = (selectedMode == i);                // 渲染checkbox checked状态
                    if (ImGui::Checkbox(modeItems[i].label, &checked)) // 用户输入将改变checked状态
                    {
                        if (checked && selectedMode != i)
                        {
                            selectedMode = i; // 状态切换
                            if (renderMode != modeItems[i].mode)
                            {
                                changeMode(modeItems[i].mode); // 状态切换响应?
                                RenderState::Dirty = true;
                                RenderState::SceneDirty = true;
                            }
                        }
                    }
                }
                ImGui::EndListBox();
            }
        }

        {
            RenderState::Dirty |= ImGui::DragFloat3("CamPosition", glm::value_ptr(cam.position), 0.01f);
            RenderState::Dirty |= ImGui::DragFloat3("LookAtCenter", glm::value_ptr(cam.lookAtCenter), 0.01f);
            RenderState::Dirty |= ImGui::DragFloat("CamFocalLength", &cam.focalLength, 0.01f);
            ImGui::Text(std::format("HFov: {}", cam.getHorizontalFOV()).c_str());
        }
        ImGui::End();
    }
}
