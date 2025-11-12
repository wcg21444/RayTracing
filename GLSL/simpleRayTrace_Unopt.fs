

#version 330 core

#include "GPURayTrace/common.glsl"

#include "GPURayTrace/sceneTex.glsl"

#include "GPURayTrace/BVH.glsl"


out vec4 FragColor;
in vec2 TexCoord;



/*****************Scene输入******************************************************************/
uniform sampler2D nodeStorageTex;
uniform sampler2D triangleStorageTex;
uniform uint sceneRootIndex;

/*****************Screen输入*****************************************************************/
uniform sampler2D lastSample;

uniform float rand;
uniform int samplesCount;

/*****************视口大小******************************************************************/
uniform int width;
uniform int height;

/*****************天空输入******************************************************************/
uniform int maxStep;
uniform float atmosphereDensity; // 大气密度
uniform float MieDensity;
uniform float gMie;
uniform float absorbMie;
uniform float MieIntensity;
uniform float skyHeight;
uniform float earthRadius;
uniform float skyIntensity;
uniform float HRayleigh;
uniform float HMie;
uniform vec4 betaMie;
uniform vec3 sunlightDir;
uniform vec4 sunlightIntensity;
uniform vec4 sunlightColor;
uniform Camera cam;

const int bounceLimit =20;
Sphere spheres[3];
vec3 viewDir;
vec2 uv;

// 面光源定义
struct AreaLight {
    vec3 corner;       // 矩形左下角
    vec3 edgeU;        // U方向边缘向量
    vec3 edgeV;        // V方向边缘向量
    vec3 emission;     // 发光强度
    vec3 normal;       // 法线（自动计算）
};

// 太阳圆盘定义（方向光+角度直径）
struct SunDisk {
    vec3 direction;    // 太阳方向（归一化）
    float angularRadius; // 角半径（弧度），真实太阳约0.53度 = 0.00925弧度
    vec3 emission;     // 太阳辐射强度
};

AreaLight areaLight;
SunDisk sunDisk;

/*****************************天空大气计算********************************************************** */
uniform samplerCube skybox;

const float PI = 3.1415926535;
const vec4 betaRayleigh = vec4(5.8e-6, 1.35e-5, 3.31e-5, 1.0f); // 散射率(波长/RGB)
vec3 camRayDir = vec3(0.f);
vec3 camPos = vec3(0.f);
vec3 sunDir = vec3(0.f);
vec3 earthCenter;
float itvl;

// 太阳光设置
vec3 sunlightDecay;
vec3 dirLightPos = vec3(2.f, 1.f, 4.f);
vec3 dirLightIntensity = vec3(1.f, 1.f, 1.f);

#include "geometry.glsl"
#include "scatter.glsl"

vec4 computeSkyColor(vec3 ori, vec3 dir)
{
    vec4 skyColor;
    vec4 scatterRayleigh = vec4(0.0f);
    vec4 scatterMie = vec4(0.0f);

    vec3 camSkyIntersection = intersectSky(ori, dir); // 摄像机视线与天空交点
    itvl = length(camSkyIntersection - ori) / float(maxStep);
    for (int i = 0; i < maxStep; ++i)
    {
        if (camSkyIntersection == vec3(0.0f))
        {
            return vec4(0.000f); // 散射点阳光被地面阻挡
        }
        vec3 scatterPoint = ori + i * itvl * dir;
        vec3 scatterSkyIntersection = intersectSky(scatterPoint, sunDir);     // 散射点与天空交点
        vec3 scatterEarthIntersection = intersectEarth(scatterPoint, sunDir); // 散射点与地面交点
        if (scatterEarthIntersection != vec3(0.0f) && length(scatterEarthIntersection - scatterPoint) < length(scatterSkyIntersection - scatterPoint))
        {
            continue; // 散射点阳光被地面阻挡
        }
        vec4 t1 = transmittance(ori, scatterPoint, 1.0f);                    // 摄像机到散射点的透射率
        vec4 t2 = transmittance(scatterPoint, scatterSkyIntersection, 1.0f); // 散射点到天空边界的透射率

        scatterRayleigh += scatterCoefficientRayleigh(scatterPoint) * t1 * t2;

        scatterMie += scatterCoefficientMie(scatterPoint) * t1 * t2;
    }

    scatterRayleigh *= phaseRayleigh(dir, sunDir);

    scatterMie *= phaseMie(dir, sunDir);

    skyColor += scatterRayleigh;
    skyColor += scatterMie * MieIntensity;
    // skyColor.rgb = skyTonemap(skyColor.rgb);

    return vec4(dirLightIntensity, 1.0f) * skyColor * skyIntensity * itvl;
}

vec4 computeAerialPerspective(vec3 camEarthIntersection)
{
    vec4 aerialColor;
    vec4 scatterRayleigh = vec4(0.0f);
    vec4 scatterMie = vec4(0.0f);

    itvl = length(camEarthIntersection - camPos) / float(maxStep);
    for (int i = 0; i < maxStep; ++i)
    {
        vec3 scatterPoint = camPos + i * itvl * camRayDir;
        vec3 scatterSkyIntersection = intersectSky(scatterPoint, sunDir);     // 散射点与天空交点
        vec3 scatterEarthIntersection = intersectEarth(scatterPoint, sunDir); // 散射点与地面交点
        if (scatterEarthIntersection != vec3(0.0f) && length(scatterEarthIntersection - scatterPoint) < length(scatterSkyIntersection - scatterPoint))
        {
            continue; // 散射点阳光被地面阻挡
        }
        vec4 t1 = transmittance(camPos, scatterPoint, 1.0f);                 // 摄像机到散射点的透射率
        vec4 t2 = transmittance(scatterPoint, scatterSkyIntersection, 1.0f); // 散射点到天空边界的透射率

        scatterRayleigh += scatterCoefficientRayleigh(scatterPoint) * t1 * t2;

        scatterMie += scatterCoefficientMie(scatterPoint) * t1 * t2;
    }

    scatterRayleigh *= phaseRayleigh(camRayDir, sunDir);

    scatterMie *= phaseMie(camRayDir, sunDir);

    aerialColor += scatterRayleigh;
    aerialColor += scatterMie * MieIntensity;
    // aerialColor.rgb = skyTonemap(aerialColor.rgb);

    return vec4(dirLightIntensity, 1.0f) * aerialColor * itvl;
}

vec3 computeSunlightDecay(vec3 camPos, vec3 fragDir, vec3 sunDir)
{
    vec3 skyIntersection = intersectSky(camPos, sunDir);
    vec4 t1 = transmittance(camPos, skyIntersection, 1.0f); // 散射点到摄像机的透射率   决定天顶-地平线透射率差异

    return t1.rgb;
}

vec3 generateSunDisk(vec3 camPos, vec3 fragDir, vec3 sunDir, vec3 sunIntensity, float sunSize)
{
    // 计算太阳方向和片段方向之间的余弦值
    float exponent = 1e2; // 锐利程度
    float sunSizeInner = 1.f - 1e-6;
    float sunSizeOuter = 1.f - 1e-3;

    float sunDot = dot(normalize(fragDir), normalize(sunDir));
    float sunSmoothstep = smoothstep(sunSizeOuter, sunSizeInner, sunDot);

    // 返回太阳亮度，与透射率相乘
    return sunIntensity * 1e4 * pow(sunSmoothstep, exponent) * pow(sunlightDecay, vec3(2.f));
}

vec3 dirLightDiffuse(vec3 fragPos, vec3 n)
{

    vec3 diffuse = vec3(0.0f);
    vec3 l = normalize(sunDir);
    float rr = dot(l, l);

    diffuse += dirLightIntensity / rr * max(0.f, dot(n, l)) * sunlightDecay;

    return diffuse;
}

//生成0-1之间的随机数
float random(vec2 st)
{
    return fract(sin(dot(st.xy, vec2(12.9898, 78.233))) * 43758.5453123);
}

vec3 sampleCosineHemisphere(vec3 normal, vec2 seed)
{
    // 1. 在一个单位圆盘内生成均匀随机点
    float r1 = 2.0 * PI * random(seed);
    float r2 = random(seed + vec2(1.0, 0.0));
    float r = sqrt(r2);

    float x = r * cos(r1);
    float y = r * sin(r1);
    float z = sqrt(1.0 - r2);

    // 2. 构建一个局部正交基
    // 这个基需要一个与 normal 垂直的向量。我们可以通过取叉积来实现。
    vec3 w = normal;
    vec3 u = normalize(cross(abs(w.y) > 0.9 ? vec3(1, 0, 0) : vec3(0, 1, 0), w));
    vec3 v = cross(w, u);

    // 3. 将随机生成的局部向量转换回世界空间
    // 这里的 (x, y, z) 是在局部空间下的余弦分布向量
    return normalize(u * x + v * y + w * z);
}

vec3 RayAt(in Ray ray, in float t)
{
    return ray.ori + t * ray.dir;
}

// 面光源求交
bool intersectAreaLight(in Ray ray, in AreaLight light, out float t, out vec2 uv) {
    // 射线-平面求交
    float denom = dot(light.normal, ray.dir);
    if (abs(denom) < 1e-6) return false; // 平行
    
    float tHit = dot(light.corner - ray.ori, light.normal) / denom;
    if (tHit < 1e-6) return false; // 在射线后方
    
    vec3 hitPos = ray.ori + tHit * ray.dir;
    vec3 localPos = hitPos - light.corner;
    
    // 计算UV坐标
    float u = dot(localPos, light.edgeU) / dot(light.edgeU, light.edgeU);
    float v = dot(localPos, light.edgeV) / dot(light.edgeV, light.edgeV);
    
    // 检查是否在矩形内
    if (u < 0.0 || u > 1.0 || v < 0.0 || v > 1.0) return false;
    
    t = tHit;
    uv = vec2(u, v);
    return true;
}

// 在面光源上均匀采样一个点
vec3 sampleAreaLight(in AreaLight light, vec2 seed, out vec3 lightPos, out vec3 lightNormal, out float pdf) {
    float u = random(seed);
    float v = random(seed + vec2(0.5, 0.3));
    
    lightPos = light.corner + u * light.edgeU + v * light.edgeV;
    lightNormal = light.normal;
    
    // PDF = 1 / 面积
    float area = length(cross(light.edgeU, light.edgeV));
    pdf = 1.0 / area;
    
    return light.emission;
}

// 在太阳圆盘上均匀采样一个方向
vec3 sampleSunDisk(in SunDisk sun, vec2 seed, out vec3 sunDir, out float pdf) {
    // 在圆锥内均匀采样
    float r1 = random(seed);
    float r2 = random(seed + vec2(0.7, 0.1));
    
    // 均匀采样圆锥立体角
    float cosTheta = 1.0 - r1 * (1.0 - cos(sun.angularRadius));
    float sinTheta = sqrt(1.0 - cosTheta * cosTheta);
    float phi = 2.0 * PI * r2;
    
    // 构建局部坐标系（太阳方向为Z轴）
    vec3 w = sun.direction;
    vec3 u = normalize(cross(abs(w.y) > 0.9 ? vec3(1, 0, 0) : vec3(0, 1, 0), w));
    vec3 v = cross(w, u);
    
    // 转换到世界空间
    sunDir = normalize(u * (sinTheta * cos(phi)) + v * (sinTheta * sin(phi)) + w * cosTheta);
    
    // PDF = 1 / 立体角 = 1 / (2π(1 - cosθ))
    float solidAngle = 2.0 * PI * (1.0 - cos(sun.angularRadius));
    pdf = 1.0 / solidAngle;
    
    return sun.emission;
}

// 检查射线是否击中太阳圆盘
bool intersectSunDisk(in Ray ray, in SunDisk sun, out float t) {
    // 太阳在无穷远，检查方向是否在圆锥内
    float cosAngle = dot(normalize(ray.dir), sun.direction);
    if (cosAngle > cos(sun.angularRadius)) {
        t = 1e10; // 无穷远
        return true;
    }
    return false;
}

void intersectSphere(in Ray ray, in vec3 center, in float radius, out HitInfos hitInfos)
{
    vec3 oc = center - ray.ori;
    float a = dot(ray.dir, ray.dir);
    float b = -2.0f * dot(ray.dir, oc);
    float c = dot(oc, oc) - radius * radius;
    float discriminant = b * b - 4 * a * c;
    if (discriminant < 0)
    {
        hitInfos = HitInfos(-1.f, ray.ori, ray.dir, vec3(0.0f), vec3(0.0f),MAT_LAMBERTIAN);
    }
    else
    {
        float t = (-b - sqrt(discriminant)) / (2.0f * a);
        if (t < 0)
        {
            hitInfos = HitInfos(-1.f, ray.ori, ray.dir, vec3(0.0f), vec3(0.0f),MAT_LAMBERTIAN);
        }
        vec3 N = normalize(RayAt(ray, t) - center);
        vec3 dir = ray.dir;
        hitInfos = HitInfos(t, ray.ori, ray.dir, RayAt(ray, t), N, MAT_LAMBERTIAN);
    }
}

vec4 skyColor(vec3 ori, vec3 dir)
{
    vec3 unit_direction = normalize(dir);
    float a = 0.5f * (unit_direction.y + 1.0f);
    return vec4((1.0f - a) * vec3(1.0f, 1.0f, 1.0f) + a * vec3(0.5f, 0.7f, 1.0f), 1.0f) / 1.f;

}

// 天空大气计算入口函数
vec4 hitSky(vec3 ori, vec3 dir)
{
    vec4 skyResult = vec4(0.0f);
    skyResult += texture(skybox, dir);
    return skyResult;
}


void hitScene(in Ray tracingRay, out HitInfos hitInfos)
{
    HitInfos closestHit;
    closestHit.t = 1.0f / 0.0f;
    
    // 测试球体
    // for (int i = 0; i < 3; ++i)
    // {
    //     HitInfos hitInfos;
    //     intersectSphere(tracingRay, spheres[i].center, spheres[i].radius, hitInfos);
    //     if (hitInfos.t >= 0.0f && hitInfos.t < closestHit.t)
    //     {
    //         closestHit = hitInfos;
    //     }
    // }
    
    // 测试面光源（作为几何体）
    float tLight;
    vec2 uvLight;
    if (intersectAreaLight(tracingRay, areaLight, tLight, uvLight) && tLight < closestHit.t) {
        closestHit.t = tLight;
        closestHit.ori = tracingRay.ori;
        closestHit.dir = tracingRay.dir;
        closestHit.pos = tracingRay.ori + tLight * tracingRay.dir;
        closestHit.normal = areaLight.normal;
        closestHit.matFlags = MAT_LAMBERTIAN; // 标记为发光材质
    }
    
    hitInfos = closestHit;
}

// 光线入口函数
vec4 castRay(in Ray ray, int traceDepth,uint rootIndex)
{
    vec4 color = vec4(0.0f);
    vec3 throughout = vec3(1.f);
    Ray tracingRay = ray;
    while (traceDepth < bounceLimit)
    {
        if (traceDepth > 2) {
            float pSurvive = clamp(max(throughout.r, max(throughout.g, throughout.b)), 0.1, 0.95);
            float RR = random(TexCoord + vec2(float(traceDepth) + rand, float(traceDepth) - rand));
            if (RR > pSurvive) {
                break;
            }
            throughout /= pSurvive;
        }
        
        traceDepth++;
        // 场景测试
        HitInfos closestHit = invalidHit;
        hitScene(tracingRay, closestHit);
        
        HitInfos closestHitBVH = invalidHit;
        closestHitBVH = BVHIntersectLoopTex(nodeStorageTex,triangleStorageTex,rootIndex, tracingRay);

        if(closestHitBVH.t!=invalidT&&closestHitBVH.t<closestHit.t){
            closestHit = closestHitBVH;
        }

        // 命中场景
        if (closestHit.t != invalidT)
        {
            // 检查是否命中面光源
            float tLight;
            vec2 uvLight;
            bool hitLight = intersectAreaLight(tracingRay, areaLight, tLight, uvLight);
            if (hitLight && abs(tLight - closestHit.t) < 1e-4) {
                // 直接命中光源，返回发光
                color.rgb += throughout * areaLight.emission;
                break;
            }
            
            // 普通表面：漫反射
            throughout *= vec3(1.0f, 0.5f, 0.6f);
            
            // NEE策略：每次弹射都尝试直接连接太阳（降低方差）
            vec3 sunSampleDir;
            float sunPdf;
            vec3 sunEmission = sampleSunDisk(sunDisk, TexCoord * (rand + float(traceDepth) * 1.3), sunSampleDir, sunPdf);
            
            float cosTheta = max(0.0, dot(closestHit.normal, sunSampleDir));
            if (cosTheta > 0.0) {
                Ray shadowRay = Ray(closestHit.pos + closestHit.normal * 1e-4, sunSampleDir, 1.0 / sunSampleDir);
                HitInfos shadowHit = invalidHit;
                hitScene(shadowRay, shadowHit);
                
                HitInfos shadowHitBVH = BVHIntersectLoopTex(nodeStorageTex, triangleStorageTex, rootIndex, shadowRay);
                if (shadowHitBVH.t != invalidT && shadowHitBVH.t < shadowHit.t) {
                    shadowHit = shadowHitBVH;
                }
                
                if (shadowHit.t == invalidT) {
                    color.rgb += throughout * sunEmission * cosTheta  / sunPdf;
                }
            }
            
            // 随机选择继续路径
            float rndChoice = random(TexCoord * (rand + float(traceDepth) * 0.7));
            
            if (rndChoice < 0.3) {
                // 30%概率采样面光源
                vec3 lightPos, lightNormal;
                float lightPdf;
                vec3 lightEmission = sampleAreaLight(areaLight, TexCoord * (rand + float(traceDepth)), lightPos, lightNormal, lightPdf);
                
                vec3 toLight = lightPos - closestHit.pos;
                float distToLight = length(toLight);
                vec3 lightDir = toLight / distToLight;
                
                float cosTheta2 = max(0.0, dot(closestHit.normal, lightDir));
                float cosLightTheta = max(0.0, dot(lightNormal, -lightDir));
                
                if (cosTheta2 > 0.0 && cosLightTheta > 0.0) {
                    Ray shadowRay2 = Ray(closestHit.pos + closestHit.normal * 1e-4, lightDir, 1.0 / lightDir);
                    HitInfos shadowHit2 = invalidHit;
                    hitScene(shadowRay2, shadowHit2);
                    
                    HitInfos shadowHitBVH2 = BVHIntersectLoopTex(nodeStorageTex, triangleStorageTex, rootIndex, shadowRay2);
                    if (shadowHitBVH2.t != invalidT && shadowHitBVH2.t < shadowHit2.t) {
                        shadowHit2 = shadowHitBVH2;
                    }
                    
                    if (shadowHit2.t == invalidT || shadowHit2.t > distToLight - 1e-3) {
                        float geometryTerm = cosTheta2 * cosLightTheta / (distToLight * distToLight);
                        color.rgb += (throughout * lightEmission * geometryTerm / lightPdf) / 0.3;
                    }
                }
                break; // 面光源采样后终止
            }
            else {
                // 70%概率BRDF采样继续路径（捕捉间接光）
                vec3 rndDir = sampleCosineHemisphere(closestHit.normal, TexCoord * (rand + float(traceDepth) + 1.0));
                vec3 bias = closestHit.normal * 1e-4;
                tracingRay = Ray(closestHit.pos + bias, rndDir, 1.0 / rndDir);
                continue;
            }
        }

        // 未命中
        float tSun;
        if (intersectSunDisk(tracingRay, sunDisk, tSun)) {
            // 直接看到太阳
            color.rgb += throughout * sunDisk.emission;
        } else {
            // 击中天空盒
            color.rgb += throughout * hitSky(tracingRay.ori, tracingRay.dir).rgb;
        }
        break;
    }

    return color;
}

void initializeScene()
{
    spheres[0] = Sphere(vec3(4.f, 2.f, 1.f), 1.f);
    spheres[1] = Sphere(vec3(-2.f, 3.f, 2.f), 2.f);
    spheres[2] = Sphere(vec3(4.f, -1e4f, 1.f), 1e4f);
    
    // 初始化面光源（天花板上的矩形光源）
    areaLight.corner = vec3(-10.0, 15.0, -2.0);      // 左下角
    areaLight.edgeU = vec3(4.0, 0.0, 0.0);         // U方向：4米宽
    areaLight.edgeV = vec3(0.0, 0.0, 4.0);         // V方向：4米深
    areaLight.emission = vec3(0.0, 1.0, 0.0);   // 白光，强度10
    areaLight.normal = normalize(cross(areaLight.edgeU, areaLight.edgeV)); // 自动计算法线
    
    // 初始化太阳圆盘
    sunDisk.direction = normalize(sunlightDir);
    sunDisk.angularRadius = 0.04; // 增大角半径（约1.15度）降低采样难度，减少噪点
    sunDisk.emission = vec3(sunlightColor)*pow(sunlightDecay, vec3(1.5f)) * 50.f; // 降低强度配合70%采样权重
}

void initialize()
{
    uv = TexCoord;

    viewDir = vec3(
        ((uv.x - 0.5f) * cam.width),
        (uv.y - 0.5f) * cam.height,
        -cam.focalLength);

    viewDir = inverse(mat3(cam.view))*normalize(viewDir);
    camRayDir = viewDir;
    camPos = cam.position;
    earthCenter = vec3(0.0f, -earthRadius, 0.0f); // 地球球心，位于地面原点正下方
    sunDir = sunlightDir;
    sunlightDecay = computeSunlightDecay(camPos, camRayDir, sunDir);

    // viewDir = rotation * (viewDir);

    initializeScene();
}

void main()
{
    initialize();

    Ray ray = Ray(cam.position, viewDir,1.0f/viewDir);
    FragColor = (texture(lastSample, TexCoord) * float(samplesCount - 1.f) +
                 castRay(ray, 0,sceneRootIndex)) /
                float(samplesCount);
}
