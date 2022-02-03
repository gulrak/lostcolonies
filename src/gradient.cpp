//
// Created by Steffen Schümann on 21.04.20.
//

#include "gradient.hpp"

#include <algorithm>
#include <cmath>

Gradient::Gradient() noexcept try
{
    addMark(0.0f, {0, 0, 0, 255});
    addMark(1.0f, {255, 255, 255, 255});
}
catch(...) {
    // just leave it empty if something goes wrong
}

Gradient::~Gradient() = default;

void Gradient::addMark(float position, Color color)
{
    position = std::clamp(position, 0.0f, 1.0f);
    removeMark(position);
    _marks.push_back({color, position});
    refreshCache();
}

void Gradient::removeMark(float position)
{
    _marks.remove_if([position](GradientMark& gm) {
        return std::fabs(position - gm.position) < 0.0001f;
    });
    refreshCache();
}

const Color& Gradient::getColorAt(float position) const
{
    position = std::clamp(position, 0.0f, 1.0f);
    int cachePos = int(position * 255);
    return _cachedValues[cachePos];
}

void Gradient::computeColorAt(float position, Color& color) const
{
    position = std::clamp(position, 0.0f, 1.0f);

    const GradientMark* lower = nullptr;
    const GradientMark* upper = nullptr;

    for (const auto& mark : _marks) {
        if (mark.position < position) {
            if (!lower || lower->position < mark.position) {
                lower = &mark;
            }
        }

        if (mark.position >= position) {
            if (!upper || upper->position > mark.position) {
                upper = &mark;
            }
        }
    }

    if (upper && !lower) {
        lower = upper;
    }
    else if (!upper && lower) {
        upper = lower;
    }
    else if (!lower && !upper) {
        color = {0, 0, 0, 255};
        return;
    }

    if (upper == lower) {
        color = upper->color;
    }
    else {
        float distance = upper->position - lower->position;
        float delta = (position - lower->position) / distance;

        // lerp
        color.r = std::clamp(((1.0f - delta) * lower->color.r) + ((delta)*upper->color.r), 0.0f, 255.0f);
        color.g = std::clamp(((1.0f - delta) * lower->color.g) + ((delta)*upper->color.g), 0.0f, 255.0f);
        color.b = std::clamp(((1.0f - delta) * lower->color.b) + ((delta)*upper->color.b), 0.0f, 255.0f);
        color.a = std::clamp(((1.0f - delta) * lower->color.a) + ((delta)*upper->color.a), 0.0f, 255.0f);
    }
}

void Gradient::refreshCache()
{
    _marks.sort([](const GradientMark& a, const GradientMark& b) { return a.position < b.position; });
    for (int i = 0; i < 256; ++i) {
        computeColorAt(float(i) / 255.0f, _cachedValues[i]);
    }
}
