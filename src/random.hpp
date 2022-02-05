#pragma once

#include "config.hpp"
#include "math.hpp"
#include <string>

class Random
{
public:
    inline static constexpr uint32_t MaxU32Value{0x7fffffff};
    explicit Random(uint64_t initialSeed);
    explicit Random(int initialSeed);
    void seed(uint64_t newSeed);
    void seed(const std::string& newSeed);
    uint32_t randomU32();
    uint64_t randomU64();
    int randomInt(int min, int max);
    int randomIndex(int range);
    float randomFloat(float range = 1.0f);
    float randomUFloat(float range = 1.0f);
    float randomExpFloat(float lambda = 0.1f);
    Vector2 randomSphereVec2(float radius);
    Vector3 randomSphereVec3(float radius);
    //Vector2 randomVec2(const Vector2& direction, float angle, float offsetRange = 0.0f);
    Vector3 randomVec3(const Vector3& direction, float angle, float offsetRange = 0.0f);
    static Random* instance();
private:
    uint64_t _state[2]{};
};
