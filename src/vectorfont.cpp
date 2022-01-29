#include "vectorfont.hpp"
#include <raymath.h>
#include <map>

static const std::map<char, std::string> _font = {
    {32, "0016"},                                                                                                              // " "
    {33, "0910eueg@ebdae0faeb@"},                                                                                              // "!"
#if 0
    {34, "0616dudn@luln@"},                                                                                                    // '"'
    {35, "1221kyd7@qyj7@dlrl@cfqf@"},                                                                                          // "#"
    {36, "2720hyh4@lyl4@qrotluhuetcrcpdnemglmjoiphqfqcoal0h0eacc@"},                                                           // "$"
    {37, "3224uuc0@hujsjqiognencpcrdtfuhujtmspsstuu@qgofndnbp0r0taucuesgqg@"},                                                 // "%"
    {38, "3526wlwmvnuntmskqfocmak0g0eadbcdcfdheilmmnnpnrmtkuithrhpimkjpcrat0v0wawb@"},                                         // "&"
    {39, "0810esdteuftfrepdo@"},                                                                                               // "'"
    {40, "1114kyiwgtepdkdgebg2i5k7@"},                                                                                         // "("
    {41, "1114cyewgtipjkjgibg2e5c7@"},                                                                                         // ")"
    {42, "0916huhi@crml@mrcl@"},                                                                                               // "*"
    {43, "0626mrm0@divi@"},                                                                                                    // "+"
    {44, "0910fae0daebfaf1e3d4@"},                                                                                             // ","
    {45, "0326divi@"},                                                                                                         // "-"
    {46, "0610ebdae0faeb@"},                                                                                                   // "."
    {47, "0322tyb7@"},                                                                                                         // "/"
    {48, "2120iuftdqclciddfai0k0napdqiqlpqntkuiu@fdos@"},                                                                      // "0"
    {49, "0520fqhrkuk0@"},                                                                                                     // "1"
    {50, "1520dpdqesfthuluntospqpoommjc0q0@"},                                                                                 // "2"
    {51, "1620eupujmmmolpkqhqfpcnak0h0eadbcd@"},                                                                               // "3"
    {52, "0720mucgrg@mum0@"},                                                                                                  // "4"
    {53, "1820oueudlemhnknnmpkqhqfpcnak0h0eadbcd@"},                                                                           // "5"
    {54, "2420protlujugteqdldgecgaj0k0napcqfqgpjnlkmjmglejdg@"},                                                               // "6"
    {55, "0620qug0@cuqu@"},                                                                                                    // "7"
    {56, "3020huetdrdpengmklnkpiqgqdpboal0h0eadbcdcgdifkilmmonppprotluhu@"},                                                   // "8"
    {57, "2420pnokmijhihfidkcncodrftiujumtorpnpiodmaj0h0eadc@"},                                                               // "9"
    {58, "1210endmelfmen@ebdae0faeb@"},                                                                                        // ":"
    {59, "1510endmelfmen@fae0daebfaf1e3d4@"},                                                                                  // ";"
    {60, "0424trdit0@"},                                                                                                       // "<"
    {61, "0626dlvl@dfvf@"},                                                                                                    // "="
    {62, "0424drtid0@"},                                                                                                       // ">"
    {64, "5627rmqooplpjoinhkhhifkenepfqh@lpjnikihjfke@rpqhqfseuewgxjxlwovqtsrtouluitgseqdoclcidfedgbial0o0ratbuc@sprhrfse@"},  // "@"
#endif
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
#if 0
    {91, "1214dyd7@eye7@dyky@d7k7@"},                                                                                          // "["
    {92, "03140un3@"},                                                                                                         // "\"
    {93, "1214iyi7@jyj7@cyjy@c7j7@"},                                                                                          // "]"
    {94, "1116fohrjo@clhqml@hqh0@"},                                                                                           // "^"
    {95, "031602p2@"},                                                                                                         // "_"
    {96, "0810fuetdrdpeofpeq@"},                                                                                               // "`"
    {97, "1819ono0@okmmknhnfmdkchcfdcfah0k0maoc@"},                                                                            // "a"
    {98, "1819dud0@dkfmhnknmmokphpfocmak0h0fadc@"},                                                                            // "b"
    {99, "1518okmmknhnfmdkchcfdcfah0k0maoc@"},                                                                                 // "c"
    {100, "1819ouo0@okmmknhnfmdkchcfdcfah0k0maoc@"},                                                                           // "d"
    {101, "1818chohojnlmmknhnfmdkchcfdcfah0k0maoc@"},                                                                          // "e"
    {102, "0912juhufteqe0@bnin@"},                                                                                             // "f"
    {103, "2319ono2n5m6k7h7f6@okmmknhnfmdkchcfdcfah0k0maoc@"},                                                                 // "g"
    {104, "1119dud0@djgminlnnmojo0@"},                                                                                         // "h"
    {105, "0908cudteudvcu@dnd0@"},                                                                                             // "i"
    {106, "1210euftgufveu@fnf3e6c7a7@"},                                                                                       // "j"
    {107, "0917dud0@nndd@hho0@"},                                                                                              // "k"
    {108, "0610cueuebfag0@"},                                                                                                  // "l"
    {109, "1930dnd0@djgminlnnmojo0@ojrmtnwnymzjz0@"},                                                                          // "m"
    {110, "1119dnd0@djgminlnnmojo0@"},                                                                                         // "n"
    {111, "1819hnfmdkchcfdcfah0k0maocpfphokmmknhn@"},                                                                          // "o"
    {112, "1819dnd7@dkfmhnknmmokphpfocmak0h0fadc@"},                                                                           // "p"
    {113, "1819ono7@okmmknhnfmdkchcfdcfah0k0maoc@"},                                                                           // "q"
    {114, "0913dnd0@dhekgminln@"},                                                                                             // "r"
    {115, "1817nkmmjngndmckdifhkgmfndncmaj0g0dacc@"},                                                                          // "s"
    {116, "0912euedfah0j0@bnin@"},                                                                                             // "t"
    {117, "1119dnddeag0j0laod@ono0@"},                                                                                         // "u"
    {118, "0616bnh0@nnh0@"},                                                                                                   // "v"
    {119, "1222cng0@kng0@kno0@sno0@"},                                                                                         // "w"
    {120, "0617cnn0@nnc0@"},                                                                                                   // "x"
    {121, "1016bnh0@nnh0f4d6b7a7@"},                                                                                           // "y"
    {122, "0917nnc0@cnnn@c0n0@"},                                                                                              // "z"
    {123, "4014iygxfweuesfqgphnhlfj@gxfvftgrhqioimhkdihgieichag0f2f4g6@fhhfhdgbfae1e3f5g6i7@"},                                // "{"
    {124, "0308dyd7@"},                                                                                                        // "|"
    {125, "4014eygxhwiuishqgpfnflhj@gxhvhtgrfqeoemfkjifgeeecfag0h2h4g6@hhfffdgbhai1i3h5g6e7@"},                                // "}"
    {126, "2424cfchdkflhljknhpgrgthuj@chdjfkhkjjngpfrftgujul@"}                                                                // "~"
#endif
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
