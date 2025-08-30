

#version 330 core
out vec4 SkyResult;
in vec3 Dir;
in vec4 FragPos;

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

// 视口大小
uniform int width;
uniform int height;

// 太阳光设置
vec3 sunlightDecay;
uniform vec3 dirLightPos;
uniform vec3 dirLightIntensity;

/*****************Camera设置******************************************************************/
uniform float nearPlane;
uniform float farPlane;
uniform vec3 eyePos;
uniform vec3 eyeFront;
uniform vec3 eyeUp;
uniform float fov;
uniform mat4 view; // 视图矩阵

/*****************************天空大气计算********************************************************** */
const float PI = 3.1415926535;
const vec4 betaRayleigh = vec4(5.8e-6, 1.35e-5, 3.31e-5, 1.0f); // 散射率(波长/RGB)
vec3 camRayDir = vec3(0.f);
vec3 camPos = vec3(0.f);
vec3 sunDir = vec3(0.f);
vec3 earthCenter;
float itvl;

#include "geometry.glsl"
#include "scatter.glsl"

vec4 computeSkyColor()
{
    vec4 skyColor;
    vec4 scatterRayleigh = vec4(0.0f);
    vec4 scatterMie = vec4(0.0f);

    vec3 camSkyIntersection = intersectSky(camPos, camRayDir); // 摄像机视线与天空交点
    itvl = length(camSkyIntersection - camPos) / float(maxStep);
    for (int i = 0; i < maxStep; ++i)
    {
        if (camSkyIntersection == vec3(0.0f))
        {
            return vec4(0.000f); // 散射点阳光被地面阻挡
        }
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

void initialize()
{
    SkyResult = vec4(0.f, 0.f, 0.f, 1.f); // Initialize Sky

    camRayDir = Dir;
    camPos = eyePos;
    earthCenter = vec3(0.0f, -earthRadius, 0.0f); // 地球球心，位于地面原点正下方
    sunDir = dirLightPos;
    sunlightDecay = computeSunlightDecay(camPos, camRayDir, sunDir);
}
vec3 dirLightDiffuse(vec3 fragPos, vec3 n)
{

    vec3 diffuse = vec3(0.0f);
    vec3 l = normalize(dirLightPos);
    float rr = dot(l, l);

    diffuse += dirLightIntensity / rr * max(0.f, dot(n, l)) * sunlightDecay;

    return diffuse;
}

void main()
{
    initialize();

    // 天空计算

    float camHeight = length(camPos - earthCenter) - earthRadius;

    vec3 camEarthIntersection = intersectEarth(camPos, camRayDir);
    if (camEarthIntersection != vec3(0.0f))
    {

        // 击中地球,渲染大气透视
        SkyResult = computeAerialPerspective(camEarthIntersection);

        vec4 t1 = transmittance(camPos, camEarthIntersection, 1.0f);

        // 渲染地面
        vec3 normal = normalize(camEarthIntersection - earthCenter);
        vec3 lighting = dirLightDiffuse(camEarthIntersection, normal);
        vec3 earthBaseColor = vec3(0.3, 0.3f, 0.34f); // 地面颜色
        SkyResult.rgb += lighting * earthBaseColor * t1.rgb;
    }
    else
    {
        if (camHeight > skyHeight)
        {
            // 摄像机在大气层外
        }
        else
        {
            SkyResult += computeSkyColor();
        }
    }
    // SkyResult.rgb = clamp(SkyResult.rgb, vec3(0.0f), vec3(1.0f));

    SkyResult.rgb = clamp(SkyResult.rgb, vec3(0.0f), vec3(1.0f));

    // SkyResult = vec4(1.0f);
}
