#pragma once

#include <iomanip>
#include <unordered_set>
#include "SimplifiedDataFwd.hpp"
struct BVHNode;
class Ray;
class Camera;
struct ImVec2;
class BVHDebugSettings
{
public:
    inline static bool toggleBVHAccel = true;

    static void RenderUI();
    static void RenderVisualization(BVHNode *root);
    static void RenderVisualization(const SimplifiedData::DataStorage &dataStorage);
};
void Output2DVisualization(const SimplifiedData::DataStorage &dataStorage, const std::string &filePath);
void InteractableVisualization(const SimplifiedData::DataStorage &dataStorage);

class RayVisualizer
{
private:
    using NodeIndex = uint32_t;
    using Depth = size_t;
    // ray lists
    // inline static std::vector<Ray> Rays;
    inline static std::vector<std::unique_ptr<Ray>> RayPtrs;

public:
    inline static bool HideAll = false;
    inline static size_t colorMaxDepth = 16;

public:
    static void LaunchRay(const ImVec2 &screenUV, const Camera &camera);
    static void ClearRays();

    // 具体渲染方法(UI Method)调用
    static void Render(const SimplifiedData::DataStorage &dataStorage);
    static void Render(const BVHNode *root);

private:
    static void RenderVisualization(const sd::DataStorage &dataStorage, const std::unordered_map<NodeIndex, Depth> &nodeHitMap);
};