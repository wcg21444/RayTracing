
#version 330 core
out vec4 FragColor;
in vec2 TexCoord;
#include "GPURayTrace/common.glsl"

#include "GPURayTrace/sceneTex.glsl"

#include "GPURayTrace/BVH.glsl"


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
uniform vec4 betaMie = vec4(21e-6, 21e-6, 21e-6, 1.0f);
uniform vec3 sunlightDir;
uniform vec4 sunlightIntensity;
uniform Camera cam;

const int bounceLimit = 10;
Sphere spheres[3];
vec3 viewDir;
vec2 uv;

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

void intersectSphere(in Ray ray, in vec3 center, in float radius, out HitInfos hitInfos)
{
    vec3 oc = center - ray.ori;
    float a = dot(ray.dir, ray.dir);
    float b = -2.0f * dot(ray.dir, oc);
    float c = dot(oc, oc) - radius * radius;
    float discriminant = b * b - 4 * a * c;
    if (discriminant < 0)
    {
        hitInfos = HitInfos(-1.f, ray.ori, ray.dir, 1 / ray.dir, vec3(0.0f), vec3(0.0f),MAT_LAMBERTIAN);
    }
    else
    {
        float t = (-b - sqrt(discriminant)) / (2.0f * a);
        if (t < 0)
        {
            hitInfos = HitInfos(-1.f, ray.ori, ray.dir, 1 / ray.dir, vec3(0.0f), vec3(0.0f),MAT_LAMBERTIAN);
        }
        vec3 N = normalize(RayAt(ray, t) - center);
        vec3 dir = ray.dir;
        hitInfos = HitInfos(t, ray.ori, ray.dir, 1 / ray.dir, RayAt(ray, t), N, MAT_LAMBERTIAN);
    }
}

// 天空大气计算入口函数
vec4 hitSky(vec3 ori, vec3 dir)
{
    vec4 skyResult = vec4(0.0f);

    skyResult += texture(skybox, dir);
    return skyResult;
    /*     vec4 skyResult = vec4(0.0f);
        float camHeight = length(ori - earthCenter) - earthRadius;

        vec3 camEarthIntersection = intersectEarth(ori, dir);
        if (camEarthIntersection != vec3(0.0f))
        {

            // 击中地球,渲染大气透视
            skyResult = computeAerialPerspective(camEarthIntersection);

            vec4 t1 = transmittance(ori, camEarthIntersection, 1.0f);

            // 渲染地面
            vec3 normal = normalize(camEarthIntersection - earthCenter);
            vec3 lighting = dirLightDiffuse(camEarthIntersection, normal);
            vec3 earthBaseColor = vec3(0.3, 0.3f, 0.34f); // 地面颜色
            skyResult.rgb += lighting * earthBaseColor * t1.rgb;
        }
        else
        {
            if (camHeight > skyHeight)
            {
                // 摄像机在大气层外
            }
            else
            {
                // skyResult += computeSkyColor(ori, dir);
                skyResult = texture(skybox, dir);
            }
        }
        // skyResult.rgb = clamp(skyResult.rgb, vec3(0.0f), vec3(1.0f));

        skyResult.rgb = clamp(skyResult.rgb, vec3(0.0f), vec3(1.0f));
        return skyResult; */
}

void hitScene(in Ray tracingRay, out HitInfos hitInfos)
{
    HitInfos closestHit;
    closestHit.t = 1.0f / 0.0f;
    for (int i = 0; i < 3; ++i)
    {
        HitInfos hitInfos;
        intersectSphere(tracingRay, spheres[i].center, spheres[i].radius, hitInfos);
        if (hitInfos.t >= 0.0f && hitInfos.t < closestHit.t)
        {
            closestHit = hitInfos;
        }
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

        traceDepth++;
        // 场景测试
        HitInfos closestHit = invalidHit;
        hitScene(tracingRay, closestHit);
        
        HitInfos closestHitBVH = invalidHit;
        // closestHitBVH = BVHIntersectLoop(nodesData,trianglesData,rootIndex, tracingRay);

        closestHitBVH = BVHIntersectLoopTex(nodeStorageTex,triangleStorageTex,rootIndex, tracingRay);

        if(closestHitBVH.t!=invalidT&&closestHitBVH.t<closestHit.t){
            closestHit = closestHitBVH;
        }

        // 命中场景
        if (closestHit.t != invalidT)
        {
            // color+= lambertianIrradiance(closestHit);
            throughout *= vec3(0.9f,0.5f,0.4f);
            vec3 rndDir = sampleCosineHemisphere(closestHit.normal, TexCoord * (rand + 1.f));
            vec3 bias = closestHit.normal*1e-3;
            tracingRay = Ray(closestHit.pos+bias, rndDir/10.f+closestHit.normal*4.f);
            // color = vec4(1.0f,0.0f,0.0f,0.0f);
            continue;
        }
        // 未命中
        color.rgb += throughout * hitSky(tracingRay.ori, tracingRay.dir).rgb;
        break;
    }
    color.rgb += generateSunDisk(tracingRay.ori, tracingRay.dir, sunDir, dirLightIntensity, 1e-3);

    return color;
}

void initializeScene()
{
    spheres[0] = Sphere(vec3(4.f, 2.f, 1.f), 1.f);
    spheres[1] = Sphere(vec3(-2.f, 3.f, 2.f), 2.f);
    spheres[2] = Sphere(vec3(4.f, -1e4f, 1.f), 1e4f);
}

void initialize()
{
    uv = TexCoord;
    // cam.focalLength = 1.0f;
    // cam.lookAtCenter = vec3(0.0f);
    // cam.position = vec3(0.0f, 7.f, -8.f);
    // cam.aspectRatio = width/float(height);

    // cam.width = 2.f;
    // cam.height =  cam.width/cam.aspectRatio;
    // viewDir = vec3(
    //     -(uv.x * cam.width - cam.width / 2.f),
    //     (uv.y * cam.height - cam.height / 2.f),
    //     cam.focalLength);
    viewDir = vec3(
        ((uv.x - 0.5f) * cam.width),
        (uv.y - 0.5f) * cam.height,
        -cam.focalLength);

    // vec3 absY = vec3(0.f, 1.f, 0.f);
    // vec3 z = normalize(cam.lookAtCenter - cam.position);
    // vec3 x = normalize(cross(absY, z));
    // vec3 y = cross(z, x);
    // mat3 rotation = mat3(x, y, z);


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

    Ray ray = Ray(cam.position, viewDir);
    FragColor = (texture(lastSample, TexCoord) * float(samplesCount - 1.f) +
                 castRay(ray, 0,sceneRootIndex)) /
                float(samplesCount);

    // int nSamples = 16;
    // for(int i=0;i<nSamples;++i) {
    //     FragColor += castRay(ray,0)/float(nSamples);
    // }

    // FragColor.rgb = getBackgroundColor(viewDir).rgb;
}
