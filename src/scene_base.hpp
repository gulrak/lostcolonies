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

            for (int id = sidAlienStart; id < sidAlienStart + 128; ++id) {
                SpriteManager::instance()->generateAlien(id, 4);
            }
            SpriteManager::instance()->generateAlien(sidPlayer, {40646, 8, 16, true, false, false, true}, Sprite::Player);
            SpriteManager::instance()->generateAlien(sidColonyShip, {114765392, 8, 48, true, false, false, true}, Sprite::ColonyShip);
            SpriteManager::instance()->generateAlien(sidColonyShip + 1, {125945237, 8, 48, true, false, false, true}, Sprite::ColonyShip);
            SpriteManager::instance()->insertImage(sidPlayerLaser, Procedural::generateBullet(BulletType::PlayerLaser), Sprite::Type::PlayerShot);
            SpriteManager::instance()->insertImage(sidAlienLaser, Procedural::generateBullet(BulletType::AlienLaser), Sprite::Type::AlienShot);
            SpriteManager::instance()->generateAlien(sidAlienBomb, {4903791, 5, 5, true, true, false, true}, Sprite::AlienBomb);

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

            for (unsigned short i = 0; i < 10; ++i) {
                _highscores.emplace(i * 10, ScoreInfo{i * 10u, "---", 1, i * 2, "---"});
            }
            _initialized = true;
        }
         _alpha = 1.0f;

#ifndef NDEBUG
         SpriteManager::instance()->dump();
#endif
    }

    void unloadBackground()
    {
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
    inline static std::multimap<uint64_t, ScoreInfo, std::greater<>> _highscores;
};
