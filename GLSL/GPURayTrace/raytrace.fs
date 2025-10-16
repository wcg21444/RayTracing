#version 460 core

#include "scene.glsl"

out vec4 FragColor;
in vec2 TexCoord;

/*****************Screen输入*****************************************************************/
uniform sampler2D lastSample;

uniform float rand;
uniform int samplesCount;

/*****************视口大小******************************************************************/
uniform int width;
uniform int height;

const int bounceLimit = 5;
vec3 viewDir;
vec2 uv;

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
vec4 castRay(in Ray ray, int traceDepth)
{
    vec4 color = vec4(0.0f);
    vec3 throughout = vec3(1.f);
    Ray tracingRay = ray;
    while (traceDepth < bounceLimit)
    {

        traceDepth++;
        // 场景测试
        HitInfos closestHit;
        closestHit.t = 1.0f / 0.0f;
        hitScene(tracingRay, closestHit);

        // 命中场景
        if (closestHit.t != 1.0f / 0.0f)
        {
            // color+= lambertianIrradiance(closestHit);
            throughout *= vec3(0.7f);
            vec3 rndDir = sampleCosineHemisphere(closestHit.normal, TexCoord * (rand + 1.f));

            tracingRay = Ray(closestHit.pos, rndDir);
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


    initializeScene();
}

void main()
{
    initialize();

    Ray ray = Ray(cam.position, viewDir);
    FragColor = (texture(lastSample, TexCoord) * float(samplesCount - 1.f) +
                 castRay(ray, 0)) /
                float(samplesCount);
}
