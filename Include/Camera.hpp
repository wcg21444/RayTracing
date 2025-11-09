#pragma once
#include "Utils.hpp"
#include "Shader.hpp"
#include <glm/gtc/matrix_transform.hpp>
class Shader;
class ICameraController;

class Camera
{
public:
    struct ControlState
    {
        vec3 position;
        vec3 lookAtCenter;
        float focalLength;
    };

    float focalLength;
    vec3 position;
    vec3 lookAtCenter;

    float width;
    float height;
    float aspectRatio;

    std::shared_ptr<ICameraController> controller = nullptr;

public:
    Camera();
    Camera(float _focalLength,
           vec3 _position,
           float _width,
           float _aspectRatio,
           vec3 _lookAtCenter = vec3(0.0f));

    void setController(std::shared_ptr<ICameraController> controller);
    void update();

    vec3 getRayDirection(const vec2 &uv) const;
    void resize(int newWidth, int newHeight);
    float getHorizontalFOV() const;
    float getVerticalFOV() const;
    glm::mat4 getViewMatrix() const;
    glm::mat4 getProjectionMatrix() const;
    void setToFragShader(Shader &shaders, std::string owner = "");
    void renderUI();
};

// Usage: 通过调用Process系列函数来控制Camera
//        getUpdatedControlState 获取新状态用于驱动 Camera
class ICameraController
{
public:
    virtual void processOrientationOffset(float xoffset, float yoffset) = 0;
    virtual void processDollyOffset(float xoffset, float yoffset) = 0;
    virtual void processMouseScroll(float yoffset) = 0;
    virtual void processPanOffset(float xoffset, float yoffset) = 0;
    virtual void processZoom(float xoffset, float yoffset) = 0;

    virtual Camera::ControlState getUpdatedControlState() = 0; // 更新控制器状态并获取当前控制器状态

    virtual void setControlState(const Camera::ControlState &state) = 0;

    virtual ~ICameraController() = default;
};

class EasyCameraController : public ICameraController
{
public:
    // cache states
    vec3 position;
    vec3 lookAtCenter;
    float focalLength;

    float sensitivity = 0.1f;

public:
    EasyCameraController() = default;
    EasyCameraController(float _focalLength,
                         vec3 _position,
                         vec3 _lookAtCenter);
    ~EasyCameraController() override = default;
    EasyCameraController(const EasyCameraController &) = delete;
    EasyCameraController(EasyCameraController &&) noexcept;
    EasyCameraController &operator=(const EasyCameraController &) = delete;
    EasyCameraController &operator=(EasyCameraController &&) noexcept;

    void setControlState(const Camera::ControlState &state);

    void processOrientationOffset(float xoffset, float yoffset) override;
    void processDollyOffset(float xoffset, float yoffset) override;
    void processMouseScroll(float yoffset) override;
    void processPanOffset(float xoffset, float yoffset) override;
    void processZoom(float xoffset, float yoffset) override;
    Camera::ControlState getUpdatedControlState() override;
    void renderUI();
};

class SmoothController : public ICameraController
{
private:
public:
    SmoothController(std::shared_ptr<ICameraController> baseController);

    void processOrientationOffset(float xoffset, float yoffset) override;
    void processDollyOffset(float xoffset, float yoffset) override;
    void processMouseScroll(float yoffset) override;
    void processPanOffset(float xoffset, float yoffset) override;
    void processZoom(float xoffset, float yoffset) override;
    Camera::ControlState getUpdatedControlState() override;

private:
    std::shared_ptr<ICameraController> baseController;
    Camera::ControlState currentState;
    float smoothFactor = 0.1f; // 越大越平滑

    void update(); // call by getUpdatedControlState()
};