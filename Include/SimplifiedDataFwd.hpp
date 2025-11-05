#pragma once

#pragma once

#include "Materials.hpp"
#include "Ray.hpp"
#include "Utils.hpp"

#include <optional>
#include <vector>
#include <array>
#include <memory>
#include <stdexcept>
#include <stack>
#include <algorithm>
namespace SimplifiedData
{
    struct Node;
    struct BoundingBox;
    struct Triangle;
    struct HitInfos;
    class TriangleStorage;
    class NodeStorage;
    class Mesh;
    struct DataStorage;
    class BVH;
    enum NodeFlags : uint8_t;
    struct Vertex;
    enum Matierals;
    struct FlatNodeStorage;
    struct FlatTriangleStorage;

}
namespace sd = SimplifiedData;