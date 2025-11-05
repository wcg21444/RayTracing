#pragma once

#include <iomanip>
#include <unordered_set>
#include "SimplifiedDataFwd.hpp"
struct BVHNode;
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