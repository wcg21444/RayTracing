#pragma once

#include "UICommon.hpp"
#include "RenderState.hpp"
#include "DebugObjectRenderer.hpp"
#include "../CPURayTrace/Scene.hpp"

class BVHSettings
{
public:
    inline static int maxDepth = 40;
    inline static int minDepth = 0;
    inline static bool toggleVisualizeBVH = true;
    inline static bool toggleBVHAccel = true;

    inline static void RenderUI()
    {
        ImGui::Begin("BVH Debug");
        {
            ImGui::DragInt("Max Depth", &maxDepth, 1, 1, 40);
            ImGui::DragInt("Min Depth", &minDepth, 1, 0, 40);
            ImGui::Checkbox("Visualize BVH", &toggleVisualizeBVH);
            RenderState::Dirty |= ImGui::Checkbox("BVH Acceleration", &toggleBVHAccel);
        }
        ImGui::End();
    }

    inline static void RenderVisualization(BVHNode *root)
    {
        std::function<void(BVHNode *, int)> traverse = [&](BVHNode *node, int depth) -> void
        {
            if (!node)
                return;
            if (node->object)
            {
                auto AABB = node->object->getBoundingBox();
                if (toggleVisualizeBVH)
                {
                    DebugObjectRenderer::AddDrawCall([AABB, depth](Shader &_shaders)
                                                     { DebugObjectRenderer::DrawWireframeCube(_shaders, AABB.pMin, AABB.pMax, color4(1.0f, 0.0f, 0.0f, 1.0f)); });
                }
            }
            else
            {
                auto AABB = node->box;
                if (depth <= maxDepth && depth >= minDepth && toggleVisualizeBVH)
                {
                    DebugObjectRenderer::AddDrawCall([AABB, depth](Shader &_shaders)
                                                     { DebugObjectRenderer::DrawWireframeCube(_shaders, AABB.pMin, AABB.pMax, color4(0.0f, 1.0f, depth / 8.f, 1.0f)); });
                }
            }
            traverse(node->left, depth + 1);
            traverse(node->right, depth + 1);
        };
        traverse(root, 0);
    }
};