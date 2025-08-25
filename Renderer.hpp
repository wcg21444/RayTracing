#pragma once

#include "GLShader/Shader.hpp"
#include "GLShader/Pass.hpp"
#include "Utils.hpp"
#include "Trace.hpp"
#include "Camera.hpp"

using namespace glm;

class Image
{
public:
    int width;
    int height;

private:
    Texture imageTexture;
    std::vector<vec4> imageData;

public:
    Image(int _width,
          int _height) : width(_width), height(_height),
                         imageData(_width * _height)
    {
        imageTexture.Generate(_width, _height, GL_RGBA16F, GL_RGBA, GL_FLOAT, NULL);
        imageTexture.SetFilterMax(GL_NEAREST);
        imageTexture.SetFilterMin(GL_NEAREST);
    }
    unsigned int getGLTextureID()
    {
        return imageTexture.ID;
    }

    void setPixel(int x, int y, vec4 &value)
    {
        imageData[y * width + x] = value;
    }
    vec4 &pixelAt(int x, int y)
    {
        return imageData[y * width + x];
    }
    vec2 uvAt(int x, int y)
    {
        return vec2(x / float(width), y / float(height));
    }

    void resize(int newWidth, int newHeight)
    {
        width = newWidth;
        height = newHeight;

        imageTexture.Resize(newWidth, newHeight);

        // 拉伸imageData
        imageData.clear();
        imageData.resize(newWidth * newHeight);
        draw();
    }

    void draw()
    {
        for (int x = 0; x < width; ++x)
        {
            for (int y = 0; y < height; ++y)
            {
                shade(x, y);
            }
        }
        imageTexture.SetData(imageData.data());
    }

    void shade(int x, int y)
    {
        vec2 uv = uvAt(x, y);
        vec4 &pixelColor = pixelAt(x, y);

        static Camera camera = Camera(0.2f, point3(0.0f), 2.0f, float(16) / float(9));

        camera.resize(width, height); // 适应image比例

        Ray ray(camera.position, camera.getRayDirction(uv)); // 每一个像素,打出一根光线进行追踪,然后着色
        pixelColor = castRay(ray);
    }
};

class Renderer
{
private:
    int width = 1600;
    int height = 900;

    Texture screenTexture;

private:
public:
    ScreenPass screenPass;
    Image image;
    // Camera camera;
    // Scene

    Renderer()
        : screenPass(ScreenPass(width, height, "GLSL/screenQuad.vs", "GLSL/texture.fs")),
          image(Image(width, height))
    {
        screenPass.contextSetup();
    }
    void render()
    {
        image.draw();
        auto imageTextureID = image.getGLTextureID();
        screenPass.render(imageTextureID);
    }
    void resize(int newWidth, int newHeight)
    {
        width = newWidth;
        height = newHeight;

        image.resize(width, height);
        screenPass.resize(width, height);
    }
};