#pragma once
#include <nlohmann/json.hpp>
#include <fstream>
#include <optional>
class SceneConfig
{
public:
    SceneConfig(const std::string &path)
    {
        std::ifstream file(path);
        if (file.is_open())
        {
            jsonConfig = nlohmann::json::parse(file);
            // Parse the JSON data into the SceneConfig object
            name = jsonConfig["name"].get<std::string>();
            type = jsonConfig["type"].get<std::string>();
        }
    }

    std::string name;
    std::string type;
    nlohmann::json jsonConfig;
};

class RenderConfig
{

public:
    RenderConfig(const std::string &path)
    {
        std::ifstream file(path);
        if (file.is_open())
        {
            jsonConfig = nlohmann::json::parse(file);
            // Parse the JSON data into the RenderConfig object
            name = jsonConfig["name"].get<std::string>();
        }
    }

    std::string name;
    nlohmann::json jsonConfig;
};

namespace RenderState
{
    void LoadConfig(const std::optional<RenderConfig> &config);
}