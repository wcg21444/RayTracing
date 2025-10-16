const uint nodesDataSize = 27u;
const uint trianglesDataSize = 50u;

const float[nodesDataSize] nodesData = float[](
0f,           0f,          -1f,      -1e-06f,    -1.55671f,           1f,     1.38598f,   -0.996302f,  1.4013e-45f,
1.4013e-45f,  1.4013e-45f,          -1f,      -1e-06f,          -1f,           1f,       1e-06f,           1f,  1.4013e-45f, 
0f,  1.4013e-45f,          -1f,      -1e-06f,    -1.55671f,           1f,     1.38598f,           1f,           0f
)
;
const float[trianglesDataSize] trianglesData = float[]( 
-1f,           0f,   -0.996303f,      0.0017f,      0.3749f,      0.9271f,           0f,           0f,           1f,           0f,          -1f,      0.0017f,      0.3749f,      0.9271f,           1f,           1f,          -1f,     1.38598f,    -1.55671f,      0.0017f,      0.3749f,      0.9271f,           0f,           1f,           0f,          -1f,           0f,   -0.996303f,          -0f,           1f,          -0f,           0f,           0f,           1f,           0f,           1f,          -0f,           1f,          -0f,           1f,           
0f,           1f,           0f,          -1f,          -0f,           1f,          -0f,           1f,           1f,           0f
);


const uint rootNodeIndex = 2u;

const uint  invalidIndex = (1u<<32u)-2u;

//  static constexpr size_t kFloatsPerTriangle = 3 * 3 /*positions*/ + 3 * 3 /*normals*/ + 3 * 2 /*uv*/ + 1 /*mat flag*/;
// static constexpr size_t kFloatsPerNode = 2 /*indices*/ + 6 /*bbox*/ + 1 /*flags*/;

const uint kFloatsPerTriangle = 3u * 3u /*positions*/ + 3u * 3u /*normals*/ + 3u * 2u /*uv*/ + 1u /*mat flag*/;//25bytes
const uint kFloatsPerNode = 2u /*indices*/ + 6u /*bbox*/ + 1u /*flags*/;


Node GetNodeFromFlatStorage( uint index,in float[nodesDataSize] src)
{

    const uint stride = kFloatsPerNode;
    uint base = index * stride;

    Node node;
    node.left = floatBitsToUint(src[base + 0u]);
    node.right = floatBitsToUint(src[base + 1u]);
    node.box.pMin = vec3(src[base + 2u], src[base + 3u], src[base + 4u]);
    node.box.pMax = vec3(src[base + 5u], src[base + 6u], src[base + 7u]);
    node.flags = floatBitsToUint(src[base + 8u]);
    return node;
}

Triangle GetTriangleFromFlatStorage( uint index,in float[trianglesDataSize] src)
{

    const uint stride = kFloatsPerTriangle;
    uint base = index * stride;

    Triangle tri;
    for (uint v = 0u; v < 3u; ++v)
    {
        uint offset = base + v * 8u;
        tri.positions[v] = vec3(src[offset + 0u], src[offset + 1u], src[offset + 2u]);
        tri.normals[v] = vec3(src[offset + 3u], src[offset + 4u], src[offset + 5u]);
        tri.texCoords[v] = vec2(src[offset + 6u], src[offset + 7u]);
    }
    tri.matFlags = floatBitsToUint(src[base + 24u]);
    return tri;
}


