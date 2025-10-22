#pragma once

#include "UICommon.hpp"
#include "RenderState.hpp"
#include "DebugObjectRenderer.hpp"
#include "Scene.hpp"
#include "SimplifiedData.hpp"
class BVHSettings
{
public:
    inline static int maxDepth = 40;
    inline static int minDepth = 0;
    inline static bool toggleVisualizeBVH = false;
    inline static bool toggleBVHAccel = true;
    inline static bool showLeafAABB = false;

    inline static void RenderUI()
    {
        ImGui::Begin("BVH Debug");
        {
            ImGui::DragInt("Max Depth", &maxDepth, 1, 1, 40);
            ImGui::DragInt("Min Depth", &minDepth, 1, 0, 40);
            ImGui::Checkbox("Visualize BVH", &toggleVisualizeBVH);
            ImGui::Checkbox("Show Leaf AABB", &showLeafAABB);
            RenderState::Dirty |= ImGui::Checkbox("BVH Acceleration", &toggleBVHAccel);
        }
        ImGui::End();
    }

    inline static void RenderVisualization(BVHNode *root)
    {
        if (!toggleVisualizeBVH)
            return;
        std::function<void(BVHNode *, int)> traverse = [&](BVHNode *node, int depth) -> void
        {
            if (!node)
                return;
            if (node->object)
            {
                auto AABB = node->object->getBoundingBox();

                DebugObjectRenderer::AddDrawCall([AABB, depth](Shader &_shaders)
                                                 { DebugObjectRenderer::DrawWireframeCube(_shaders, AABB.pMin, AABB.pMax, color4(1.0f, 0.0f, 0.0f, 1.0f)); });
            }
            else
            {
                auto AABB = node->box;
                if (depth <= maxDepth && depth >= minDepth)
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
    inline static void RenderVisualization(const sd::DataStorage &dataStorage)
    {

        static thread_local std::array<uint32_t, 32> callStack; // 假设栈深度不会超过32
        static thread_local size_t top = 0;

        if (!toggleVisualizeBVH)
            return;
        size_t depth = 0;

        callStack[top++] = dataStorage.rootIndex;

        while (top > 0)
        {
            depth = top;
            uint32_t index = callStack[--top];
            const sd::Node &node = dataStorage.nodeStorage.nodes[index];

            if (index == sd::invalidIndex)
            {
                continue;
            }
            if (node.flags == sd::NODE_LEAF) // 叶子节点
            {
                if (!showLeafAABB)
                    continue;
                auto AABB = node.box;

                DebugObjectRenderer::AddDrawCall([AABB](Shader &_shaders)
                                                 { DebugObjectRenderer::DrawWireframeCube(_shaders, AABB.pMin, AABB.pMax, color4(1.0f, 0.0f, 0.0f, 1.0f)); });
                continue;
            }
            else
            {
                auto AABB = node.box;
                if (depth <= maxDepth && depth >= minDepth)
                {
                    DebugObjectRenderer::AddDrawCall([AABB, depth](Shader &_shaders)
                                                     { DebugObjectRenderer::DrawWireframeCube(_shaders, AABB.pMin, AABB.pMax, color4(0.0f, 1.0f, depth / 8.f, 1.0f)); });
                }
            }
            callStack[top++] = node.left;
            callStack[top++] = node.right;
        }
    }
};