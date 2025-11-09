#include "Camera.hpp"
#include <algorithm>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <format>
#include "RenderState.hpp"

Camera::Camera()
{
}

Camera::Camera(float _focalLength, vec3 _position, float _width, float _aspectRatio, vec3 _lookAtCenter)
    : focalLength(_focalLength),
      position(_position),
      width(_width),
      height(_width / _aspectRatio),
      aspectRatio(_aspectRatio),
      lookAtCenter(_lookAtCenter)
{
}

void Camera::setController(std::shared_ptr<ICameraController> controller)
{
    this->controller = controller; // 增加sptr引用
}

void Camera::update()
{
    renderUI();
    if (controller)
    {
        auto [position, lookAtCenter, focalLength] = controller->getUpdatedControlState();
        if (position != this->position || lookAtCenter != this->lookAtCenter || focalLength != this->focalLength)
        {
            RenderState::Dirty = true;
            this->position = position;
            this->lookAtCenter = lookAtCenter;
            this->focalLength = focalLength;
        }
    }
}

vec3 Camera::getRayDirection(const vec2 &uv) const
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

void Camera::resize(int newWidth, int newHeight)
{
    this->aspectRatio = float(newWidth) / newHeight;
    height = width / aspectRatio;
}

float Camera::getHorizontalFOV() const
{
    return 2.f * std::atan(width / 2 / focalLength) * 180.f / glm::pi<float>();
}

float Camera::getVerticalFOV() const
{
    return 2.f * std::atan(height / 2 / focalLength) * 180.f / glm::pi<float>();
}

glm::mat4 Camera::getViewMatrix() const
{
    if (position == lookAtCenter)
    {
        return glm::mat4(1.0f);
    }
    return glm::lookAt(position, lookAtCenter, vec3(0.0f, 1.0f, 0.0f));
}

glm::mat4 Camera::getProjectionMatrix() const
{
    return glm::perspective(glm::radians(getVerticalFOV()), aspectRatio, 0.1f, 1e5f);
}

void Camera::setToFragShader(Shader &shaders, std::string owner)
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

void Camera::renderUI()
{
    ImGui::Begin("Camera Control");
    {
        ImGui::Text(std::format("HFov: {}", getHorizontalFOV()).c_str());
        ImGui::Text(std::format("VFov: {}", getVerticalFOV()).c_str());
        ImGui::Text(std::format("Aspect Ratio: {}", aspectRatio).c_str());
    }
    ImGui::End();
}

EasyCameraController::EasyCameraController(float _focalLength, vec3 _position, vec3 _lookAtCenter)
    : position(_position),
      lookAtCenter(_lookAtCenter),
      focalLength(_focalLength)
{
}

EasyCameraController::EasyCameraController(EasyCameraController &&other) noexcept
{
    position = std::move(other.position);
    lookAtCenter = std::move(other.lookAtCenter);
    focalLength = std::move(other.focalLength);
    sensitivity = std::move(other.sensitivity);
}

EasyCameraController &EasyCameraController::operator=(EasyCameraController &&other) noexcept
{
    if (this != &other)
    {
        position = std::move(other.position);
        lookAtCenter = std::move(other.lookAtCenter);
        focalLength = std::move(other.focalLength);
        sensitivity = std::move(other.sensitivity);
    }
    return *this;
}

void EasyCameraController::setControlState(const Camera::ControlState &state)
{
    position = state.position;
    lookAtCenter = state.lookAtCenter;
    focalLength = state.focalLength;
}

void EasyCameraController::processOrientationOffset(float xoffset, float yoffset)
{
    vec3 front = normalize(lookAtCenter - position);
    vec3 right = normalize(cross(front, vec3(0.0f, 1.0f, 0.0f)));
    vec3 up = normalize(cross(right, front));
    float dist = glm::length(lookAtCenter - position);
    xoffset *= sensitivity;
    yoffset *= sensitivity;
    position -= (xoffset * right + yoffset * up) * sensitivity * glm::sqrt(dist);
    position = lookAtCenter + normalize(position - lookAtCenter) * dist;
}

void EasyCameraController::processDollyOffset(float xoffset, float yoffset)
{
    vec3 front = normalize(lookAtCenter - position);
    float dist = glm::length(lookAtCenter - position);
    xoffset *= sensitivity;
    yoffset *= sensitivity;
    float offset = xoffset + yoffset;
    position += front * offset * sensitivity * glm::sqrt(dist);
}

void EasyCameraController::processMouseScroll(float yoffset)
{
    sensitivity += yoffset * 0.01f;
    sensitivity = std::max(0.001f, sensitivity);
}
void EasyCameraController::processPanOffset(float xoffset, float yoffset)
{
    vec3 front = normalize(lookAtCenter - position);
    vec3 right = normalize(cross(front, vec3(0.0f, 1.0f, 0.0f)));
    vec3 up = normalize(cross(right, front));
    float dist = glm::length(lookAtCenter - position);
    xoffset *= sensitivity;
    yoffset *= sensitivity;
    lookAtCenter += (-xoffset * right + yoffset * up) * sensitivity * glm::sqrt(dist);
    position += (-xoffset * right + yoffset * up) * sensitivity * glm::sqrt(dist);
}

void EasyCameraController::processZoom(float xoffset, float yoffset)
{
    float offset = xoffset + yoffset;
    float curve = 1.5f;
    focalLength += std::pow(focalLength, curve) * offset *1e-2f;
    focalLength = std::max(0.1f, focalLength);
    focalLength = std::min(1e4f, focalLength);
}

Camera::ControlState EasyCameraController::getUpdatedControlState()
{
    renderUI();
    return Camera::ControlState(position, lookAtCenter, focalLength);
}

void EasyCameraController::renderUI()
{
    ImGui::Begin("Camera Control");
    {
        RenderState::Dirty |= ImGui::DragFloat3("CamPosition", glm::value_ptr(position), 0.01f);
        RenderState::Dirty |= ImGui::DragFloat3("LookAtCenter", glm::value_ptr(lookAtCenter), 0.01f);
        RenderState::Dirty |= ImGui::SliderFloat("CamFocalLength", &focalLength, 1e-4f, 100.0f, "%.4f");
        ImGui::SliderFloat("Sensitivity", &sensitivity, 0.001f, 1.0f);
    }
    ImGui::End();
}
