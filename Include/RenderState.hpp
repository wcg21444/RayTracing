#pragma once

namespace RenderState
{
     inline bool Dirty = true; // 标记渲染器状态是否需要重置采样 所有更新方法都需要将此变量置true 所有重绘方法都需要检查此变量 并将其置false
     inline bool SceneDirty = true;
     inline bool IsUIInteracting(){
          return ImGui::GetIO().WantCaptureMouse || ImGui::GetIO().WantCaptureKeyboard;
     }
}