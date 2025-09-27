#pragma once
#include <glm/glm.hpp>
#include "UICommon.hpp"
#include "RenderState.hpp"

class SkySettings
{
public:
    inline static float skyHeight = 1e5;       // 大气层高度
    inline static float earthRadius = 6.371e6; // 地球半径
    inline static float skyIntensity = 1.f;    // 天空光强度
    inline static float HRayleigh = 8.5e3;
    inline static float HMie = 1e3;
    inline static float atmosphereDensity = 1.f; // 大气密度
    inline static float MieDensity = 1.0f;
    inline static float gMie = 0.56f;
    inline static float absorbMie = 0.1f;
    inline static float MieIntensity = 1e-1f;
    inline static glm::vec4 betaMie = glm::vec4(21e-6, 21e-6, 21e-6, 1.0f);
    inline static int maxStep = 32;
    inline static glm::vec3 sunlightDir = glm::vec3(1.0f, 0.3f, 0.4f);
    inline static glm::vec4 sunlightIntensity = glm::vec4(1.0f);

    inline static void RenderUI()
    {
        ImGui::Begin("ShadersGUI");
        {
            ImGui::PushItemWidth(100.f);

            // β_Mie
            ImGui::Text("BetaMie");
            ImGui::PushID("BetaMie");
            RenderState::Dirty |= ImGui::DragFloat("R", &betaMie.r, 1.0e-7f, 1e-7f, 1e-4f, "%.5e");
            RenderState::Dirty |= ImGui::DragFloat("G", &betaMie.g, 1.0e-7f, 1e-7f, 1e-4f, "%.5e");
            RenderState::Dirty |= ImGui::DragFloat("B", &betaMie.b, 1.0e-7f, 1e-7f, 1e-4f, "%.5e");
            ImGui::PopID();

            RenderState::Dirty |= ImGui::DragFloat("skyHeight", &skyHeight, 1e3f, 1e1f, 1e7f);
            RenderState::Dirty |= ImGui::DragFloat("earthRadius", &earthRadius, 1e4f, 1e1f, 1e7f);
            RenderState::Dirty |= ImGui::DragFloat("skyIntensity", &skyIntensity, 1e-1f, 0.0f, 1e3);
            RenderState::Dirty |= ImGui::DragInt("maxStep", &maxStep, 1, 1, 128);
            RenderState::Dirty |= ImGui::DragFloat("HRayleigh", &HRayleigh, 10.f, 0.0f, 1e5);
            RenderState::Dirty |= ImGui::DragFloat("HMie", &HMie, 2.f, 0.0f, 1e4);
            RenderState::Dirty |= ImGui::DragFloat("AtmosphereDensity", &atmosphereDensity, 0.05f, 0.0f, 1e2);
            RenderState::Dirty |= ImGui::DragFloat("MieDensity", &MieDensity, 0.05f, 0.0f, 1e2);
            RenderState::Dirty |= ImGui::DragFloat("gMie", &gMie, 0.01f, 0.0f, 1.f);
            RenderState::Dirty |= ImGui::DragFloat("absorbMie", &absorbMie, 0.01f, 1e-3f, 1e1);
            RenderState::Dirty |= ImGui::DragFloat("MieIntensity", &MieIntensity, 0.01f, 1e-2f, 1e2);
            ImGui::PopItemWidth();

            RenderState::Dirty |= ImGui::DragFloat3("SunlightDir", glm::value_ptr(sunlightDir));
            RenderState::Dirty |= ImGui::ColorPicker4("SunlightIntensity", glm::value_ptr(sunlightIntensity));
        }
        ImGui::End();
    }

    inline static void SetShaderUniforms(Shader &shaders)
    {
        shaders.setFloat("skyHeight", SkySettings::skyHeight);
        shaders.setFloat("earthRadius", SkySettings::earthRadius);
        shaders.setFloat("skyIntensity", SkySettings::skyIntensity);
        shaders.setInt("maxStep", SkySettings::maxStep);
        shaders.setFloat("HRayleigh", SkySettings::HRayleigh);
        shaders.setFloat("HMie", SkySettings::HMie);
        shaders.setFloat("atmosphereDensity", SkySettings::atmosphereDensity);
        shaders.setFloat("MieDensity", SkySettings::MieDensity);
        shaders.setFloat("gMie", SkySettings::gMie);
        shaders.setFloat("absorbMie", SkySettings::absorbMie);
        shaders.setFloat("MieIntensity", SkySettings::MieIntensity);
        shaders.setUniform("betaMie", SkySettings::betaMie);
        shaders.setUniform("sunlightDir", SkySettings::sunlightDir);
        shaders.setUniform("sunlightIntensity", SkySettings::sunlightIntensity);
    }
};
