#pragma once
#include <nlohmann/json.hpp>
#include <fstream>
#include <string>
#include "RenderState.hpp"
#include "Config.hpp"

namespace RenderState
{
    void LoadConfig(const std::optional<RenderConfig> &config)
    {
        if (!config)
        {
            return;
        }
        // 假设 config->jsonConfig 已经存储了 nlohmann::json
        const auto &j = config->jsonConfig;
        if (j.contains("camera"))
        {
            if (j["camera"].contains("focalLength"))
                RenderState::CameraInstance.focalLength = j["camera"]["focalLength"];
            if (j["camera"].contains("position"))
            {
                auto arr = j["camera"]["position"];
                RenderState::CameraInstance.position = glm::vec3(arr[0], arr[1], arr[2]);
            }
            if (j["camera"].contains("lookAtCenter"))
            {
                auto arr = j["camera"]["lookAtCenter"];
                RenderState::CameraInstance.lookAtCenter = glm::vec3(arr[0], arr[1], arr[2]);
            }
            if (j["camera"].contains("width"))
                RenderState::CameraInstance.width = j["camera"]["width"];
            if (j["camera"].contains("height"))
                RenderState::CameraInstance.height = j["camera"]["height"];
            if (j["camera"].contains("aspectRatio"))
                RenderState::CameraInstance.aspectRatio = j["camera"]["aspectRatio"];

            auto CameraController = std::make_shared<EasyCameraController>(
                RenderState::CameraInstance.focalLength,
                RenderState::CameraInstance.position,
                RenderState::CameraInstance.lookAtCenter);
            if (j["camera"].contains("sensitivity"))
                CameraController->sensitivity = j["camera"]["sensitivity"];
            RenderState::CameraInstance.setController(
                CameraController);
        }
        if (j.contains("InitWidth"))
            RenderState::InitWidth = j["InitWidth"];
        if (j.contains("InitHeight"))
            RenderState::InitHeight = j["InitHeight"];
        if (j.contains("CPUNumThreads"))
            RenderState::CPUNumThreads = j["CPUNumThreads"];
    }
}