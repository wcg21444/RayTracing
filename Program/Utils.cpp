#include <fstream>
#include <iomanip>
#include <filesystem>
#include <chrono>
#include <unordered_map>

#include <Windows.h>
#include <crtdbg.h>

#include "UICommon.hpp"

#include "Utils.hpp"
void SimplifiedData::DumpFlatFloatData(const float* data, size_t size, std::string path)
{
    std::ofstream outfile;
    outfile.open(path);
    // 0th 不换行. 输出位数对齐
    for (size_t i = 0; i < size; ++i)
    {
        // outfile << std::fixed << std::setprecision(8) << data[i]<<'f';
        outfile << data[i] << 'f';
        if (i < size - 1)
            outfile << ',';
        outfile << std::setw(12);
        if ((i + 1) % 40 == 0)
        {
            outfile << '\n';
        }
    }
    outfile.close();
}

std::string SimplifiedData::DumpFlatFloatDataString(const float* data, size_t size)
{
    std::stringstream outString;
    // 0th 不换行. 输出位数对齐
    for (size_t i = 0; i < size; ++i)
    {
        // outString << std::fixed << std::setprecision(8) << data[i]<<'f';
        outString << data[i] << 'f';
        if (i < size - 1)
            outString << ',';
        outString << std::setw(12);
        if ((i + 1) % 40 == 0)
        {
            outString << '\n';
        }
    }
    return outString.str();
}

bool Output::CreateParentDirectories(const std::string& filepath)
{
    std::filesystem::path p(filepath);
    std::filesystem::path parent_dir = p.parent_path();

    if (!parent_dir.empty() && !std::filesystem::exists(parent_dir))
    {
        try
        {
            std::filesystem::create_directories(parent_dir);
            // std::cerr << "Created directory: " << parent_dir << std::endl;
            return true;
        }
        catch (const std::filesystem::filesystem_error& e)
        {
            std::cerr << "Failed to create directory " << parent_dir << ": " << e.what() << std::endl;
            return false;
        }
    }
    return true; // 目录已存在或路径无父目录
}

void Output::ExportShaderSource(const std::string& filename, const std::string& source, bool readonly = true)
{
    // 尝试创建父目录。如果失败，立即返回。
    if (!Output::CreateParentDirectories(filename))
    {
        std::cerr << "Cannot export shader source due to directory creation failure." << std::endl;
        return;
    }

    DWORD attributes = GetFileAttributesA(filename.c_str());
    if (attributes != INVALID_FILE_ATTRIBUTES)
    {
        SetFileAttributesA(filename.c_str(), attributes & ~FILE_ATTRIBUTE_READONLY);
    }
    else
    {
        throw std::runtime_error("Failed to Read File Attribute");
    }
    int status = std::remove(filename.c_str());

    std::ofstream file(filename);

    if (file.is_open())
    {
        file << source;
        file.close();
        std::cerr << "Shader source exported to " << filename << std::endl;
        DWORD attributes = GetFileAttributesA(filename.c_str());
        if (attributes != INVALID_FILE_ATTRIBUTES)
        {
            SetFileAttributesA(filename.c_str(), attributes | FILE_ATTRIBUTE_READONLY);
        }
        else
        {
            throw std::runtime_error("Failed to Read File Attribute");
        }
    }
    else
    {
        // std::cerr << "Failed to export shader source to " << filename << std::endl;
        throw std::runtime_error("Failed to export shader source to " + filename);
    }
}

std::string Output::GetFilenameNoExtension(const std::string& path_str)
{
    std::filesystem::path p(path_str);
    return p.stem().string();
}

namespace Profiler
{
    // 用两个方法, 划分开始结束区间 , 监测区间内代码耗时,每个区间用户自定义名字

    std::unordered_map<std::string, TimeBeginEnd> TimeBlocks;

    void Profiler::BeginTimeBlock(const std::string& name)
    {
        TimeBlocks[name].startTime = high_resolution_clock::now();
        TimeBlocks[name].endTime = TimeBlocks[name].startTime;
    }

    void Profiler::EndTimeBlock(const std::string& name)
    {
        TimeBlocks[name].endTime = high_resolution_clock::now();
    }

    void RenderUI()
    {
        ImGui::Begin("Profiler");
        for (const auto& [name, timeBlock] : TimeBlocks)
        {
            auto duration = duration_cast<milliseconds>(timeBlock.endTime - timeBlock.startTime).count();
            ImGui::Text("%s: %lld ms", name.c_str(), duration);
        }
        ImGui::End();
    }

}

std::string GetCurrentWorkingDirectory()
{
    return std::filesystem::current_path().string();
}
