#pragma once

#include "Utils.hpp"
#include "Camera.hpp"
#include <imgui.h>
namespace RenderState
{
     inline bool Dirty = true; // 标记渲染器状态是否需要重置采样 所有更新方法都需要将此变量置true 所有重绘方法都需要检查此变量 并将其置false
     inline bool SceneDirty = true;
     inline bool IsUIInteracting(){
          return ImGui::GetIO().WantCaptureMouse || ImGui::GetIO().WantCaptureKeyboard;
     }

     inline Camera CameraInstance = Camera(1.0f, point3(0.0f, 0.0f, 1.0f), 2.0f, float(16) / float(9));

     inline int InitWidth = 640;
     inline int InitHeight = 360;

     inline size_t CPUNumThreads = 16;
}