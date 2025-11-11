#include "RenderState.hpp"
#include "Config.hpp"
namespace RenderState
{
    namespace Test
    {
        inline int SampleTimes;
        inline std::string RenderModeString;

        inline void LoadConfig(const std::optional<RenderConfig> &config)
        {
            if (!config)
            {
                return;
            }
            RenderState::LoadConfig(config); // 复用主程序的配置加载逻辑
            // Test Features
            const auto &j = config->jsonConfig;
            if (j.contains("SampleTimes"))
                SampleTimes = j["SampleTimes"];
            if (j.contains("RenderMode"))
                RenderModeString = j["RenderMode"];
        }
    }
}