#pragma once

#include "UICommon.hpp"
#include "RenderState.hpp"
#include "DebugObjectRenderer.hpp"
#include "Scene.hpp"
#include "SimplifiedData.hpp"
#include <iomanip>
#include <unordered_set>
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
    // TODO 格式化输出BVH, 2D可视化到一个文件

    inline static void Output2DVisualization(const sd::DataStorage &dataStorage, const std::string &filePath)
    {
        std::ofstream outFile(filePath);
        if (!outFile.is_open())
        {
            std::cerr << "Failed to open file for BVH visualization: " << filePath << std::endl;
            return;
        }
        outFile << std::fixed << std::setprecision(2);
        std::vector<std::string> paddingStack;

        auto dfs = [&](auto &&dfsSelf, uint32_t nodeIndex, bool isRightChild) -> void
        {
            if (nodeIndex == sd::invalidIndex)
                return;

            const sd::Node &node = dataStorage.nodeStorage.nodes[nodeIndex];

            for (const auto &pad : paddingStack)
            {
                outFile << pad;
            }

            if (!paddingStack.empty())
            {
                outFile << (isRightChild ? "└──" : "├──");
            }

            if (node.flags == sd::NODE_LEAF)
            {
                auto tri = dataStorage.triangleStorage.triangles[node.left];
                outFile << " Leaf[" << nodeIndex << "] tri:" << node.left
                        << " [" << node.box.pMin.x << "," << node.box.pMin.y << "," << node.box.pMin.z << "]"
                        << "-[" << node.box.pMax.x << "," << node.box.pMax.y << "," << node.box.pMax.z << "]"
                        << " vertices0: [" << tri.positions[0].x << "," << tri.positions[0].y << "," << tri.positions[0].z << "]  "
                        << " vertices1: [" << tri.positions[1].x << "," << tri.positions[1].y << "," << tri.positions[1].z << "]  "
                        << " vertices2: [" << tri.positions[2].x << "," << tri.positions[2].y << "," << tri.positions[2].z << "]\n";
            }
            else
            {
                outFile << " Node[" << nodeIndex << "] L:" << node.left << " R:" << node.right
                        << " [" << node.box.pMin.x << "," << node.box.pMin.y << "," << node.box.pMin.z << "]"
                        << "-[" << node.box.pMax.x << "," << node.box.pMax.y << "," << node.box.pMax.z << "]\n";

                paddingStack.push_back(isRightChild ? "    " : "│   ");

                dfsSelf(dfsSelf, node.left, false); // 左子节点
                dfsSelf(dfsSelf, node.right, true); // 右子节点

                paddingStack.pop_back();
            }
        };

        outFile << "BVH Tree (Root: " << dataStorage.rootIndex << ")\n";
        dfs(dfs, dataStorage.rootIndex, false);
        outFile.close();
        std::cout << "BVH visualization written to " << filePath << std::endl;
    }
    inline static void InteractableVisualization(const sd::DataStorage &dataStorage)
    {
        // TODO 实现交互式BVH可视化
        // 思路:
        // 1. 使用ImGui树形结构显示BVH节点
        // 2. 点击节点时，在3D视图中高亮显示对应的AABB
        // 具体展开
        // 创建一张哈希表:<index,status>
        ////// 为什么使用index作为映射键?   dataStorage.node[index]->node  因此index<->node. 又index->status.所以可以得到 node->status
        // 遍历dataStorage，构建ImGui树形结构
        ////// 对于每个节点，检查其状态(选中/未选中)
        ////// 状态更新到<index,status>
        // 在右侧输出节点属性信息
        //// 维护一个集合存储选中节点:<index :status==1>
        //// 渲染节点属性信息时，遍历该数组，输出对应节点的信息,以列表的形式
        // 在3D视图渲染阶段，检查<index,status>，高亮显示选中的节点AABB
        ///// 根据status使用不同的渲染方法

        // 统计数据
        ////节点容纳三角形(叶子节点)数量
        ////树高
        ////节点总数
        ////三角形总数
        struct NodeStatus
        {
            bool isSelected = false;
            uint32_t triangleCount = 0;
            uint32_t nodeDepth = 0;
        };
        static std::unordered_map<uint32_t, NodeStatus> nodeStatusMap; // 存储节点状态
        static std::unordered_set<uint32_t> selectedNodes;             // 存储选中节点索引

        std::function<void(uint32_t)> renderNode = [&](uint32_t nodeIndex)
        {
            if (nodeIndex == sd::invalidIndex)
                return;
            const sd::Node &node = dataStorage.nodeStorage.nodes[nodeIndex];
            bool isSelected = nodeStatusMap[nodeIndex].isSelected;

            ImGuiTreeNodeFlags UINodeFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_DrawLinesToNodes;
            if (isSelected)
                UINodeFlags |= ImGuiTreeNodeFlags_Selected;

            std::string nodeLabel;
            if (node.flags == sd::NODE_LEAF)
            {
                nodeLabel = "Leaf[" + std::to_string(nodeIndex) + "] tri:" + std::to_string(node.left);
            }
            else
            {
                nodeLabel = "Node[" + std::to_string(nodeIndex) + "] L:" + std::to_string(node.left) + " R:" + std::to_string(node.right);
            }
            if (node.flags != sd::NODE_LEAF)
            {
                bool nodeOpen = ImGui::TreeNodeEx(nodeLabel.c_str(), UINodeFlags);
                if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
                {
                    if (isSelected)
                        selectedNodes.erase(nodeIndex);
                    else
                        selectedNodes.insert(nodeIndex);
                    nodeStatusMap[nodeIndex].isSelected = !isSelected; // 切换选中状态
                }
                if (nodeOpen)
                {
                    renderNode(node.left);
                    renderNode(node.right);
                    ImGui::TreePop();
                }
            }
            else
            {
                bool nodeOpen = ImGui::TreeNodeEx(nodeLabel.c_str(), UINodeFlags | ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_Bullet);
                if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
                {
                    if (isSelected)
                        selectedNodes.erase(nodeIndex);
                    else
                        selectedNodes.insert(nodeIndex);
                    nodeStatusMap[nodeIndex].isSelected = !isSelected; // 切换选中状态
                    nodeStatusMap[nodeIndex].triangleCount = 1;
                }
            }
        };
        std::function<void()> renderNodeProperties = [&]()
        {
            for (auto &nodeIndex : selectedNodes)
            {
                ImGui::Separator();
                const sd::Node &node = dataStorage.nodeStorage.nodes[nodeIndex];
                ImGui::Text("Node Index: %u", nodeIndex);
                ImGui::Text("AABB Min: (%.2f, %.2f, %.2f)", node.box.pMin.x, node.box.pMin.y, node.box.pMin.z);
                ImGui::Text("AABB Max: (%.2f, %.2f, %.2f)", node.box.pMax.x, node.box.pMax.y, node.box.pMax.z);
                ImGui::Text("Triangle Count: %u", nodeStatusMap[nodeIndex].triangleCount);
                if (node.flags == sd::NODE_LEAF)
                {
                    ImGui::Text("Type: Leaf");
                    ImGui::Text("Triangle Index: %u", node.left);
                    const sd::Triangle &tri = dataStorage.triangleStorage.triangles[node.left];
                    ImGui::Text("Vertex 0: (%.2f, %.2f, %.2f)", tri.positions[0].x, tri.positions[0].y, tri.positions[0].z);
                    ImGui::Text("Vertex 1: (%.2f, %.2f, %.2f)", tri.positions[1].x, tri.positions[1].y, tri.positions[1].z);
                    ImGui::Text("Vertex 2: (%.2f, %.2f, %.2f)", tri.positions[2].x, tri.positions[2].y, tri.positions[2].z);
                }
                else
                {
                    ImGui::Text("Type: Internal Node");
                    ImGui::Text("Left Child Index: %u", node.left);
                    ImGui::Text("Right Child Index: %u", node.right);
                }
            }
        };
        std::function<void()> renderSelectedNodeBoundingBoxes = [&]()
        {
            static std::array<uint32_t, 32> callStack; // 假设栈深度不会超过32
            static size_t top = 0;

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
                if (nodeStatusMap.find(index) == nodeStatusMap.end() || !nodeStatusMap[index].isSelected)
                {
                    // 未选中该节点，跳过
                    if (node.flags != sd::NODE_LEAF)
                    {
                        callStack[top++] = node.left;
                        callStack[top++] = node.right;
                    }
                    continue;
                }

                auto AABB = node.box;
                if (node.flags == sd::NODE_LEAF) // 叶子节点
                {
                    DebugObjectRenderer::AddDrawCall([AABB](Shader &_shaders)
                                                     { DebugObjectRenderer::DrawWireframeCube(_shaders, AABB.pMin, AABB.pMax, color4(1.0f, 0.0f, 0.0f, 1.0f)); });
                }
                else
                {
                    DebugObjectRenderer::AddDrawCall([AABB, depth](Shader &_shaders)
                                                     { DebugObjectRenderer::DrawWireframeCube(_shaders, AABB.pMin, AABB.pMax, color4(0.0f, 1.0f, depth / 8.f, 1.0f)); });
                    callStack[top++] = node.left;
                    callStack[top++] = node.right;
                }
            }
        };
        std::function<uint32_t(uint32_t)> countNodeTriangles = [&](uint32_t nodeIndex) -> uint32_t
        {
            if (nodeIndex == sd::invalidIndex)
                throw("Invalid node index in countNodeTriangles");
            const sd::Node &node = dataStorage.nodeStorage.nodes[nodeIndex];
            if (node.flags == sd::NODE_LEAF)
            {
                nodeStatusMap[nodeIndex].triangleCount = 1;
                return 1;
            }
            auto leftCount = countNodeTriangles(node.left);
            auto rightCount = countNodeTriangles(node.right);
            nodeStatusMap[nodeIndex].triangleCount = leftCount + rightCount;
            return nodeStatusMap[nodeIndex].triangleCount;
        };

        ImGui::Begin("BVH Interactive Visualization");

        renderNode(dataStorage.rootIndex);
        if (ImGui::Button("Clear Selection"))
        {
            nodeStatusMap.clear();
            selectedNodes.clear();
        }
        countNodeTriangles(dataStorage.rootIndex);
        renderNodeProperties();
        renderSelectedNodeBoundingBoxes();

        ImGui::End();
    }
};