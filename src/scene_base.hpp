#pragma once

#include <external/msf_gif.h>
#include <raylib.h>
#include <raymath.h>

#include "gradient.hpp"
#include "scene.hpp"
#include "spritemanager.hpp"
#include "vectorfont.hpp"

#include <array>
#include <cstdlib>
#include <map>

/*
 *  .......#
 *  .......#
 *  ......##
 *  .....###
 *  .#....##
 *  .#...###
 *  .##.####
 *  .#######
 */

class SceneBase : public Scene
{
    inline static const uint8_t gfx[] = {0x03, 0x1f, 0x3f, 0x39, 0x3f, 0x06, 0x09, 0x04, 0x03, 0x1f, 0x3f, 0x39, 0x3f, 0x06, 0x0d, 0x18, 0x08, 0x04, 0x0f, 0x1b, 0x3f, 0x2f, 0x28, 0x06,
                                         0x08, 0x24, 0x2f, 0x3b, 0x3f, 0x1f, 0x08, 0x10, 0x01, 0x03, 0x07, 0x0d, 0x0f, 0x02, 0x04, 0x02, 0x01, 0x03, 0x07, 0x0d, 0x0f, 0x02, 0x05, 0x0a,

                                         0x01, 0x01, 0x03, 0x07, 0x43, 0x47, 0x6f, 0x7f};

public:
    enum SpriteId { sidPlayer = 1, sidPlayerLaser, sidColonyShip = 10, sidColonyShipEnd = 15, sidAlienLaser = 50, sidAlienBomb, sidAlienStart = 100, sidAlienEnd = 228 };
    SceneBase() = default;
    ~SceneBase() override = default;

    void loadBase()
    {
        _width = width();
        _height = height();

        if (!_initialized) {
            for (auto& star : _stars) {
                star._x = GetRandomValue(0, _width - 1);
                star._y = static_cast<float>(GetRandomValue(0, _height - 1));
                star._d = GetRandomValue(10, 255);
            }

            /*
            _spriteSheet = GenImageColor(256, 256, {0,0,0,0});
            const auto* ptr = gfx;
            for(int i = 0; i < 7; ++i) {
                for (int y = 0; y < 8; ++y, ++ptr) {
                    for (int x = 0; x < 8; ++x) {
                        if((*ptr & (1<<x))) {
                            ImageDrawPixel(&_spriteSheet, i*16 + 7 - x, y, WHITE);
                            ImageDrawPixel(&_spriteSheet, i*16 + 8 + x, y, WHITE);
                        }
                    }
                }
            }
            */
            for (int id = sidAlienStart; id < sidAlienStart + 128; ++id) {
                SpriteManager::instance()->generateAlien(id, 4);
            }
            SpriteManager::instance()->generateAlien(sidPlayer, {40646, 8, 16, true, false, false, true}, Sprite::Player);
            SpriteManager::instance()->generateAlien(sidColonyShip, {114765392, 8, 48, true, false, false, true}, Sprite::ColonyShip);
            SpriteManager::instance()->generateAlien(sidColonyShip + 1, {125945237, 8, 48, true, false, false, true}, Sprite::ColonyShip);
            SpriteManager::instance()->insertImage(sidPlayerLaser, Procedural::generateBullet(BulletType::PlayerLaser), Sprite::Type::PlayerShot);
            SpriteManager::instance()->insertImage(sidAlienLaser, Procedural::generateBullet(BulletType::AlienLaser), Sprite::Type::AlienShot);
            // SpriteManager::instance()->generateAlien(sidAlienBomb, {253101687, 5, 5, true, true, false, true}, Sprite::AlienBomb);
            SpriteManager::instance()->generateAlien(sidAlienBomb, {4903791, 5, 5, true, true, false, true}, Sprite::AlienBomb);

#ifndef NDEBUG
            SpriteManager::instance()->dump();
#endif
            /*
            Gradient grad;
            ImageDrawPixel(&_spriteSheet, 3, 8, grad.getColorAt(1.0f));
            ImageDrawPixel(&_spriteSheet, 4, 8, grad.getColorAt(1.0f));
            ImageDrawPixel(&_spriteSheet, 3, 9, grad.getColorAt(0.9f));
            ImageDrawPixel(&_spriteSheet, 4, 9, grad.getColorAt(0.9f));
            ImageDrawPixel(&_spriteSheet, 3, 10, grad.getColorAt(0.8f));
            ImageDrawPixel(&_spriteSheet, 4, 10, grad.getColorAt(0.8f));
            ImageDrawPixel(&_spriteSheet, 3, 11, grad.getColorAt(0.8f));
            ImageDrawPixel(&_spriteSheet, 4, 11, grad.getColorAt(0.8f));
            ImageDrawPixel(&_spriteSheet, 3, 12, grad.getColorAt(0.75f));
            ImageDrawPixel(&_spriteSheet, 4, 12, grad.getColorAt(0.75f));
            ImageDrawPixel(&_spriteSheet, 3, 13, grad.getColorAt(0.5f));
            ImageDrawPixel(&_spriteSheet, 4, 13, grad.getColorAt(0.5f));
            */

            _explosionColors.addMark(0.0f, {255, 255, 0, 255});
            _explosionColors.addMark(0.5f, {200, 100, 0, 255});
            _explosionColors.addMark(0.75f, {200, 200, 200, 200});
            _explosionColors.addMark(1.0f, {32, 32, 32, 20});

            _alienLaserTrailColors.addMark(0.0f, {29, 184, 29, 80});
            _alienLaserTrailColors.addMark(1.0f, {29, 184, 29, 0});
            _alienLaserTrailColors.addMark(0.0f, {29, 184, 29, 200});
            _alienLaserTrailColors.addMark(1.0f, {29, 184, 29, 0});
            _playerLaserTrailColors.addMark(0.0f, {29, 91, 184, 255});
            _playerLaserTrailColors.addMark(1.0f, {29, 91, 184, 0});
            _playerEngineTrailColors.addMark(0.0f, {10, 200, 200, 128});
            _playerEngineTrailColors.addMark(1.0f, {10, 200, 200, 0});

            for (int i = 0; i < 10; ++i) {
                _highscores.emplace(i * 10, ScoreInfo{i * 10, "---", 1, i * 2, "---"});
            }
            _initialized = true;
        }
        //_spriteTexture = LoadTextureFromImage(_spriteSheet);
        // Sprite::_image = &_spriteSheet;
        // Sprite::_texture = &_spriteTexture;
        _alpha = 1.0f;

        // SpriteManager::instance()->dump();
    }

    void unloadBackground()
    {
        // UnloadTexture(_spriteTexture);
    }

    void updateBackground(float dt_s, float timeFlow = 1.0f)
    {
        _stateTime += dt_s;
        ++_frameCount;
        for (auto& star : _stars) {
            star._y += (static_cast<float>(star._d) / 120.0f) * timeFlow;
            if (static_cast<int>(star._y) >= _height) {
                star._x = GetRandomValue(0, _width - 1);
                star._y = 0;
            }
        }
    }

    RenderMode renderMode() const override
    {
        return RenderMode::Upscale2;
    }

    void renderBackground()
    {
        ClearBackground(BasePalette[0]);
        for (auto& star : _stars) {
            DrawRectangle(star._x, int(star._y), 1, 1, {uint8_t(star._d), uint8_t(star._d), uint8_t(star._d), 255});
        }

        // DrawText("INVADE v" INVADE_VERSION_STRING_SHORT, 10, 10, 20, GREEN);
    }

protected:
    int _width{0};
    int _height{0};
    struct Star
    {
        int _x;
        float _y;
        int _d;
    };
    float _alpha = 1.0f;  // Useful for fading
    float _stateTime{0};
    VectorFont _vectorFont;
    struct ScoreInfo
    {
        uint64_t score = 0;
        std::string name;
        int level = 0;
        int colonists = 0;
        std::string planet{};
    };
    inline static bool _initialized{false};
    inline static Gradient _explosionColors;
    inline static Gradient _playerLaserTrailColors;
    inline static Gradient _playerEngineTrailColors;
    inline static Gradient _alienLaserTrailColors;
    inline static Gradient _alienBombTrailColors;
    inline static std::array<Star, 100> _stars;
    inline static ScoreInfo _lastScoreInfo{0, "", 0, 0, ""};
    inline static std::multimap<uint64_t, ScoreInfo, std::greater<>> _highscores;
};
