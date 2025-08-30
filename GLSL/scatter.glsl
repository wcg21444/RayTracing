
float phaseRayleigh(vec3 _camRayDir, vec3 _sunDir)
{
    float cosine = dot(_camRayDir, _sunDir) / length(_camRayDir) / length(_sunDir);
    return 3.0f / 16.0f * PI * (1 + cosine * cosine);
}
float phaseMie(vec3 _camRayDir, vec3 _sunDir)
{
    float gMie2 = gMie * gMie;
    float cosine = dot(_camRayDir, _sunDir) / length(_camRayDir) / length(_sunDir);
    return (1.0 - gMie2) / pow(1.0 + gMie2 - 2.0 * gMie * cosine, 1.5);

    // return 3.0f / 8.0f * PI * (1.0 - gMie2) * (1 + cosine * cosine) / (2 + gMie2) / pow((1 + gMie2 - 2.0 * gMie * cosine), 1.5f);
}
float rhoRayleigh(float h)
{
    if (h < 0.0f)
    {
        h = 0.0f;
    }
    return atmosphereDensity * exp(-abs(h) / HRayleigh); // 大气密度近似
}
float rhoMie(float h)
{
    if (h < 0.0f)
    {
        h = 0.0f;
    }
    return MieDensity * exp(-abs(h) / HMie);
}

vec4 scatterCoefficientRayleigh(vec3 p)
{
    // vec3 intersection = intersectSky(camPos, camRayDir);

    float h = (heightToGround(p)); // 散射点高度
    return betaRayleigh * rhoRayleigh(h);
}
vec4 scatterCoefficientMie(vec3 p)
{
    // vec3 intersection = intersectSky(camPos, camRayDir);

    float h = (heightToGround(p)); // 散射点高度
    return betaMie * rhoMie(h);
}

vec4 transmittance(vec3 ori, vec3 end, float scale)
{
    vec4 t;                                              // 透射率
    vec4 betaMieAbsorb = vec4(2.5e-5, 4e-5, 1e-5, 1.0f); // Hacking 让地平线呈现微妙紫色
    const float tMaxStep = 64;

    float dist = length(end - ori);
    float tItvl = dist / float(tMaxStep);

    // 光学深度积分
    float opticalDepthMie = 0.f;
    float opticalDepthRayleigh = 0.f;
    for (int i = 0; i < tMaxStep; ++i)
    {
        vec3 p = ori + i * (end - ori) / tMaxStep;
        float h = heightToGround(p);
        opticalDepthMie += tItvl * rhoMie(h) * scale;
        opticalDepthRayleigh += tItvl * rhoRayleigh(h) * scale;
    }

    // 总透射率计算
    vec4 extictionMie = (betaMie + betaMieAbsorb * absorbMie) * opticalDepthMie;
    vec4 extictionRayleigh = betaRayleigh * opticalDepthRayleigh;
    t = vec4(exp(-(extictionMie + extictionRayleigh).r),
             exp(-(extictionMie + extictionRayleigh).g),
             exp(-(extictionMie + extictionRayleigh).b),
             1.0f);
    return t;
}

