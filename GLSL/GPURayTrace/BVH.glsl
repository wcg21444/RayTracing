
bool IntersectBoundingBox(in BoundingBox box, in Ray ray, float tMin, float tMax)
{
const float EPSILON_DIV = 1e-19f; 
    vec3 invDir = vec3(1.0f / ray.dir.x, 1.0f / ray.dir.y, 1.0f / ray.dir.z);
    vec3 origin = ray.ori;
    for (int axis = 0; axis < 3; ++axis)
    {
        float invD = invDir[axis];
        float t0 = (box.pMin[axis] - origin[axis]) * invD;
        float t1 = (box.pMax[axis] - origin[axis]) * invD;
        if (invD < 0.0f)
        {
            float tmp = t0;
            t0 = t1;
            t1 = tmp;
        }
        tMin = max(t0, tMin);
        tMax = min(t1, tMax);
        if (tMax <= tMin)
        {
            return false;
        }
    }
    return true;
}


HitInfos IntersectTriangle(in Triangle tri, in Ray ray, float tMin, float tMax)
{
    const float EPSILON = 1e-16f;
    vec3 edge1 = tri.positions[1] - tri.positions[0];
    vec3 edge2 = tri.positions[2] - tri.positions[0];
    vec3 dir = ray.dir;
    vec3 h = cross(dir, edge2);
    float a = dot(edge1, h);

    HitInfos debugHit = HitInfos(
    1.0f,
    vec3(0.0f),
    vec3(0.0f),
    vec3(0.0f),
    vec3(0.0f),
    vec3(0.0f),
    0u
    );

    if (abs(a) < EPSILON)
    {
        return invalidHit; // Ray parallel to triangle.
    }

    float f = 1.0f / a;
    vec3 s = ray.ori - tri.positions[0];
    float u = f * dot(s, h);
    if (u < 0.0f || u > 1.0f)
    {
        return invalidHit;
        // return debugHit;
    }

    vec3 q = cross(s, edge1);
    float v = f * dot(dir, q);
    if (v < 0.0f || u + v > 1.0f)
    {
        return invalidHit;
    }

    float t = f * dot(edge2, q);
    if (t <= max(EPSILON, tMin) || t >= tMax)
    {
        return invalidHit;
    }

    vec3 pos = ray.ori + t * dir;
    vec3 normal = normalize((1.0f - u - v) * tri.normals[0] + u * tri.normals[1] + v * tri.normals[2]);
    vec3 invDir = vec3(1.0f / dir.x, 1.0f / dir.y, 1.0f / dir.z);

    return HitInfos(
        t,
        ray.ori,
        dir,
        invDir,
        pos,
        normal,
        tri.matFlags);
}

// HitInfos BVHIntersectLoop(in float[nodesDataSize] nodeSrc,in float[trianglesDataSize] triSrc, uint rootIndex, in Ray ray)
// {

//     const int stackSize = 32;
//     uint[stackSize] callStack;
//     int top = 0;

//     HitInfos closestHit = invalidHit;

//     callStack[top++] = rootIndex;

//     while (top > 0)
//     {
//         //注意索引越界问题 
//         uint index = callStack[--top];

//         Node node = GetNodeFromFlatStorage(index,nodeSrc);

//         if( IntersectBoundingBox(node.box, ray, 1e-6f, 1e20f) == false)
//         {
//             continue;
//         }

//         if (node.flags == NODE_LEAF) // 叶子节点
//         {
//             // 展开求交
//             Triangle tri = GetTriangleFromFlatStorage(node.left,triSrc);
//             HitInfos hitInfos = IntersectTriangle(tri, ray, 1e-6f, closestHit.t);
//             if(hitInfos.t == invalidT){
//                 continue;
//             }
//             if (hitInfos.t < closestHit.t)
//             {
//                 closestHit = hitInfos;
//             }
//             continue;
//         }

//         callStack[top++] = node.left;
//         callStack[top++] = node.right; // node.right-> tri 不是1u 因此数组越界

//     }
//     return closestHit;
// }

HitInfos BVHIntersectLoopTex(in sampler2D nodeSrc,in sampler2D triSrc, uint rootIndex, in Ray ray)
{

    const int stackSize = 32;
    uint[stackSize] callStack;
    int top = 0;

    HitInfos closestHit = invalidHit;

    callStack[top++] = rootIndex;

    while (top > 0)
    {
        //注意索引越界问题 
        uint index = callStack[--top];

        Node node = GetNodeFromFlatStorageTex(index,nodeSrc);

        if( IntersectBoundingBox(node.box, ray, 1e-6f, 1e24f) == false)
        {
            continue;
        }

        if (node.flags == NODE_LEAF) // 叶子节点
        {
            // 展开求交
            Triangle tri = GetTriangleFromFlatStorageTex(node.left,triSrc);
            HitInfos hitInfos = IntersectTriangle(tri, ray, 1e-6f, closestHit.t);
            if(hitInfos.t == invalidT){
                continue;
            }
            if (hitInfos.t < closestHit.t)
            {
                closestHit = hitInfos;
            }
            continue;
        }

        callStack[top++] = node.left;
        callStack[top++] = node.right; // node.right-> tri 不是1u 因此数组越界

    }
    return closestHit;
}
