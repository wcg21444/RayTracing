#include "Profiler.hpp"
#include "UICommon.hpp"

#include <unordered_map>
#include <chrono>
#include <thread>
#include <mutex>
#include <format>
#include <iostream>
namespace Profiler
{
    // 用两个方法, 划分开始结束区间 , 监测区间内代码耗时,每个区间用户自定义名字
    using namespace std::chrono;
    using hrClock = std::chrono::high_resolution_clock;

    void BeginTimeBlock(const std::string &name)
    {
        // 注册时间块
        if (TimeBlocks.find(name) == TimeBlocks.end())
        {
            TimeBlocks[name] = TimeBeginEnd(); // 重置时间
        }
        TimeBlocks[name].startTime = hrClock::now();
    }

    void EndTimeBlock(const std::string &name)
    {
        // 计算结束时间
        TimeBlocks[name].endTime = hrClock::now();
    }

    ScopedTimeBlock::ScopedTimeBlock(const std::string &name) : blockName(name)
    {
        BeginTimeBlock(blockName);
    }
    ScopedTimeBlock::~ScopedTimeBlock()
    {
        EndTimeBlock(blockName);
    }

    void ProcessStatistics()
    {
        // 这里可以添加处理统计数据的逻辑，比如计算平均时间、最大时间等
        // 表驱动
        for (const auto &[name, timeBlock] : TimeBlocks)
        {
            auto duration = duration_cast<microseconds>(timeBlock.endTime - timeBlock.startTime);
            BlockDurations[name] = duration;
        }
    }

    void DisplayTimeAuto(const std::string &msg, long long durationCount)
    {
        if (durationCount < 1000)
        {
            // std::cout << msg << durationCount << " us" << std::endl; // 微秒
            ImGui::Text("%s: %lld us", msg.c_str(), durationCount);
        }
        else if (durationCount < 1000000)
        {
            ImGui::Text("%s: %f ms", msg.c_str(), durationCount / 1000.0f);
        }
        else
        {
            ImGui::Text("%s: %f s", msg.c_str(), durationCount / 1000000.0f);
        }
    }

    void RenderUI()
    {
        const size_t NumThreads = 16;
        int screenWidth = ImGui::GetIO().DisplaySize.x;
        int screenHeight = ImGui::GetIO().DisplaySize.y;
        ImGui::Begin("Profiler");
        for (const auto &[name, duration] : BlockDurations)
        {
            DisplayTimeAuto(std::format("{}: ", name), duration.count());
        }

        for (const auto &[name, count] : Aggregator::s_GetTotalCounterTable())
        {
            ImGui::Text("%s: %zu per Sample", name.c_str(), count);                                                             // 计数器累计
            ImGui::Text("%s: %f per Sample per Pixel", name.c_str(), static_cast<float>(count) / (screenWidth * screenHeight)); // 单位采样单位像素计数
        }

        for (const auto &[name, stats] : Aggregator::s_GetTotalTimeStatsTable())
        {
            ImGui::Text("%s Call Counts per Sample: %zu", name.c_str(), stats.callCount);                                                        // 调用次数
            DisplayTimeAuto(std::format("{} Time Costs per Sample", name), stats.totalDuration.count() / NumThreads);                            // 总耗时
            // DisplayTimeAuto(std::format("{} Time per Call", name), (stats.totalDuration.count()) / stats.callCount / NumThreads); // 平均耗时
        }

        ImGui::End();
    }

    std::unordered_map<std::string, TimeBeginEnd> TimeBlocks;
    std::unordered_map<std::string, microseconds> BlockDurations;

} // namespace Profiler