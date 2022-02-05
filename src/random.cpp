
#include "random.hpp"

#include <cmath>
#include <type_traits>
#include <ctime>

Random::Random(uint64_t initialSeed)
{
    seed(initialSeed);
}

Random::Random(int initialSeed)
{
    seed(static_cast<uint64_t>(initialSeed));
}

Random* Random::instance()
{
    static Random _instance{static_cast<uint64_t>(std::time(nullptr))};
    return &_instance;
}

static inline uint64_t rotl(const uint64_t x, int k) {
    return (x << k) | (x >> (64 - k));
}

static uint64_t splitmix64(uint64_t* seed) {
    uint64_t z = (*seed += UINT64_C(0x9E3779B97F4A7C15));
    z = (z ^ (z >> 30)) * UINT64_C(0xBF58476D1CE4E5B9);
    z = (z ^ (z >> 27)) * UINT64_C(0x94D049BB133111EB);
    return z ^ (z >> 31);
}

void Random::seed(uint64_t newSeed)
{
    _state[0] = splitmix64(&newSeed);
    _state[1] = splitmix64(&newSeed);
}

void Random::seed(const std::string& newSeed)
{
    uint64_t hash = UINT64_C(0x9E3779B97F4A7C15);
    for(auto c : newSeed) {
        hash = rotl(hash, 28) ^ (c * 0x47804C93);
    }
    seed(hash);
}

uint32_t Random::randomU32()
{
    return randomU64() & 0x7fffffff;
}

uint64_t Random::randomU64()
{
    const uint64_t s0 = _state[0];
    uint64_t s1 = _state[1];
    const uint64_t result = rotl(s0 + s1, 17) + s0;

    s1 ^= s0;
    _state[0] = rotl(s0, 49) ^ s1 ^ (s1 << 21); // a, b
    _state[1] = rotl(s1, 28); // c

    return result>>1; // ensure positive values if used in signed context
}

int Random::randomInt(int min, int max)
{
    if (min > max) {
        std::swap(min, max);
    }
    return (static_cast<int>(randomU32()) % (std::abs(max - min) + 1) + min);
}

float Random::randomFloat(float range)
{
    return (float(randomU32()&0xfffff)/0x80000-1.0f) * range;
}

float Random::randomUFloat(float range)
{
    return (float(randomU32()&0xfffff)/0xfffff) * range;
}

int Random::randomIndex(int range)
{
    return randomInt(0, range-1);
}

float Random::randomExpFloat(float lambda)
{
    return static_cast<float>(-std::log(1.0 - static_cast<double>(randomU32()) / (MaxU32Value + 1.0)) / lambda);
}

Vector2 Random::randomSphereVec2(float radius)
{
    float t = randomUFloat(float(2.0f * M_PI));
    float x = std::cos(t);
    float y = std::sin(t);
    return {x * radius, y * radius};
}

Vector3 Random::randomSphereVec3(float radius)
{
    float z = randomFloat();
    float t = randomUFloat(float(2.0f * M_PI));
    float x = std::sqrt(1.0f - z*z) * std::cos(t);
    float y = std::sqrt(1.0f - z*z) * std::sin(t);
    return {x * radius, y * radius, z * radius};
}

Vector3 Random::randomVec3(const Vector3& direction, float /*angle*/, float /*offsetRange*/)
{
    //return direction;
    float length = Vector3Length(direction);
    if(length < 0.1f) {
        return direction;
    }
    float least = std::fabs(direction.x);
    int axis = 0;
    if(std::fabs(direction.y) < least) {
        least = std::fabs(direction.y);
        axis = 1;
    }
    if(std::fabs(direction.z) < least) {
        least = std::fabs(direction.z);
        axis = 2;
    }
    Vector3 n1;
    return n1;
    /*
    switch(axis) {
        case 0:
            n1 = glm::normalize(glm::cross(direction, {1.0f, 0.0f, 0.0f}));
            break;
        case 1:
            n1 = glm::normalize(glm::cross(direction, {0.0f, 1.0f, 0.0f}));
            break;
        default:
            n1 = glm::normalize(glm::cross(direction, {0.0f, 0.0f, 1.0f}));
            break;
    }
    if(glm::any(glm::isnan(n1))) {
        return direction;
    }
    glm::vec3 n2 = glm::normalize(glm::cross(direction, n1));
    float a = randomf(2 * M_PI);
    float r = std::tan(angle) * length * std::sqrt(std::fabs(randomf()));
    glm::vec3 result;
    if(offsetRange > 0.0001f) {
        result = direction/length*(length+randomf(offsetRange)) + n1 * r * std::cos(a) + n2 * r * std::sin(a);
    }
    else {
        result = direction + n1 * r * std::cos(a) + n2 * r * std::sin(a);
    }
    if(glm::any(glm::isnan(result))) {
        return direction;
    }

    return result;
     */
}
