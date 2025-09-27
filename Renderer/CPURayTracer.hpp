
#pragma once
#include "Shader.hpp"
#include "Utils.hpp"
#include "Camera.hpp"
#include "Renderer.hpp"
#include <future>
#include <vector>
#include <imgui.h>
#include <glm/gtc/type_ptr.hpp>

class CPURayTracer
{
public:
    int width;
    int height;

private:
    Texture2D imageTexture;
    std::vector<vec4> imageData;
    std::vector<std::future<void>> shadingFutures;
    int sampleCount = 1;
    float perturbStrength = 1e-3f;
    float secPerSample = 0.f;

    void setPixel(int x, int y, vec4 &value);
    vec4 &pixelAt(int x, int y);
    vec2 uvAt(int x, int y);
    void syncAndUploadShadingResult();
    void shadingAsync(int numThreads);
    void shade(int x, int y);

public:
    CPURayTracer(int _width, int _height);
    ~CPURayTracer();
    unsigned int getGLTextureID();
    void resize(int newWidth, int newHeight);
    void resetSamples();
    void draw(int numThreads=16);
    void sync();
};
