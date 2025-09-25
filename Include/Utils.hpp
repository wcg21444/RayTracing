#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <cmath>
#include <iostream>
#include <format>

using point3 = glm::vec3;
using poinn2 = glm::vec2;
using vec2 = glm::vec2;
using vec3 = glm::vec3;
using vec4 = glm::vec4;
using mat3 = glm::mat3;
using mat4 = glm::mat4;
using color4 = glm::vec4;
using color3 = glm::vec3;

const color4 RED = color4(1.0f, 0.0f, 0.0f, 1.0f);
const color4 GREEN = color4(0.0f, 1.0f, 0.0f, 1.0f);
const color4 BLUE = color4(0.0f, 0.0f, 1.0f, 1.0f);

inline void CheckGLErrors()
{
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR)
    {
        std::cout << "OpenGL error: " << err << std::endl;
    }
}

inline vec3 DirectionOf(float x, float y, float z)
{
    return normalize(vec3(x, y, z));
}

inline vec3 DirectionOf(const vec3 &v)
{
    return normalize(v);
}
inline vec3 DirectionOf(const point3 &end, const point3 &ori)
{
    return normalize(end - ori);
}
struct FrustumCorners
{
    glm::vec3 nearTopLeft, nearTopRight, nearBottomRight, nearBottomLeft;
    glm::vec3 farTopLeft, farTopRight, farBottomRight, farBottomLeft;
};

class FrustumBase
{
public:
    virtual glm::mat4 getViewMatrix() const = 0;
    virtual glm::mat4 getProjectionMatrix() const = 0;
    virtual glm::mat4 getProjViewMatrix() const = 0;
    virtual FrustumCorners getCorners() const = 0;
    virtual glm::vec3 getPosition() const = 0;
    virtual glm::vec3 getFront() const = 0;
    virtual glm::vec3 getUp() const = 0;
    virtual float getNearPlane() const = 0;
    virtual float getFarPlane() const = 0;
};

class Frustum : public FrustumBase
{
public:
    glm::vec3 m_position;
    glm::vec3 m_front;
    glm::vec3 m_up;
    float m_nearPlane;
    float m_farPlane;
    float m_fov;
    float m_aspect;

    inline Frustum();
    inline Frustum(const glm::vec3 &position, const glm::vec3 &front, const glm::vec3 &up,
            float nearPlane, float farPlane, float fov, float aspect);
    inline Frustum(const glm::mat4 &viewMatrix, const glm::mat4 &projectionMatrix);
    inline Frustum getSubFrustum(float nearPlane, float farPlane) const;
    inline glm::vec3 getPosition() const override;
    inline glm::vec3 getFront() const override;
    inline glm::vec3 getUp() const override;
    inline float getNearPlane() const override;
    inline float getFarPlane() const override;
    inline glm::mat4 getViewMatrix() const override;
    inline glm::mat4 getProjectionMatrix() const override;
    inline glm::mat4 getProjViewMatrix() const override;
    inline FrustumCorners getCorners() const override;
};

Frustum::Frustum()
{
}

// Frustum implementations
Frustum::Frustum(const glm::vec3 &position, const glm::vec3 &front, const glm::vec3 &up,
                 float nearPlane, float farPlane, float fov, float aspect)
    : m_position(position), m_front(front), m_up(up),
      m_nearPlane(nearPlane), m_farPlane(farPlane), m_fov(fov), m_aspect(aspect)
{
}

Frustum::Frustum(const glm::mat4 &viewMatrix, const glm::mat4 &projectionMatrix)
{
    glm::mat4 invView = glm::inverse(viewMatrix);

    m_position = glm::vec3(invView[3]);
    m_front = glm::normalize(glm::vec3(invView * glm::vec4(0.0f, 0.0f, -1.0f, 0.0f)));
    m_up = glm::normalize(glm::vec3(invView * glm::vec4(0.0f, 1.0f, 0.0f, 0.0f)));

    m_fov = glm::degrees(2.0f * atan(1.0f / projectionMatrix[1][1]));
    m_aspect = projectionMatrix[1][1] / projectionMatrix[0][0];
    m_nearPlane = projectionMatrix[3][2] / (projectionMatrix[2][2] - 1.0f);
    m_farPlane = projectionMatrix[3][2] / (projectionMatrix[2][2] + 1.0f);
}

Frustum Frustum::getSubFrustum(float nearPlane, float farPlane) const
{
    return Frustum(m_position, m_front, m_up, nearPlane, farPlane, m_fov, m_aspect);
}

glm::vec3 Frustum::getPosition() const { return m_position; }
glm::vec3 Frustum::getFront() const { return m_front; }
glm::vec3 Frustum::getUp() const { return m_up; }
float Frustum::getNearPlane() const { return m_nearPlane; }
float Frustum::getFarPlane() const { return m_farPlane; }

glm::mat4 Frustum::getViewMatrix() const
{
    return glm::lookAt(m_position, m_position + m_front, m_up);
}

glm::mat4 Frustum::getProjectionMatrix() const
{
    return glm::perspective(glm::radians(m_fov), m_aspect, m_nearPlane, m_farPlane);
}

glm::mat4 Frustum::getProjViewMatrix() const
{
    return glm::perspective(glm::radians(m_fov), m_aspect, m_nearPlane, m_farPlane) * glm::lookAt(m_position, m_position + m_front, m_up);
}

FrustumCorners Frustum::getCorners() const
{
    // 1. 计算近平面和远平面的宽高
    float tanHalfFov = glm::tan(glm::radians(m_fov / 2.0f));
    float nearHalfHeight = tanHalfFov * m_nearPlane;
    float nearHalfWidth = nearHalfHeight * m_aspect;
    float farHalfHeight = tanHalfFov * m_farPlane;
    float farHalfWidth = farHalfHeight * m_aspect;

    // 2. 计算摄像机局部空间的八个角点，并按统一顺序
    const glm::vec3 nearTopLeft(-nearHalfWidth, nearHalfHeight, -m_nearPlane);
    const glm::vec3 nearTopRight(nearHalfWidth, nearHalfHeight, -m_nearPlane);
    const glm::vec3 nearBottomRight(nearHalfWidth, -nearHalfHeight, -m_nearPlane);
    const glm::vec3 nearBottomLeft(-nearHalfWidth, -nearHalfHeight, -m_nearPlane);
    const glm::vec3 farTopLeft(-farHalfWidth, farHalfHeight, -m_farPlane);
    const glm::vec3 farTopRight(farHalfWidth, farHalfHeight, -m_farPlane);
    const glm::vec3 farBottomRight(farHalfWidth, -farHalfHeight, -m_farPlane);
    const glm::vec3 farBottomLeft(-farHalfWidth, -farHalfHeight, -m_farPlane);

    // 3. 计算世界矩阵
    const glm::mat4 worldMatrix = glm::inverse(getViewMatrix());

    // 4. 将局部空间的角点变换到世界空间并返回 FrustumCorners
    FrustumCorners corners;
    const auto transform = [&](const glm::vec3 &localPos)
    {
        return glm::vec3(worldMatrix * glm::vec4(localPos, 1.0f));
    };

    corners.nearTopLeft = transform(nearTopLeft);
    corners.nearTopRight = transform(nearTopRight);
    corners.nearBottomRight = transform(nearBottomRight);
    corners.nearBottomLeft = transform(nearBottomLeft);
    corners.farTopLeft = transform(farTopLeft);
    corners.farTopRight = transform(farTopRight);
    corners.farBottomRight = transform(farBottomRight);
    corners.farBottomLeft = transform(farBottomLeft);

    return corners;
}