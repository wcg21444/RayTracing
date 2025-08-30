#pragma once
#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_glfw.h"
#include "imgui/backends/imgui_impl_opengl3.h"
#include <glm/glm.hpp>

#define IMGUI_EDITED_STATE_DEFINE \
    bool IMGUI_EDITED = false;
#define CHECK_IMGUI_EDITED     \
    if (ImGui::IsItemEdited()) \
    {                          \
        IMGUI_EDITED = true;   \
    }

class SkyGUI
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
    inline static bool Render()
    {
        IMGUI_EDITED_STATE_DEFINE

        ImGui::Begin("ShadersGUI");
        {
            ImGui::PushItemWidth(100.f);

            // β_Mie
            ImGui::Text("BetaMie");
            ImGui::PushID("BetaMie");
            ImGui::DragFloat("R", &betaMie.r, 1.0e-7f, 1e-7f, 1e-4f, "%.5e");
            CHECK_IMGUI_EDITED;
            ImGui::DragFloat("G", &betaMie.g, 1.0e-7f, 1e-7f, 1e-4f, "%.5e");
            CHECK_IMGUI_EDITED;
            ImGui::DragFloat("B", &betaMie.b, 1.0e-7f, 1e-7f, 1e-4f, "%.5e");
            CHECK_IMGUI_EDITED;
            ImGui::PopID();

            ImGui::DragFloat("skyHeight", &skyHeight, 1e3f, 1e1f, 1e7f);
            CHECK_IMGUI_EDITED;
            ImGui::DragFloat("earthRadius", &earthRadius, 1e4f, 1e1f, 1e7f);
            CHECK_IMGUI_EDITED;
            ImGui::DragFloat("skyIntensity", &skyIntensity, 1e-1f, 0.0f, 1e3);
            CHECK_IMGUI_EDITED;
            ImGui::DragInt("maxStep", &maxStep, 1, 1, 128);
            CHECK_IMGUI_EDITED;
            ImGui::DragFloat("HRayleigh", &HRayleigh, 10.f, 0.0f, 1e5);
            CHECK_IMGUI_EDITED;
            ImGui::DragFloat("HMie", &HMie, 2.f, 0.0f, 1e4);
            CHECK_IMGUI_EDITED;
            ImGui::DragFloat("AtmosphereDensity", &atmosphereDensity, 0.05f, 0.0f, 1e2);
            CHECK_IMGUI_EDITED;
            ImGui::DragFloat("MieDensity", &MieDensity, 0.05f, 0.0f, 1e2);
            CHECK_IMGUI_EDITED;
            ImGui::DragFloat("gMie", &gMie, 0.01f, 0.0f, 1.f);
            CHECK_IMGUI_EDITED;
            ImGui::DragFloat("absorbMie", &absorbMie, 0.01f, 1e-3f, 1e1);
            CHECK_IMGUI_EDITED;
            ImGui::DragFloat("MieIntensity", &MieIntensity, 0.01f, 1e-2f, 1e2);
            CHECK_IMGUI_EDITED;
            ImGui::PopItemWidth();

            ImGui::DragFloat3("SunlightDir", glm::value_ptr(sunlightDir));
            CHECK_IMGUI_EDITED;
            ImGui::ColorPicker4("SunlightIntensity", glm::value_ptr(sunlightIntensity));
            CHECK_IMGUI_EDITED;
        }
        ImGui::End();
        return IMGUI_EDITED;
    }
};