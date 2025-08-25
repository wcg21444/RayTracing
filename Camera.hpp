#pragma once
#include "Utils.hpp"

class Camera
{
public:
    float focalLength;
    vec3 position;
    float width;
    float height;
    float aspectRatio;

public:
    Camera() {}
    Camera(float _focalLength,
           vec3 _position, float _width, float _aspectRatio) : focalLength(_focalLength),
                                                               position(_position),
                                                               width(_width),
                                                               height(width / _aspectRatio),
                                                               aspectRatio(_aspectRatio)
    {
    }

    vec3 getRayDirction(const vec2 &uv)
    {
        vec3 point = vec3(
            uv.x * width - width / 2,
            uv.y * height - height / 2,
            focalLength);
        return point - position;
    }
    void resize(int newWidth, int newHeight)
    {
        aspectRatio = float(newWidth) / newHeight;
        height = width / aspectRatio;
    }
};