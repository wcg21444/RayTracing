#pragma once
#include "Utils.hpp"

class Camera
{
public:
    float focalLength;
    vec3 position;
    vec3 lookAtCenter;
    float width;
    float height;
    float aspectRatio;

public:
    Camera() {}
    Camera(float _focalLength,
           vec3 _position,
           float _width,
           float _aspectRatio,
           vec3 _lookAtCenter = vec3(0.0f)) : focalLength(_focalLength),
                                              position(_position),
                                              width(_width),
                                              height(width / _aspectRatio),
                                              aspectRatio(_aspectRatio),
                                              lookAtCenter(_lookAtCenter)

    {
    }

    vec3 getRayDirction(const vec2 &uv)
    {
        vec3 viewDir = vec3(
            uv.x * width - width / 2,
            uv.y * height - height / 2,
            focalLength);

        vec3 absY = vec3(0.f, 1.f, 0.f);
        vec3 z = DirectionOf(lookAtCenter, position);
        vec3 x = glm::normalize(glm::cross(absY, z));
        vec3 y = glm::cross(z, x);
        glm::mat3 rotation(x, y, z);

        return rotation * normalize(viewDir);
    }
    void resize(int newWidth, int newHeight)
    {
        this->aspectRatio = float(newWidth) / newHeight;
        height = width / aspectRatio;
    }
    float getHorizontalFOV()
    {
        return 2.f * std::atan(width / 2 / focalLength) * 180.f / glm::pi<float>();
    }
};