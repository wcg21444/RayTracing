#pragma once

#include <random>
#include <glm/glm.hpp>
#include <glm/gtc/random.hpp>
#include <vector>

namespace Random
{
    inline std::uniform_real_distribution<float> randomFloats(0.0, 1.0); // random floats between [0.0, 1.0]
    inline std::default_random_engine generator;
    inline glm::vec3 randomVector(float strength)
    {
        return glm::linearRand(glm::vec3(-strength), glm::vec3(strength));
    }

}