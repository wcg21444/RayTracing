
// /*****************Scene输入******************************************************************/
// uniform sampler2D nodeStorageTex;
// uniform sampler2D triangleStorageTex;
// uniform uint sceneRootIndex;

const uint  invalidIndex = (1u<<32u)-2u;
const uint kFloatsPerTriangle = 3u * 3u /*positions*/ + 3u * 3u /*normals*/ + 3u * 2u /*uv*/ + 1u /*mat flag*/;//25bytes
const uint kFloatsPerNode = 2u /*indices*/ + 6u /*bbox*/ + 1u /*flags*/;


ivec2 IndexToTexCoord(uint flatIndex, int texWidth)
{
    // 检查 texWidth 是否为 0 或 1，以避免除零或简化运算，
    // 实际项目中应确保 texWidth > 0
    if (texWidth <= 0) {
        // 返回一个无效/安全值，虽然通常不会发生
        return ivec2(0, 0); 
    }
    
    int index_int = int(flatIndex); 
    
    // X = 索引 % 宽度 (获取列索引)
    int x = index_int % texWidth; 
    
    // Y = 索引 / 宽度 (获取行索引)
    int y = index_int / texWidth; 
    
    return ivec2(x, y);
}
Node GetNodeFromFlatStorageTex(uint index, in sampler2D src)
{
    int texWidth = textureSize(src, 0).x;
    const uint stride = kFloatsPerNode; // 假设 kFloatsPerNode = 9
    uint base = index * stride;

    Node node;
    
    // Y 坐标固定为 0，坐标转换直接使用 IndexToTexCoord(flatIndex)

    // 1. node.left (base + 0u)
    node.left = floatBitsToUint(texelFetch(src, IndexToTexCoord(base + 0u,texWidth), 0).r);

    // 2. node.right (base + 1u,texWidth)
    node.right = floatBitsToUint(texelFetch(src, IndexToTexCoord(base + 1u,texWidth), 0).r);
    
    // 3. node.box.pMin (base + 2u, 3u, 4u,texWidth)
    node.box.pMin = vec3(
        texelFetch(src, IndexToTexCoord(base + 2u,texWidth), 0).r,
        texelFetch(src, IndexToTexCoord(base + 3u,texWidth), 0).r,
        texelFetch(src, IndexToTexCoord(base + 4u,texWidth), 0).r
    );
    
    // 4. node.box.pMax (base + 5u, 6u, 7u,texWidth)
    node.box.pMax = vec3(
        texelFetch(src, IndexToTexCoord(base + 5u,texWidth), 0).r,
        texelFetch(src, IndexToTexCoord(base + 6u,texWidth), 0).r,
        texelFetch(src, IndexToTexCoord(base + 7u,texWidth), 0).r
    );
    
    // 5. node.flags (base + 8u)
    node.flags = floatBitsToUint(texelFetch(src, IndexToTexCoord(base + 8u,texWidth), 0).r);

    return node;
}

Triangle GetTriangleFromFlatStorageTex(uint index, in sampler2D src)
{
    int texWidth = textureSize(src, 0).x;
    const uint stride = kFloatsPerTriangle; // 假设 kFloatsPerTriangle = 25
    uint base = index * stride;

    Triangle tri;

    for (uint v = 0u; v < 3u; ++v)
    {
        uint offset = base + v * 8u; // 8 floats per vertex

        // 1. 位置 (positions[v]): offset + 0u, 1u, 2u
        tri.positions[v] = vec3(
            texelFetch(src, IndexToTexCoord(offset + 0u, texWidth), 0).r,
            texelFetch(src, IndexToTexCoord(offset + 1u, texWidth), 0).r,
            texelFetch(src, IndexToTexCoord(offset + 2u, texWidth), 0).r
        );
        
        // 2. 法线 (normals[v]): offset + 3u, 4u, 5u
        tri.normals[v] = vec3(
            texelFetch(src, IndexToTexCoord(offset + 3u,texWidth), 0).r,
            texelFetch(src, IndexToTexCoord(offset + 4u,texWidth), 0).r,
            texelFetch(src, IndexToTexCoord(offset + 5u,texWidth), 0).r
        );
        
        // 3. 纹理坐标 (texCoords[v]): offset + 6u, 7u
        tri.texCoords[v] = vec2(
            texelFetch(src, IndexToTexCoord(offset + 6u,texWidth), 0).r,
            texelFetch(src, IndexToTexCoord(offset + 7u,texWidth), 0).r
        );
    }
    
    // 4. 材质标志 (matFlags): base + 24u
    tri.matFlags = floatBitsToUint(texelFetch(src, IndexToTexCoord(base + 24u,texWidth), 0).r);
    
    return tri;
}