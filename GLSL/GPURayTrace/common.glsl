struct Ray
{
    vec3 ori;
    vec3 dir;
};

struct Camera
{
    float focalLength;
    vec3 position;
    vec3 lookAtCenter;
    float width;
    float height;
    float aspectRatio;
    mat4 view;
    mat4 projection;
};

struct Sphere
{
    vec3 center;
    float radius;
};

struct HitInfos
{
    float t; // 命中时光线的t
    vec3 origin;                                 // 光线起点
    vec3 dir;                                    // 命中光线的dir
    vec3 invDir;                                 // 光线dir倒数
    vec3 pos;                                    // 命中位置
    vec3 normal;                                 // 归一化世界法线
    uint matFlags;                               // 材质
};

const uint MAT_LAMBERTIAN = 1u << 0;
const uint MAT_METAL      = 1u << 1;
const uint MAT_DIELECTRIC = 1u << 2;
const uint MAT_LIGHT      = 1u << 3;

const uint   NODE_INTERNAL = 0u;
const uint   NODE_LEAF = 1u;
const uint   NODE_MESH = 2u;
const uint   NODE_TRIANGLE = 3u;

const HitInfos invalidHit = HitInfos(
    1.0f/0.0f,
    vec3(0.0f),
    vec3(0.0f),
    vec3(0.0f),
    vec3(0.0f),
    vec3(0.0f),
    0u
);

const HitInfos debugHit =  HitInfos(
    0.1f,
    vec3(1.0f),
    vec3(1.1f),
    vec3(1.1f),
    vec3(1.1f),
    vec3(1.1f),
    0u
);

const float invalidT = 1.0f/0.0f;

struct Triangle
{
    vec3 positions[3];
    vec3 normals[3];
    vec2 texCoords[3];
    uint matFlags;
};
struct BoundingBox
{
    vec3 pMin ;
    vec3 pMax ;
};
struct Node
{
    uint left;
    uint right;
    BoundingBox box;
    uint flags; // 节点? 叶子节点? Mesh节点? 三角形节点?...
};
