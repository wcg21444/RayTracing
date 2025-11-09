#include "Profiler.hpp"
#include "UICommon.hpp"
#include "RenderState.hpp"

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

    // Micro version
    //  void DisplayTimeAuto(const std::string &msg, long long durationCount)
    //  {
    //      if (durationCount < 1000)
    //      {
    //          // std::cout << msg << durationCount << " us" << std::endl; // 微秒
    //          ImGui::Text("%s: %lld us", msg.c_str(), durationCount);
    //      }
    //      else if (durationCount < 1000000)
    //      {
    //          ImGui::Text("%s: %f ms", msg.c_str(), durationCount / 1000.0f);
    //      }
    //      else
    //      {
    //          ImGui::Text("%s: %f s", msg.c_str(), durationCount / 1000000.0f);
    //      }
    //  }

    // Nano version
    void DisplayTimeAuto(const std::string &msg, long long nanoDuration)
    {
        const int MircosInNs = 1000;
        const int MillisInNs = 1000000;
        const int SecondsInNs = 1000000000;
        if (nanoDuration < MircosInNs)
        {
            ImGui::Text("%-10lld ns - %-40s", nanoDuration, msg.c_str()); // 纳秒
        }
        else if (nanoDuration < MillisInNs)
        {
            ImGui::Text("%-10f us - %-40s", nanoDuration / static_cast<float>(MircosInNs), msg.c_str()); // 微秒
        }
        else if (nanoDuration < SecondsInNs)
        {
            ImGui::Text("%-10f ms - %-40s", nanoDuration / static_cast<float>(MillisInNs), msg.c_str()); // 毫秒
        }
        else
        {
            ImGui::Text("%-10f s - %-40s", nanoDuration / static_cast<float>(SecondsInNs), msg.c_str()); // 秒
        }
    }

    void RenderUI()
    {
        float screenWidth = ImGui::GetIO().DisplaySize.x;
        float screenHeight = ImGui::GetIO().DisplaySize.y;
        ImGui::Begin("Profiler");
        for (const auto &[name, duration] : BlockDurations)
        {
            DisplayTimeAuto(name, duration.count());
        }
        ImGui::SeparatorText("Aggregated Stats");

        for (const auto &[name, count] : ThreadStatsAggregator::s_GetTotalCounterTable())
        {
            if (ImGui::TreeNode(std::format("{}##Counter", name).c_str())) // 根节点
            {
                ImGui::Text("%s: %zu per Sample", name.c_str(), count);                                                             // 计数器累计
                ImGui::Text("%s: %f per Sample per Pixel", name.c_str(), static_cast<float>(count) / (screenWidth * screenHeight)); // 单位采样单位像素计数
                ImGui::TreePop();
            }
        }

        for (const auto &[name, stats] : ThreadStatsAggregator::s_GetTotalTimeStatsTable())
        {
            if (ImGui::TreeNode(std::format("{}##TimeStats", name).c_str())) // 根节点
            {
                ImGui::Text("Call Counts per Sample: %zu", stats.callCount);                                         // 调用次数
                DisplayTimeAuto(std::format("Avg Time Costs per Sample"), stats.totalDuration.count() / RenderState::CPUNumThreads); // 平均耗时
                DisplayTimeAuto(std::format("Max Time Costs per Sample"), stats.maxDuration.count());                // 最大耗时
                DisplayTimeAuto(std::format("Min Time Costs per Sample"), stats.minDuration.count());                // 最小耗时
                ImGui::TreePop();
            }
        }

        ImGui::End();
    }

    std::unordered_map<std::string, TimeBeginEnd> TimeBlocks;
    std::unordered_map<std::string, nanoseconds> BlockDurations;

} // namespace Profiler