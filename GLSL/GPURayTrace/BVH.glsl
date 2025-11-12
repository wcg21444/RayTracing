
bool IntersectBoundingBox(in BoundingBox box, in Ray ray, float tMin, float tMax)
{
    // 利用 SIMD 向量化计算，一次处理3个轴
    vec3 t0 = (box.pMin - ray.ori) * ray.invDir;
    vec3 t1 = (box.pMax - ray.ori) * ray.invDir;
    
    // 根据射线方向符号交换 t0/t1（避免分支）
    vec3 tNear = min(t0, t1);
    vec3 tFar = max(t0, t1);
    
    // 找到最大的近点和最小的远点
    float tNearMax = max(max(tNear.x, tNear.y), max(tNear.z, tMin));
    float tFarMin = min(min(tFar.x, tFar.y), min(tFar.z, tMax));
    
    // 一次比较判断是否相交
    return tNearMax <= tFarMin;
}
//TODO 改进BB求交,inout tHit.
bool IntersectBoundingBox_O1(in BoundingBox box, in Ray ray, float tMin, float tMax,inout float tHit)
{
    // 利用 SIMD 向量化计算，一次处理3个轴
    vec3 t0 = (box.pMin - ray.ori) * ray.invDir;
    vec3 t1 = (box.pMax - ray.ori) * ray.invDir;
    
    // 根据射线方向符号交换 t0/t1（避免分支）
    vec3 tNear = min(t0, t1);
    vec3 tFar = max(t0, t1);
    
    // 找到最大的近点和最小的远点
    float tNearMax = max(max(tNear.x, tNear.y), max(tNear.z, tMin));
    float tFarMin = min(min(tFar.x, tFar.y), min(tFar.z, tMax));
    
    // 一次比较判断是否相交
    if (tNearMax <= tFarMin)
    {
        tHit = tNearMax;
        return true;
    }
    return false;
}

HitInfos IntersectTriangle(in Triangle tri, in Ray ray, float tMin, float tMax)
{
    const float EPSILON = 1e-16f;
    //背面剔除
    if (dot(ray.dir, tri.normals[0]) >= 0.0f) return invalidHit;
    // 1. 减少临时变量，直接使用ray.dir和ray.ori
    vec3 edge1 = tri.positions[1] - tri.positions[0];
    vec3 edge2 = tri.positions[2] - tri.positions[0];
    vec3 h = cross(ray.dir, edge2);
    float a = dot(edge1, h);

    // 2. Early out：尽早退出减少寄存器压力
    if (abs(a) < EPSILON) return invalidHit;

    // 3. 合并计算，减少中间变量
    float f = 1.0f / a;
    vec3 s = ray.ori - tri.positions[0];
    float u = f * dot(s, h);
    // 4. 提前退出避免后续计算
    if (u < 0.0f || u > 1.0f) return invalidHit;

    vec3 q = cross(s, edge1);
    float v = f * dot(ray.dir, q);
    
    if (v < 0.0f || u + v > 1.0f) return invalidHit;

    float t = f * dot(edge2, q);
    
    if (t <= max(EPSILON, tMin) || t >= tMax) return invalidHit;

    // 5. 延迟昂贵计算到确认命中后
    vec3 pos = ray.ori + t * ray.dir;
    float w = 1.0f - u - v;
    vec3 normal = normalize(w * tri.normals[0] + u * tri.normals[1] + v * tri.normals[2]);

    // 6. 复用ray.invDir避免重复除法（如果Ray结构体有invDir字段）
    return HitInfos(
        t,
        ray.ori,
        ray.dir,
        pos,
        normal,
        tri.matFlags);
}

//TODO 改进BVH求交
//去掉入口BB求交判定
//在末尾入栈处,求出左右节点hitT,如果未命中,不入栈s
HitInfos BVHIntersectLoopTex(in sampler2D nodeSrc, in sampler2D triSrc, uint rootIndex, in Ray ray)
{
    const int stackSize = 32;
    uint[stackSize] callStack;
    int top = 0;

    HitInfos closestHit = invalidHit;
    float closestT = 1e24f; // 缓存closestHit.t，减少结构体访问

    callStack[top++] = rootIndex;

    while (top > 0)
    {
        uint index = callStack[--top];
        Node node = GetNodeFromFlatStorageTex(index, nodeSrc);

        // 1. 使用closestT而不是1e24f，提前剔除不可能更近的节点
        if (!IntersectBoundingBox(node.box, ray, 1e-6f, closestT))
            continue;

        // 2. 叶子节点处理：合并条件判断
        if (node.flags == NODE_LEAF)
        {
            Triangle tri = GetTriangleFromFlatStorageTex(node.left, triSrc);
            HitInfos hitInfos = IntersectTriangle(tri, ray, 1e-6f, closestT);
            
            // 3. 合并两次判断，减少分支
            if (hitInfos.t != invalidT && hitInfos.t < closestT)
            {
                closestHit = hitInfos;
                closestT = hitInfos.t; // 更新缓存的最近距离
            }
            continue;
        }

        // 4. 优化子节点入栈顺序：先压远子节点，后压近子节点（深度优先更快找到命中）
        // 根据射线方向判断哪个子节点更近
        Node leftNode = GetNodeFromFlatStorageTex(node.left, nodeSrc);
        Node rightNode = GetNodeFromFlatStorageTex(node.right, nodeSrc);
        
        // 计算子节点中心到射线原点的距离（简化启发式）
        vec3 leftCenter = (leftNode.box.pMin + leftNode.box.pMax) * 0.5f;
        vec3 rightCenter = (rightNode.box.pMin + rightNode.box.pMax) * 0.5f;
        float leftDist = dot(leftCenter - ray.ori, ray.dir);
        float rightDist = dot(rightCenter - ray.ori, ray.dir);
        
        // 先压远节点，后压近节点（栈后进先出，近节点先处理）
        if (leftDist < rightDist)
        {
            callStack[top++] = node.right; // 远
            callStack[top++] = node.left;  // 近
        }
        else
        {
            callStack[top++] = node.left;  // 远
            callStack[top++] = node.right; // 近
        }
    }
    
    return closestHit;
}

HitInfos BVHIntersectLoopTex_O1(in sampler2D nodeSrc, in sampler2D triSrc, uint rootIndex, in Ray ray)
{
    const int stackSize = 32;
    uint[stackSize] callStack;
    int top = 0;

    HitInfos closestHit = invalidHit;
    float closestT = 1e24f; // 缓存closestHit.t，减少结构体访问

    callStack[top++] = rootIndex;

    while (top > 0)
    {
        uint index = callStack[--top];
        Node node = GetNodeFromFlatStorageTex(index, nodeSrc);

        // if (!IntersectBoundingBox(node.box, ray, 1e-6f, closestT))
        //     continue;

        // 2. 叶子节点处理：合并条件判断
        if (node.flags == NODE_LEAF)
        {
            Triangle tri = GetTriangleFromFlatStorageTex(node.left, triSrc);
            HitInfos hitInfos = IntersectTriangle(tri, ray, 1e-6f, closestT);
            
            // 3. 合并两次判断，减少分支
            if (hitInfos.t != invalidT && hitInfos.t < closestT)
            {
                closestHit = hitInfos;
                closestT = hitInfos.t; // 更新缓存的最近距离
            }
            continue;
        }

        float hitTLeft = invalidT;
        float hitTRight = invalidT;
        IntersectBoundingBox_O1(GetBoxFromFlatStorageTex(node.left, nodeSrc), ray, 1e-4f, closestT,hitTLeft);
        IntersectBoundingBox_O1(GetBoxFromFlatStorageTex(node.right, nodeSrc), ray, 1e-4f, closestT,hitTRight);
        if( hitTLeft < hitTRight) // AABB center closer first
        {
            if (hitTRight != invalidT)
            {
                callStack[top++] = node.right; // 远
            }
            if (hitTLeft != invalidT)
            {
                callStack[top++] = node.left;  // 近
            }
        }
        else
        {
            if (hitTLeft != invalidT)
            {
                callStack[top++] = node.left;  // 远
            }
            if (hitTRight != invalidT)
            {
                callStack[top++] = node.right; // 近
            }
        }
    }
    
    return closestHit;
}