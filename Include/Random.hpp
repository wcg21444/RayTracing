#pragma once

#include <random>
#include <glm/glm.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtc/constants.hpp>
#include <vector>

namespace Random
{
    inline std::uniform_real_distribution<float> randomFloats(0.0, 1.0); // random floats between [0.0, 1.0]
    inline std::default_random_engine generator;
    inline glm::vec3 RandomVector(float strength)
    {
        return glm::linearRand(glm::vec3(-strength), glm::vec3(strength));
    }
    inline glm::vec3 GenerateSemiSphereVector(glm::vec3 normal)
    {
        // Generate a random vector in the full sphere
        float u1 = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
        float u2 = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);

        float theta = 2.0f * glm::pi<float>() * u1;
        float phi = glm::acos(2.0f * u2 - 1.0f);

        float x = glm::sin(phi) * glm::cos(theta);
        float y = glm::sin(phi) * glm::sin(theta);
        float z = glm::cos(phi);

        glm::vec3 randomVec = glm::vec3(x, y, z);

        // Make sure the random vector is in the hemisphere defined by the normal
        if (glm::dot(randomVec, normal) < 0.0f)
        {
            randomVec = -randomVec;
        }

        return randomVec;
    }

    inline glm::vec3 GenerateCosineSemiSphereVector(const glm::vec3 &normal)
    {
        std::uniform_real_distribution<float> dist(0.0f, 1.0f);

        float u1 = dist(generator);
        float u2 = dist(generator);

        // 将均匀分布的随机数映射到圆盘上
        float r = glm::sqrt(u1);
        float theta = 2.0f * glm::pi<float>() * u2;

        float x = r * glm::cos(theta);
        float y = r * glm::sin(theta);
        float z = glm::sqrt(1.0f - u1); // 余弦加权的z坐标

        glm::vec3 local_dir = glm::vec3(x, y, z);

        // 2. 构建TBN矩阵，将向量从切线空间转换到世界空间
        glm::vec3 T, B;
        if (glm::abs(normal.z) < 0.99f)
        {
            T = glm::normalize(glm::cross(normal, glm::vec3(0.0f, 0.0f, 1.0f)));
            B = glm::cross(T, normal);
        }
        else
        {
            T = glm::normalize(glm::cross(normal, glm::vec3(1.0f, 0.0f, 0.0f)));
            B = glm::cross(T, normal);
        }
        glm::mat3 TBN_matrix = glm::mat3(T, B, normal);
        return TBN_matrix * local_dir;
    }

    inline float RussianRoulette(float p)
    {
        std::uniform_real_distribution<float> dist(0.0f, 1.0f);
        return dist(generator) < p ? 1.0f / p : 0.0f;
    }
}