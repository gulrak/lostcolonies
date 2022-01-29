#pragma once

#include <raylib.h>
#include <list>

struct GradientMark
{
    Color color;
    float position; //0 to 1
};

class Gradient
{
public:
    Gradient();
    ~Gradient();

    const Color& getColorAt(float position) const;
    void addMark(float position, Color color);
    void removeMark(float position);
    void refreshCache();
    std::list<GradientMark> & getMarks() { return _marks; }

private:
    void computeColorAt(float position, Color& color) const;
    std::list<GradientMark> _marks;
    Color _cachedValues[256];
};

