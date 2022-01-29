#include "vectorfont.hpp"
#include <raymath.h>
#include <map>

static const std::map<char, std::string> _font = {
    {32, "0016"},                                                                                                              // " "
    {33, "0910eueg@ebdae0faeb@"},                                                                                              // "!"
    {63, "2118cpcqdsetgukumtnsoqoonmmlijig@ibhai0jaib@"},                                                                      // "?"
    {65, "0918iua0@iuq0@dgng@"},                                                                                               // "A"
    {66, "2421dud0@dumuptqsrqroqmplmk@dkmkpjqirgrdqbpam0d0@"},                                                                 // "B"
    {67, "1921rpqrotmuiugterdpcmchdeecgai0m0oaqcre@"},                                                                         // "C"
    {68, "1621dud0@dukuntprqprmrhqepcnak0d0@"},                                                                                // "D"
    {69, "1219dud0@duqu@dklk@d0q0@"},                                                                                          // "E"
    {70, "0918dud0@duqu@dklk@"},                                                                                               // "F"
    {71, "2321rpqrotmuiugterdpcmchdeecgai0m0oaqcrerh@mhrh@"},                                                                  // "G"
    {72, "0922dud0@rur0@dkrk@"},                                                                                               // "H"
    {73, "0910eue0@cugu@c0g0@"},                                                                                               // "I"
    {74, "1116lulekbjah0f0dacbbebg@"},                                                                                         // "J"
    {75, "0921dud0@rudg@ilr0@"},                                                                                               // "K"
    {76, "0617dud0@d0p0@"},                                                                                                    // "L"
    {77, "1224dud0@dul0@tul0@tut0@"},                                                                                          // "M"
    {78, "0922dud0@dur0@rur0@"},                                                                                               // "N"
    {79, "2222iugterdpcmchdeecgai0m0oaqcreshsmrpqrotmuiu@"},                                                                   // "O"
    {80, "1421dud0@dumuptqsrqrnqlpkmjdj@"},                                                                                    // "P"
    {81, "2522iugterdpcmchdeecgai0m0oaqcreshsmrpqrotmuiu@ldr2@"},                                                              // "Q"
    {82, "1721dud0@dumuptqsrqroqmplmkdk@kkr0@"},                                                                               // "R"
    {83, "2120qrotluhuetcrcpdnemglmjoiphqfqcoal0h0eacc@"},                                                                     // "S"
    {84, "0616huh0@auou@"},                                                                                                    // "T"
    {85, "1122dudfecgaj0l0oaqcrfru@"},                                                                                         // "U"
    {86, "0618aui0@qui0@"},                                                                                                    // "V"
    {87, "1224bug0@lug0@luq0@vuq0@"},                                                                                          // "W"
    {88, "0620cuq0@quc0@"},                                                                                                    // "X"
    {89, "0718auiki0@quik@"},                                                                                                  // "Y"
    {90, "0920quc0@cuqu@c0q0@"},                                                                                               // "Z"
};

VectorFont::VectorFont()
{
}

inline int decode(char c)
{
    return (c >= 48 && c <= 57) ? -(c - 48) : c - 96;
}

float VectorFont::drawGlyph(char glyph, Vector2 position, float size, Color col)
{
    auto scale = size / 25.0f;
    auto iter = _font.find(glyph);
    if (iter != _font.end()) {
        const auto& code = iter->second;
        // auto numSteps = std::stoi(code.substr(0,2));
        auto width = std::stoi(code.substr(2, 2));
        auto si = code.begin() + 4;
        bool draw = false;
        Vector2 pos{};
        while (si < code.end()) {
            if (*si == 64) {
                draw = false;
                ++si;
            }
            else {
                auto xx = static_cast<float>(decode(*si++)) * scale;
                auto yy = static_cast<float>(decode(*si++)) * scale;
                if (draw) {
                    auto newPos = Vector2Add(position, {xx, -yy});
                    DrawLineEx(pos, newPos, size/6, col);
                    pos = newPos;
                    // context.addLine(to: CGPoint(x: CGFloat(x+xx), y: CGFloat(y-yy)))
                }
                else {
                    pos = Vector2Add(position, {xx, -yy});
                    // context.move(to: CGPoint(x: CGFloat(x+xx), y: CGFloat(y-yy)))
                    draw = true;
                }
            }
        }
        return width * scale;
    }
    return 0;
}

float VectorFont::drawString(const std::string& text, Vector2 position, float size, Color col)
{
    float start = position.x;
    for (auto c : text) {
        position.x += drawGlyph(c, position, size, col);
    }
    return position.x - start;
}

float VectorFont::textWidth(const std::string& text, float size)
{
    auto scale = size / 25.0f;
    float width{0};
    for (auto c : text) {
        auto iter = _font.find(c);
        if (iter != _font.end()) {
            width += std::stoi(iter->second.substr(2, 2)) * scale;
        }
    }
    return width;
}
