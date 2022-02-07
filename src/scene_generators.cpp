#include "config.hpp"
#include "scene_generators.hpp"

#include <ctime>
#include <vector>

#define RAYGUI_IMPLEMENTATION
#include "raygui.hpp"


DISABLE_WARNING_PUSH

DISABLE_WARNING_PEDANTIC

#if 0
static std::vector<GuiDMProperty> _shipgenProperties{
        PBOOL("Bool", 0, true),
        PSECTION("#102#SECTION", 0, 2),
        PINT("Int", 0, 123),
        PFLOAT("Float", 0, 0.99f),
        PTEXT("Text", 0, (char*)&(char[30]){"Hello!"}, 30),
        PSELECT("Select", 0, "ONE;TWO;THREE;FOUR", 0),
        PINT_RANGE("Int Range", 0, 32, 1, 0, 100),
        PRECT("Rect", 0, 0, 0, 100, 200),
        PVEC2("Vec2", 0, 20, 20),
        PVEC3("Vec3", 0, 12, 13, 14),
        PVEC4("Vec4", 0, 12, 13, 14, 15),
        PCOLOR("Color", 0, 0, 255, 0, 255),
};
#endif

static std::vector<GuiDMProperty> _soundProperties{
    PINT_RANGE("Seed", 0, 1234, 1, 1, 0x7fffffff),
    PSELECT("Wave", 0, "#126#Square;#127#Sawtooth;#125#Sinewave;#124#Noise", 0),
    PSECTION("Envelope", 0, 4),
    PFLOAT_RANGE("Attack Time", 0, 0.0f, 0.01f, 2, 0.0f, 1.0f),
    PFLOAT_RANGE("Sustain Time", 0, 0.3f, 0.01f, 2, 0.0f, 1.0f),
    PFLOAT_RANGE("Sustain Punch", 0, 0.0f, 0.01f, 2, 0.0f, 1.0f),
    PFLOAT_RANGE("Decay Time", 0, 0.4f, 0.01f, 2, 0.0f, 1.0f),
    PSECTION("Frequency", 0, 6),
    PFLOAT_RANGE("Start", 0, 0.3f, 0.01f, 2, 0.0f, 1.0f),
    PFLOAT_RANGE("Min", 0, 0.0f, 0.01f, 2, 0.0f, 1.0f),
    PFLOAT_RANGE("Slide", 0, 0.0f, 0.01f, 2, -1.0f, 1.0f),
    PFLOAT_RANGE("delta Slide", 0, 0.0f, 0.01f, 2, -1.0f, 1.0f),
    PFLOAT_RANGE("Vibrato Depth", 0, 0.0f, 0.01f, 2, 0.0f, 1.0f),
    PFLOAT_RANGE("Vibrato Speed", 0, 0.0f, 0.01f, 2, 0.0f, 1.0f),
    PSECTION("Tone Change", 0, 2),
    PFLOAT_RANGE("Amount", 0, 0.0f, 0.01f, 2, -1.0f, 1.0f),
    PFLOAT_RANGE("Speed", 0, 0.0f, 0.01f, 2, 0.0f, 1.0f),
    PSECTION("Square Wave", 0, 2),
    PFLOAT_RANGE("Duty", 0, 0.0f, 0.01f, 2, 0.0f, 1.0f),
    PFLOAT_RANGE("Sweep", 0, 0.0f, 0.01f, 2, -1.0f, 1.0f),
    PSECTION("Repeat", 0, 1),
    PFLOAT_RANGE("Speed", 0, 0.0f, 0.01f, 2, 0.0f, 1.0f),
    PSECTION("Phaser", 0, 2),
    PFLOAT_RANGE("Offset", 0, 0.0f, 0.01f, 2, -1.0f, 1.0f),
    PFLOAT_RANGE("Sweep", 0, 0.0f, 0.01f, 2, -1.0f, 1.0f),
    PSECTION("Filter", 0, 5),
    PFLOAT_RANGE("LP Cutoff", 0, 1.0f, 0.01f, 2, 0.0f, 1.0f),
    PFLOAT_RANGE("LP Sweep", 0, 0.0f, 0.01f, 2, -1.0f, 1.0f),
    PFLOAT_RANGE("LP Resonance", 0, 0.0f, 0.01f, 2, 0.0f, 1.0f),
    PFLOAT_RANGE("HP Cutoff", 0, 0.0f, 0.01f, 2, 0.0f, 1.0f),
    PFLOAT_RANGE("HP Sweep", 0, 0.0f, 0.01f, 2, -1.0f, 1.0f),
};

static std::vector<GuiDMProperty> _spaceshipProperties{
    PINT_RANGE("Seed", 0, 1234, 1, 1, 0x7fffffff),
    PSECTION("Generator Size", 0, 2),
    PINT_RANGE("Width", 0, 6, 1, 2, 32),
    PINT_RANGE("Height", 0, 12, 1, 2, 64),
    PSECTION("Mirroring", 0, 2),
    PBOOL("Horizontal", 0, true),
    PBOOL("Vertical", 0, false),
    PSECTION("Filter", 0, 1),
    PBOOL("1 Pixel", 0, true),
    PSECTION("Player Colors", 0, 1),
    PBOOL("Force", 0, false),
};

static std::vector<GuiDMProperty> _bulletProperties{
    PINT_RANGE("Seed", 0, 1234, 1, 1, 0x7fffffff),
    PINT_RANGE("Width", 0, 8, 1, 4, 16),
    PINT_RANGE("Height", 0, 8, 1, 4, 16),
};

DISABLE_WARNING_POP

GeneratorsScene::GeneratorsScene()
: _random(uint64_t(std::time(nullptr)))
{
    GuiLoadStyleDefault();
    GuiSetStyle(LISTVIEW, LIST_ITEMS_HEIGHT, 24);
    GuiSetStyle(LISTVIEW, SCROLLBAR_WIDTH, 12);
}

GeneratorsScene::~GeneratorsScene()
{
}

void GeneratorsScene::render()
{
    //renderBackground();
    static std::vector<GuiDMProperty>* generatorList[] = { &_soundProperties, &_spaceshipProperties, &_bulletProperties };
    static int toggleGroupActive = 1;
    auto guiWidth = 220.0f;
    auto gridWidth = width() - guiWidth;
    //auto gridHeight = height() - 128;
    DrawRectangle(0, 0, gridWidth, height(), Scene::BasePalette[0]);
    toggleGroupActive = GuiToggleGroup({ width() - guiWidth, 0, guiWidth/3, 25 }, "#124#Sound;#152#Alien;#145#Bullet", toggleGroupActive);
    if(toggleGroupActive == 1) {
        auto w = _spaceshipProperties[2].value.vint.val * (_spaceshipProperties[5].value.vbool ? 2 : 1);
        auto h = _spaceshipProperties[3].value.vint.val * (_spaceshipProperties[6].value.vbool ? 2 : 1);
        GuiGrid({0,0,(float)(w*_pixelCellSize),(float)(h*_pixelCellSize)}, (float)_pixelCellSize*4, 4);
        DrawRectangleLines(0,0,(float)(w*_pixelCellSize), (float)(h*_pixelCellSize),{123,146,157,255});
    }
//    DrawLine(0, gridHeight, gridWidth, gridHeight, {123,146,157,255});
    if(GuiButton({width() - guiWidth, 25, guiWidth, 25}, "Regenerate")) {
        switch(toggleGroupActive) {
            case 0:
                break;
            case 1:
                if(_currentSpaceship.data) {
                    UnloadImage(_currentSpaceship);
                }
                _spaceshipProperties[0].value.vint.val = _random.randomInt(1, 0x0fffffff);
                _spaceshipParams = {_spaceshipProperties[0].value.vint.val, _spaceshipProperties[2].value.vint.val, _spaceshipProperties[3].value.vint.val, _spaceshipProperties[5].value.vbool, _spaceshipProperties[6].value.vbool, _spaceshipProperties[8].value.vbool, _spaceshipProperties[10].value.vbool};
                _currentSpaceship = Procedural::generateSpaceship(_spaceshipParams, &_colorsUsedForSpaceship);
                break;
            default:
                break;
        }
    }
    switch(toggleGroupActive) {
        case 1:
            if(_currentSpaceship.data) {
                renderImage(_currentSpaceship, 0, 0, _pixelCellSize);
                renderImage(_currentSpaceship, 8, _pixelCellSize * _spaceshipProperties[3].value.vint.val * (_spaceshipProperties[6].value.vbool ? 2 : 1) + 10, 2);
                DrawText(TextFormat("%d colors used", _colorsUsedForSpaceship), _pixelCellSize * _spaceshipProperties[2].value.vint.val * (_spaceshipProperties[5].value.vbool ? 2 : 1) + 8, 10, 10, WHITE);
            }
            break;
        default:
            break;
    }
    auto& prop = *(generatorList[toggleGroupActive]);
    GuiDMPropertyList({width() - guiWidth, 25*2, guiWidth, height() - 25*2}, prop.data(), (int)prop.size(), &_focus, &_scroll);
    //if (prop[0].value.vbool >= 1)
    //{
    //    DrawText(TextFormat("FOCUS:%i | SCROLL:%i | FPS:%i", _focus, _scroll, GetFPS()), prop[8].value.v2.x, prop[8].value.v2.y, 20, prop[11].value.vcolor);
    //}
    //DrawTextureRec(_spriteTexture, {0, 0, 16, 8}, {100, 100}, {0,255,0,255});
}

void GeneratorsScene::renderImage(Image& img, int xpos, int ypos, int scale)
{
    auto offset = scale > 4 ? 1 : 0;
    auto size = scale > 4 ? scale - 1 : scale;
    if(img.data) {
        for (int y = 0; y < img.height; ++y) {
            for (int x = 0; x < img.width; ++x) {
                DrawRectangle(x * scale + xpos + offset, y * scale + ypos + offset, size, size, GetImageColor(_currentSpaceship, x, y));
            }
        }
    }
}