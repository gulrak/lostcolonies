#pragma once

#include <raylib.h>

#include "gradient.hpp"
#include "math.hpp"
#include "random.hpp"

template <typename Vector_t = Vector2>
struct Particle
{
    inline static const float CONST_GRAVITY{1.0f};
    enum Flags {
        eGRAVITY = 1,
        eBOUNCE = 2,
        eSPLASH = 4,
        eFADE = 8,
        eHIT = 16,
        eEXPLODE = 32,
        eMUTATE = 64,
        eSWING = 128,
        eSTATIC = 256,
        ePLAYER = 512,
    };
    Flags flags;
    Vector_t pos;
    Vector_t vel;
    Vector_t projected;
    Color col{};
    const Gradient* gradient{nullptr};
    uint32_t age{0};
    uint32_t maxage{0};
    inline static Random _rnd{1234};

    Particle(Flags flags, const Vector_t& pos, const Vector_t& vel, const Color& col = {255, 255, 255, 255}, uint32_t maxAge = 1000)
        : flags(flags)
        , pos(pos)
        , vel(vel)
        , col(col)
        , age(0)
        , maxage(maxAge)
    {
    }

    ~Particle() = default;

    bool isActive() const { return age >= maxage; }

    bool update(float dt);
    void draw() const;
};

template <>
inline bool Particle<Vector2>::update(float dt)
{
    if (flags & eSTATIC) {
        return true;
    }
    if (maxage) {
        age += static_cast<uint32_t>(dt * 1000);  // increment aging by milliseconds
        if (age > maxage) {
            return false;
        }
    }
    if (flags & eGRAVITY) {
        vel.y += -CONST_GRAVITY * dt;
    }
    pos += vel * dt;
    if (maxage && gradient && (flags & eFADE)) {
        col = gradient->getColorAt(static_cast<float>(age) / static_cast<float>(maxage));
    }
    return true;
}

template <>
inline void Particle<Vector2>::draw() const
{
    DrawRectangle(pos.x + 0.5f, pos.y + 0.5f, 1, 1, col);
}

template <>
inline void Particle<Vector3>::draw() const
{
}

using Particle2 = Particle<Vector2>;