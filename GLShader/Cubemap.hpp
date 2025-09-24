#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>
#include <string>
#include <iostream>

class CubemapParameters
{
public:
    glm::mat4 projectionMartix;
    std::vector<glm::mat4> viewMatrices;
    glm::vec3 viewPosition;
    float aspect;
    float nearPlane;
    float farPlane;
    float fov;

    CubemapParameters(float _nearPlane, float _farPlane, glm::vec3 _viewPosition, float _fov = 90.f, float _aspect = 1.0f)
        : nearPlane(_nearPlane), farPlane(_farPlane), viewPosition(_viewPosition), fov(_fov), aspect(_aspect)
    {
        projectionMartix = glm::perspective(glm::radians(90.0f), aspect, nearPlane, farPlane);
        viewMatrices = {
            glm::lookAt(viewPosition, viewPosition + glm::vec3(1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0)),
            glm::lookAt(viewPosition, viewPosition + glm::vec3(-1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0)),
            glm::lookAt(viewPosition, viewPosition + glm::vec3(0.0, 1.0, 0.0), glm::vec3(0.0, 0.0, 1.0)),
            glm::lookAt(viewPosition, viewPosition + glm::vec3(0.0, -1.0, 0.0), glm::vec3(0.0, 0.0, -1.0)),
            glm::lookAt(viewPosition, viewPosition + glm::vec3(0.0, 0.0, 1.0), glm::vec3(0.0, -1.0, 0.0)),
            glm::lookAt(viewPosition, viewPosition + glm::vec3(0.0, 0.0, -1.0), glm::vec3(0.0, -1.0, 0.0))};
    }
    ~CubemapParameters() = default;
    void update(const glm::vec3 &position)
    {
        viewPosition = position;
        std::vector<glm::mat4> tempViewMatrices = {
            glm::lookAt(position, position + glm::vec3(1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0)),
            glm::lookAt(position, position + glm::vec3(-1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0)),
            glm::lookAt(position, position + glm::vec3(0.0, 1.0, 0.0), glm::vec3(0.0, 0.0, 1.0)),
            glm::lookAt(position, position + glm::vec3(0.0, -1.0, 0.0), glm::vec3(0.0, 0.0, -1.0)),
            glm::lookAt(position, position + glm::vec3(0.0, 0.0, 1.0), glm::vec3(0.0, -1.0, 0.0)),
            glm::lookAt(position, position + glm::vec3(0.0, 0.0, -1.0), glm::vec3(0.0, -1.0, 0.0))};

        viewMatrices = tempViewMatrices;
    }
};
