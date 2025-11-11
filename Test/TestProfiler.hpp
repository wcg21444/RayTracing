#pragma once
#include "Profiler.hpp"
namespace Profiler
{
    namespace Test
    {
        inline std::unordered_map<std::string, TimeBeginEnd> TimeBlocks;
        class ScopedTimeBlock
        {
            using hrClock = std::chrono::high_resolution_clock;

        public:
            inline ScopedTimeBlock(const std::string &name)
                : blockName(name)
            {
                if (TimeBlocks.find(blockName) == TimeBlocks.end())
                {
                    TimeBlocks[blockName] = TimeBeginEnd(); // 重置时间
                }
                TimeBlocks[blockName].startTime = hrClock::now();
            }
            inline ~ScopedTimeBlock()
            {
                TimeBlocks[blockName].endTime = hrClock::now();
            }

        private:
             std::string blockName; // 有字串构造, 不应该频繁使用
        };

        inline void OutputStatistics()
        {
            for (const auto &[name, timeBlock] : TimeBlocks)
            {
                auto duration = timeBlock.duration<std::chrono::nanoseconds>();
                std::cout << name << " Duration: " << ConvertNsAuto(duration) << std::endl;
            }
        }
    }
}
