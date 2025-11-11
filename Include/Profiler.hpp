#pragma once
#include <cassert>
#include <chrono>
#include <iostream>
#include <mutex>
#include <unordered_map>

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

    // 微秒级可能难以统计每一个像素的时间花费? 会不会是因为精度不够,所以最终累积的时间和实际时间差距过大?
    struct TimeStats
    {
        high_resolution_clock::time_point timerStartTime;
        high_resolution_clock::time_point timerEndTime;
        nanoseconds totalDuration{0}; // for scopedTimer

        nanoseconds maxDuration{0}; // for total stats
        nanoseconds minDuration{0}; // for total stats
        size_t callCount{0};
    };

    // 利用RAII进行时间统计,将结果加到绑定的TimeStats中
    class ThreadScopedTimer
    {
        TimeStats &statsRef;
        // inline static thread_local high_resolution_clock::time_point startTime; // 有趣的地方. 如果一个线程有多个计时器, 这里会冲突  所以发生'测不准'
        // inline static thread_local high_resolution_clock::time_point endTime;

    public:
        ThreadScopedTimer(TimeStats &stats)
            : statsRef(stats)
        {
            statsRef.callCount++;
            statsRef.timerStartTime = high_resolution_clock::now();
        }
        ~ThreadScopedTimer()
        {
            statsRef.timerEndTime = high_resolution_clock::now();
            statsRef.totalDuration += (statsRef.timerEndTime - statsRef.timerStartTime);
        }
    };
    // 利用RAII进行时间统计,将结果加到绑定的TimeStats中,可以设置时间采样间隔
    class ThreadScopedTimeSampler
    {
        TimeStats &statsRef;
        int sampleStep;

    public:
        ThreadScopedTimeSampler(TimeStats &stats, int _sampleStep)
            : statsRef(stats), sampleStep(_sampleStep)
        {
            assert(sampleStep > 0);
            if (statsRef.callCount % sampleStep == 0)
            {
                statsRef.timerStartTime = high_resolution_clock::now();
            }
            statsRef.callCount++;
        }
        ~ThreadScopedTimeSampler()
        {
            if (statsRef.callCount % sampleStep == sampleStep - 1)
            {
                statsRef.timerEndTime = high_resolution_clock::now();
                statsRef.totalDuration += (statsRef.timerEndTime - statsRef.timerStartTime);
            }
        }
    };

    class ThreadStatsAggregator // 定义在线程入口处,
    {
        using Count = size_t;
        using Index = size_t;

        inline static std::unordered_map<std::string, Count> s_TotalCounterTable;
        inline static std::mutex s_TotalCounterTableMutex;
        inline static std::unordered_map<std::string, TimeStats> s_TotalTimeStatsTable;
        inline static std::mutex s_TotalTimeStatsTableMutex;

        inline static thread_local std::unique_ptr<ThreadStatsAggregator> th_AggregatorInstance = nullptr;

        inline static thread_local std::unordered_map<std::string, Count> th_CounterTable;
        inline static thread_local std::unordered_map<std::string, TimeStats> th_TimeStatsTable;
        inline static thread_local std::unordered_map<std::string, Count> th_CounterTableDummy;
        inline static thread_local std::unordered_map<std::string, TimeStats> th_TimeStatsTableDummy;

    public:
        ThreadStatsAggregator() // Acquire Counter
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

        // 静态调用注册函数 例如: thread_local auto& count = Profiler::Aggregator::RegisterCounter("name");
        static Count &RegisterCounter(const std::string &name)
        {
            // assert(th_AggregatorInstance);
            if(!th_AggregatorInstance) {
                return th_CounterTableDummy[name];
            }
            th_CounterTable[name] = 0;
            std::unique_lock<std::mutex> lock(s_TotalCounterTableMutex);
            s_TotalCounterTable[name] = 0;
            return th_CounterTable[name];
        }
        static TimeStats &RegisterTimeStats(const std::string &name)
        {
            // assert(th_AggregatorInstance);
            if(!th_AggregatorInstance) {
                return th_TimeStatsTableDummy[name];
            }
            th_TimeStatsTable[name] = TimeStats{};
            std::unique_lock<std::mutex> lock(s_TotalTimeStatsTableMutex);
            s_TotalTimeStatsTable[name] = TimeStats{};
            return th_TimeStatsTable[name];
        }

        static void CreateInstance()
        {
            assert(th_AggregatorInstance == nullptr); // 线程唯一
            th_AggregatorInstance = std::make_unique<ThreadStatsAggregator>();
        }

        static void Submit()
        {
            th_AggregatorInstance.reset();
        }

        static std::unordered_map<std::string, Count> &s_GetTotalCounterTable()
        {
            return s_TotalCounterTable;
        }
        static std::unordered_map<std::string, TimeStats> &s_GetTotalTimeStatsTable()
        {
            return s_TotalTimeStatsTable;
        }

        ~ThreadStatsAggregator() // Submit All Counter
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
                    if (stats.maxDuration.count() == 0 || inc.totalDuration > stats.maxDuration)
                    {
                        stats.maxDuration = inc.totalDuration;
                    }
                    if (stats.minDuration.count() == 0 || inc.totalDuration < stats.minDuration)
                    {
                        stats.minDuration = inc.totalDuration;
                    }
                    th_TimeStatsTable[name] = TimeStats{};
                }
            }
        }
    };
    // 线程汇总器RAII类,线程唯一
    class AggregatorGuard
    {
    public:
        AggregatorGuard()
        {
            ThreadStatsAggregator::CreateInstance();
        }
        ~AggregatorGuard()
        {
            ThreadStatsAggregator::Submit();
        }
    };

    // 数据收集
    extern std::unordered_map<std::string, TimeBeginEnd> TimeBlocks;

    // 数据处理后
    extern std::unordered_map<std::string, nanoseconds> BlockDurations;

    // 可以考虑改成配对的方式,Begin传入名字,这应该需要一个栈来维护
    void BeginTimeBlock(const std::string &name);
    void EndTimeBlock(const std::string &name);
    void ProcessStatistics();

    void RenderUI();
} // namespace Profiler