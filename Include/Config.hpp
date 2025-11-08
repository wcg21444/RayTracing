#pragma once
#include <nlohmann/json.hpp>
#include <fstream>
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
