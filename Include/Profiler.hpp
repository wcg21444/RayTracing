#pragma once
#include <unordered_map>
#include <chrono>
#include <mutex>
#include <cassert>
#include <iostream>
namespace Profiler
{
    // 用两个方法, 划分开始结束区间 , 监测区间内代码耗时,每个区间用户自定义名字
    // 输出区间耗时到imgui
    using namespace std::chrono;

    class ScopedTimeBlock
    {
    public:
        ScopedTimeBlock(const std::string &name);
        ~ScopedTimeBlock();

    private:
        std::string blockName;
    };
    struct TimeBeginEnd
    {
        high_resolution_clock::time_point startTime;
        high_resolution_clock::time_point endTime;
    };

    struct TimeStats
    {
        microseconds totalDuration{0};
        size_t callCount{0};
    };

    class ScopedTimer
    {
        TimeStats &statsRef;
        inline static thread_local high_resolution_clock::time_point startTime;
        inline static thread_local high_resolution_clock::time_point endTime;

    public:
        ScopedTimer(TimeStats &stats) : statsRef(stats)
        {
            statsRef.callCount++;
            startTime = high_resolution_clock::now();
        }
        ~ScopedTimer()
        {
            endTime = high_resolution_clock::now();
            statsRef.totalDuration += duration_cast<microseconds>(endTime - startTime);
        }
    };

    class Aggregator // 定义在线程入口处,
    {
        using Count = size_t;
        using Index = size_t;

        inline static std::unordered_map<std::string, Count> s_TotalCounterTable;
        inline static std::mutex s_TotalCounterTableMutex;
        inline static std::unordered_map<std::string, TimeStats> s_TotalTimeStatsTable;
        inline static std::mutex s_TotalTimeStatsTableMutex;

        inline static thread_local std::unique_ptr<Aggregator> th_Instance = nullptr;

        inline static thread_local std::unordered_map<std::string, Count> th_CounterTable;
        inline static thread_local std::unordered_map<std::string, TimeStats> th_TimeStatsTable;

    public:
        Aggregator() // Acquire Counter
        {
            // Clear Thread Total Count Data
            for (auto &[name, cnt] : s_TotalCounterTable)
            {
                cnt = 0;
            }
            // Clear Thread Total Time Stats
            for (auto &[name, stats] : s_TotalTimeStatsTable)
            {
                stats = TimeStats{};
            }
        }

        static void CreateInstance()
        {
            if (th_Instance == nullptr)
            {
                th_Instance = std::make_unique<Aggregator>();
            }
        }

        // 静态调用注册函数 例如: thread_local auto& count = Profiler::Aggregator::RegisterCounter("name");
        static Count &RegisterCounter(const std::string &name)
        {
            assert(th_Instance);
            th_CounterTable[name] = 0;
            std::unique_lock<std::mutex> lock(s_TotalCounterTableMutex);
            s_TotalCounterTable[name] = 0;
            return th_CounterTable[name];
        }
        static TimeStats &RegisterTimeStats(const std::string &name)
        {
            assert(th_Instance);
            th_TimeStatsTable[name] = TimeStats{};
            std::unique_lock<std::mutex> lock(s_TotalTimeStatsTableMutex);
            s_TotalTimeStatsTable[name] = TimeStats{};
            return th_TimeStatsTable[name];
        }

        static void Submit()
        {
            th_Instance.reset();
        }

        static std::unordered_map<std::string, Count> &s_GetTotalCounterTable()
        {
            return s_TotalCounterTable;
        }
        static std::unordered_map<std::string, TimeStats> &s_GetTotalTimeStatsTable()
        {
            return s_TotalTimeStatsTable;
        }

        ~Aggregator() // Submit All Counter
        {
            {
                std::unique_lock<std::mutex> lock(s_TotalCounterTableMutex);
                for (auto &[name, cnt] : s_TotalCounterTable)
                {
                    Count inc = th_CounterTable[name];
                    cnt += inc;
                    th_CounterTable[name] = 0;
                }
            }
            {
                std::unique_lock<std::mutex> lock(s_TotalTimeStatsTableMutex);
                for (auto &[name, stats] : s_TotalTimeStatsTable)
                {
                    TimeStats inc = th_TimeStatsTable[name];
                    stats.callCount += inc.callCount;
                    stats.totalDuration += inc.totalDuration;
                    th_TimeStatsTable[name] = TimeStats{};
                }
            }
        }
    };
    // 线程汇总器RAII类
    class AggregatorGuard
    {
    public:
        AggregatorGuard()
        {
            Aggregator::CreateInstance();
        }
        ~AggregatorGuard()
        {
            Aggregator::Submit();
        }
    };

    // 数据收集
    extern std::unordered_map<std::string, TimeBeginEnd> TimeBlocks;

    // 数据处理后
    extern std::unordered_map<std::string, microseconds> BlockDurations;

    // 可以考虑改成配对的方式,Begin传入名字,这应该需要一个栈来维护
    void BeginTimeBlock(const std::string &name);
    void EndTimeBlock(const std::string &name);
    void ProcessStatistics();

    void RenderUI();
} // namespace Profiler