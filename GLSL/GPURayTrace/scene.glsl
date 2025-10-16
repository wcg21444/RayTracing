
// layout(binding = ?,std430) readonly buffer NodeStorage
// {
//     uint nexts[];//left ,right , flags 3uint
//     float AABBs[];//vec3 pMin,vec3 pMax  6float
// };
// layout(binding = ?,std430) readonly buffer TriangleStorage
// {
//     float positions[];//vec3[3]
//     float normals[];  //vec3[3]
//     float texCoords[];//vec2[3],float matFlags;
// };
layout(binding = 0,std430) readonly buffer NodeStorage
{
 float nodeData[];//pMin,pMax,left,right,flags
};

layout(binding = 1,std430) readonly buffer TriangleStorage {
    float triangleData[]; // 每三角形 25 个 float 扁平化
};

uniform uint rootNodeIndex;

struct Triangle
{
    vec3 positions[3];
    vec3 normals[3];
    vec2 texCoords[3];
    uint matFlags;
};
struct BoundingBox
{
    vec3 pMin = vec3(1.0f/0.0f);
    vec3 pMax = vec3(-1.0f/0.0f);
};
struct Node
{
    uint32_t left;
    uint32_t right;
    BoundingBox box;
    uint flags; // 节点? 叶子节点? Mesh节点? 三角形节点?...
};

// inline Node GetNodeFromFlatStorage( size_t index)
// {
//     const auto &src = flatNodeStorage.nodes;
//     constexpr size_t stride = FlatNodeStorage::kFloatsPerNode;
//     size_t base = index * stride;

//     Node node;
//     node.left = static_cast<uint32_t>(src[base + 0]);
//     node.right = static_cast<uint32_t>(src[base + 1]);
//     node.box.pMin = vec3(src[base + 2], src[base + 3], src[base + 4]);
//     node.box.pMax = vec3(src[base + 5], src[base + 6], src[base + 7]);
//     node.flags = static_cast<uint8_t>(src[base + 8]);
//     return node;
// }

// inline Triangle GetTriangleFromFlatStorage( size_t index)
// {
//     const auto &src = flatTriangleStorage.triangles;
//     constexpr size_t stride = FlatTriangleStorage::kFloatsPerTriangle;
//     size_t base = index * stride;

//     Triangle tri;
//     for (int v = 0; v < 3; ++v)
//     {
//         size_t offset = base + v * 8;
//         tri.positions[v] = vec3(src[offset + 0], src[offset + 1], src[offset + 2]);
//         tri.normals[v] = vec3(src[offset + 3], src[offset + 4], src[offset + 5]);
//         tri.texCoords[v] = vec2(src[offset + 6], src[offset + 7]);
//     }
//     tri.matFlags = static_cast<uint16_t>(src[base + 24]);
//     return tri;
// }
