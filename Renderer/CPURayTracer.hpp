
#pragma once
#include "Shader.hpp"
#include "Utils.hpp"
#include "Trace.hpp"
#include "Camera.hpp"
#include "Random.hpp"
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
    Camera camera = Camera(1.0f, point3(0.0f, 0.0f, 1.0f), 2.0f, float(16) / float(9));
    int sampleCount = 1;
    float perturbStrength = 1e-3f;

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
    void draw();
};
