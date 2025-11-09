#include "BVHUI.hpp"
#include <iostream>
#include <fstream>
#include <array>
#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <functional>

#include "UICommon.hpp"
#include "DebugObjectRenderer.hpp"
#include "RenderState.hpp"
#include "SimplifiedData.hpp"
#include "Scene.hpp"
#include "Ray.hpp"

static int maxDepth = 40;
static int minDepth = 0;
static bool toggleVisualizeBVH = false;
static bool showLeafAABB = false;

void BVHDebugSettings::RenderUI()
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

void BVHDebugSettings::RenderVisualization(BVHNode *root)
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

void BVHDebugSettings::RenderVisualization(const sd::DataStorage &dataStorage)
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

void Output2DVisualization(const sd::DataStorage &dataStorage, const std::string &filePath)
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
                    << "- [" << node.box.pMax.x << "," << node.box.pMax.y << "," << node.box.pMax.z << "]"
                    << " vertices0: [" << tri.positions[0].x << "," << tri.positions[0].y << "," << tri.positions[0].z << "]  "
                    << " vertices1: [" << tri.positions[1].x << "," << tri.positions[1].y << "," << tri.positions[1].z << "]  "
                    << " vertices2: [" << tri.positions[2].x << "," << tri.positions[2].y << "," << tri.positions[2].z << "]\n";
        }
        else
        {
            outFile << " Node[" << nodeIndex << "] L:" << node.left << " R:" << node.right
                    << " [" << node.box.pMin.x << "," << node.box.pMin.y << "," << node.box.pMin.z << "]"
                    << "- [" << node.box.pMax.x << "," << node.box.pMax.y << "," << node.box.pMax.z << "]\n";

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

void InteractableVisualization(const sd::DataStorage &dataStorage)
{
    using TriCount = uint32_t;
    using Index = uint32_t;
    struct NodeStatus
    {
        bool isSelected = false;
        TriCount triangleCount = 0;
        uint32_t nodeDepth = 0;
    };
    static std::unordered_map<Index, NodeStatus> nodeStatusMap; // 存储节点状态
    static std::unordered_set<Index> selectedNodes;             // 存储选中节点索引

    std::function<void(Index)> renderNode = [&](Index nodeIndex)
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
            ImGui::Text("Node Depth: %u", nodeStatusMap[nodeIndex].nodeDepth);
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

    std::function<TriCount(Index, uint32_t)> countNodeStatus = [&](Index nodeIndex, uint32_t depth) -> TriCount
    {
        if (nodeIndex == sd::invalidIndex)
            throw("Invalid node index in countNodeStatus");
        const sd::Node &node = dataStorage.nodeStorage.nodes[nodeIndex];

        if (node.flags == sd::NODE_LEAF)
        {
            nodeStatusMap[nodeIndex].triangleCount = 1;
            nodeStatusMap[nodeIndex].nodeDepth = depth;
            return 1;
        }

        auto leftCount = countNodeStatus(node.left, depth + 1);
        auto rightCount = countNodeStatus(node.right, depth + 1);

        nodeStatusMap[nodeIndex].triangleCount = leftCount + rightCount;
        nodeStatusMap[nodeIndex].nodeDepth = depth;
        return nodeStatusMap[nodeIndex].triangleCount;
    };

    ImGui::Begin("BVH Interactive Visualization");

    renderNode(dataStorage.rootIndex);
    if (ImGui::Button("Clear Selection"))
    {
        nodeStatusMap.clear();
        selectedNodes.clear();
    }
    countNodeStatus(dataStorage.rootIndex, 0);
    renderNodeProperties();
    renderSelectedNodeBoundingBoxes();

    ImGui::End();
}

void RayVisualizer::LaunchRay(const ImVec2 &screenUV, const Camera &camera)
{
    float screenWidth = ImGui::GetIO().DisplaySize.x;
    float screenHeight = ImGui::GetIO().DisplaySize.y;
    auto normalizedUV = glm::vec2(screenUV.x / screenWidth, 1.0f - screenUV.y / screenHeight);
    std::cout << "Launching ray at UV: (" << normalizedUV.x << ", " << normalizedUV.y << ")\n";
    // Ray newRay = ...;
    Ray newRay(camera.position, camera.getRayDirection(normalizedUV));
    // RayVisualizer::Rays.push_back(newRay);
    RayVisualizer::RayPtrs.push_back(std::make_unique<Ray>(std::move(newRay)));
}

void RayVisualizer::ClearRays()
{
    RayVisualizer::RayPtrs.clear();
}

void RayVisualizer::RenderVisualization(const sd::DataStorage &dataStorage, const std::unordered_map<NodeIndex, Depth> &nodeHitMap)
{
    if (!RayVisualizer::HideAll)
    {
        for (auto &[index, depth] : nodeHitMap)
        {
            float x = float(depth) / colorMaxDepth;
            float h = (1.0f - x) * 240.0f; // 240=蓝，0=红
            float s = 1.0f, v = 1.0f;
            glm::vec3 color = hsv2rgb(glm::vec3(h, s, v)); // 需实现HSV到RGB
            const sd::Node &node = dataStorage.nodeStorage.nodes[index];
            DebugObjectRenderer::AddDrawCall([node, color](Shader &_shaders)
                                             { DebugObjectRenderer::DrawWireframeCube(_shaders, node.box.pMin, node.box.pMax, glm::vec4(color, 1.0f)); });
        }
    }
}

void UpdateRayIntersection(const sd::DataStorage &dataStorage, const Ray &ray, std::unordered_map<uint32_t, size_t> &nodeHitMap)
{

    static std::array<uint32_t, 32> callStack; // 假设栈深度不会超过32
    static size_t top = 0;

    callStack[top++] = dataStorage.rootIndex;

    while (top > 0)
    {
        uint32_t index = callStack[--top];
        const sd::Node &node = dataStorage.nodeStorage.nodes[index];

        if (index == sd::invalidIndex || !sd::IntersectBoundingBox(node.box, ray, 1e-6f, FLT_MAX))
        {
            continue;
        }
        if (node.flags == sd::NODE_LEAF) // 叶子节点
        {
            continue;
        }
        if (nodeHitMap.find(index) == nodeHitMap.end())
        {
            nodeHitMap.insert({index, top});
        }
        callStack[top++] = node.left;
        callStack[top++] = node.right;
    }
}

void RayVisualizer::Render(const SimplifiedData::DataStorage &dataStorage)
{
    static std::unordered_map<NodeIndex, Depth> nodeHitMap;
    static std::unordered_set<Ray *> rayCache;

    if (rayCache.size() < RayPtrs.size())
    {
        for (const auto &pRay : RayPtrs)
        {
            size_t hitCounts = nodeHitMap.size();
            rayCache.insert(pRay.get());
            UpdateRayIntersection(dataStorage, *pRay, nodeHitMap);
            if(nodeHitMap.size() > hitCounts)
            {
                std::cout << "New hits recorded. Total hit nodes: " << nodeHitMap.size() << std::endl;
                hitCounts = nodeHitMap.size();
            }
        }
    }

    ImGui::Begin("BVH Interactive Visualization");
    if (ImGui::Button("Clear Rays"))
    {
        rayCache.clear();
        nodeHitMap.clear();
        RayVisualizer::ClearRays();
    }
    ImGui::SliderInt("Color Max Depth", reinterpret_cast<int *>(&RayVisualizer::colorMaxDepth), 1, 32);
    ImGui::Checkbox("Hide All", &RayVisualizer::HideAll);
    ImGui::End();

    for (const auto &pRay : RayPtrs)
    {
        glm::vec3 start = pRay->getOrigin();
        glm::vec3 end = pRay->at(1000.0f); // 假设绘制长度为1000单位的射线
        DebugObjectRenderer::AddDrawCall([&dataStorage, start, end](Shader &_shaders)
                                         {
                                             DebugObjectRenderer::DrawLine(_shaders, start, end, glm::vec4(1.0f, 1.0f, 0.0f, 1.0f), 0.01f); // 用Cube 模拟射线
                                         });
    }
    RenderVisualization(dataStorage, nodeHitMap); // 不需要每帧遍历的其实. 只需要在发射新射线时遍历一次,对节点标记,然后逐帧渲染就行了
}

void RayVisualizer::Render(const BVHNode *root)
{
    DebugObjectRenderer::AddDrawCall([](Shader &_shaders)
                                     {
        ImGui::Begin("BVH Interactive Visualization");
        if (ImGui::Button("Clear Rays"))
        {
            RayVisualizer::ClearRays();
        }
        ImGui::SliderInt("Color Max Depth", reinterpret_cast<int*>(&RayVisualizer::colorMaxDepth), 1, 32);
        ImGui::Checkbox("Hide All", &RayVisualizer::HideAll);
        ImGui::End();

        for (const auto &pRay : RayPtrs)
        {
            glm::vec3 start = pRay->getOrigin();
            glm::vec3 end = pRay->at(1000.0f); // 假设绘制长度为1000单位的射线

            DebugObjectRenderer::DrawLine(_shaders, start, end, glm::vec4(1.0f, 1.0f, 0.0f, 1.0f),0.01f); // 用Cube 模拟射线

            //TODO
        } });
}
