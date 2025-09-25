#pragma once
#include "Utils.hpp"
#include <glm/gtc/matrix_transform.hpp>
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
        vec3 viewDir = normalize(vec3(
            uv.x * width - width / 2,
            uv.y * height - height / 2,
            focalLength));
        viewDir.x = -viewDir.x;
        vec3 absY = vec3(0.f, 1.f, 0.f);
        vec3 z = DirectionOf(lookAtCenter, position);
        vec3 x = glm::normalize(glm::cross(absY, z));
        vec3 y = glm::cross(z, x);
        glm::mat3 rotation(x, y, z);

        return rotation * viewDir;
    }
    void resize(int newWidth, int newHeight)
    {
        this->aspectRatio = float(newWidth) / newHeight;
        height = width / aspectRatio;
    }
    float getHorizontalFOV() const
    {
        return 2.f * std::atan(width / 2 / focalLength) * 180.f / glm::pi<float>();
    }

    float getVerticalFOV() const
    {
        return 2.f * std::atan(height / 2 / focalLength) * 180.f / glm::pi<float>();
    }

    glm::mat4 getViewMatrix() const
    {
        return glm::lookAt(position, lookAtCenter, vec3(0.0f, 1.0f, 0.0f));
    }

    glm::mat4 getProjectionMatrix() const
    {
        return glm::perspective(glm::radians(getVerticalFOV()), aspectRatio, 0.1f, 1e5f);
    }

    void setToFragShader(Shader &shaders, std::string owner = "")
    {
        if (owner != "")
        {
            owner += ".";
        }
        shaders.setUniform(std::format("{}view", owner), getViewMatrix());
        shaders.setUniform(std::format("{}projection", owner), getProjectionMatrix());
        shaders.setUniform(std::format("{}focalLength", owner), focalLength);
        shaders.setUniform(std::format("{}position", owner), position);
        shaders.setUniform(std::format("{}lookAtCenter", owner), lookAtCenter);
        shaders.setUniform(std::format("{}width", owner), width);
        shaders.setUniform(std::format("{}height", owner), height);
        shaders.setUniform(std::format("{}aspectRatio", owner), aspectRatio);
    }
};