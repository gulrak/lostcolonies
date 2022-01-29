#pragma once

#include <raylib.h>
#include <string>

class VectorFont
{
public:
    VectorFont();
    float drawGlyph(char glyph, Vector2 position, float size, Color col);
    float drawString(const std::string& text, Vector2 position, float size, Color col);
    float textWidth(const std::string& text, float size);
};
