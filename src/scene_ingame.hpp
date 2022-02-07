#pragma once

#include "random.hpp"
#include "scene_base.hpp"
#include "sound.hpp"

#include <extras/easings.h>

#include <cmath>
#include <vector>

/*
 *
 *      A  A  A  A  A  A  A
 *      B B B B B B B B B B
 *       C C C C C C C C C
 *      E E E E E E E E E E
 *
 *
 */

class IngameScene : public SceneBase
{
public:
    enum class State { Init, Playing, LevelComplete, Killed, GameOver };
    static constexpr float InitStateFreezeTime = 2.0f;
    static constexpr float InitStateTime = InitStateFreezeTime + 6.0f;
    static constexpr float LevelCompleteStateTime = 6.0f;
    static constexpr float KilledStateTime = 4.0f;
    static constexpr float GameOverStateTime = 10.0f;

    IngameScene()
    {
        _projectiles.reserve(100);
        _particles.reserve(100000);
    }
    ~IngameScene() override = default;

    void load() override
    {
        loadBase();
        _score = 0;
        _playerLives = 3;
        _colonistsSaved = 0;
        _alpha = 1.0f;
        _levelSequenceRandom.seed(3456);
        setupLevel();
    }

    void setupLevel()
    {
        ++_level;
        auto offset = (width() - 15 * 20) / 2;
        for (int row = 0; row < 6; ++row) {
            auto id = GetRandomValue(sidAlienStart, sidAlienEnd-1);
            for (int col = 0; col < 15; ++col) {
                if((col + row) & 1) {
                    auto sprite = SpriteManager::instance()->getSprite(id, {offset + (float)col * 20.0f + 8, (float)row * 16.0f + 32});
                    _aliens.push_back(sprite);
                }
                //_aliens.push_back({Sprite::Alien, {static_cast<float>(col * 20) + 8, static_cast<float>(row * 16 + 32)}, {0.0f, 0.0f}, {static_cast<float>(int((5 - row) / 2) * 32) + 16, 0, 16, 8}, {static_cast<float>(int((5 - row) / 2) * 32), 0, 16, 8},
                //GREEN});
            }
        }
        _playerSprite = SpriteManager::instance()->getSprite(sidPlayer);
        _playerSprite._pos = {width() / 2.0f + 8, height() - 48.0f};
        _colonyShip = SpriteManager::instance()->getSprite(sidColonyShip, {width()/2.0f + width()/3.0f - 20, height() - 48.0f - 32});
        _speed = 1.0f + float(_level - 1)/5;
        _currentPlanet = getRandomPlanetName();
        generateTerrain(_currentPlanet);
        _colonists = _random.randomInt(40,250);
        _nextAlienBombTime = _random.randomInt(10, 30);
        _pixelOverGround = 1000;
        setState(State::Init);
    }

    void update(float dt_s) override
    {
        auto timeFlow = 1.0f;
        if(_state == State::Init ) {
            if(_stateTime >= InitStateFreezeTime) {
                _pixelOverGround = EaseSineIn(_stateTime - InitStateFreezeTime, 1000, -1000, InitStateTime - InitStateFreezeTime);
                _playerSprite._pos.y = EaseSineIn(_stateTime - InitStateFreezeTime, height() - 48, 16, InitStateTime - InitStateFreezeTime);
                timeFlow = std::min((_stateTime-InitStateFreezeTime)/4.0f, 1.0f);
            }
            else {
                timeFlow = 0.0f;
            }
        }
        updateBackground(dt_s, timeFlow);
        _weaponCharge += dt_s;
        _aliens.erase(std::remove_if(_aliens.begin(), _aliens.end(), [](const Sprite& s) { return s._type == Sprite::Unused; }), _aliens.end());
        if(_aliens.empty() && _state == State::Playing && !_numAlienBombs) {
            _score += _colonists * 50;
            _colonistsSaved += _colonists;
            if(_colonists > 0 && _colonyShip._velocity.y >= 0.0f) {
                _colonyShip._pos.y = height() + 50;
                _colonyShip._velocity.y = -75.0f;
            }
            setState(State::LevelComplete);
        }
        auto spd = 57.0f + (90.0f - static_cast<float>(_aliens.size())) + (float)_level * 3;
        _speed = _speed < 0 ? -spd : spd;
        float stepDown = 0.0f;
        if(_state != State::Init) {
            for (auto& alien : _aliens) {
                if (alien._pos.x <= 4) {
                    _speed = std::abs(_speed);
                    if (_state != State::Init) {
                        stepDown = 4;
                    }
                    break;
                }
                else if (alien._pos.x >= width() - 20) {
                    _speed = -std::abs(_speed);
                    if (_state != State::Init) {
                        stepDown = 4;
                    }
                    break;
                }
            }
            int bombAlien = -1;
            if(_state == State::Playing && (float)_nextAlienBombTime < _stateTime && !_aliens.empty()) {
                bombAlien = GetRandomValue(0, (int)_aliens.size()-1);
            }
            int countAliens = 0;
            for (auto& alien : _aliens) {
                alien._pos.x += _speed * dt_s;  // 0.01667f;
                alien._pos.y += stepDown;
                if (_state == State::Playing) {
                    if (alien._pos.y >= height()) {
                        setState(State::GameOver);
                    }
                    if(bombAlien >= 0 && bombAlien >= countAliens && alien._type != Sprite::Unused && alien._pos.y < height() - 100) {
                        _projectiles.push_back(SpriteManager::instance()->getSprite(sidAlienBomb, {alien._pos.x + 2, alien._pos.y + alien._sprite.height}, {0.0f, 0.8f}));
                        _nextAlienBombTime = int(_stateTime + (float)_random.randomInt(10,30));
                        bombAlien = -1;
                    }
                    else if (GetRandomValue(0, 1500) < 1) {
                        _projectiles.push_back(SpriteManager::instance()->getSprite(sidAlienLaser, {alien._pos.x + 6, alien._pos.y + alien._sprite.height}, {0.0f, 1.0f}, {100, 200, 100, 255}));
                    }
                    if (alien._type == Sprite::Alien && _playerSprite._type == Sprite::Player && alien.isColliding(_playerSprite)) {
                        explosion({_playerSprite._pos.x + 7, _playerSprite._pos.y + 6}, 50);
                        SoundManager::instance()->playSound(SoundId::Explosion);
                        _playerSprite._type = Sprite::Unused;
                        alien._type = Sprite::Unused;
                        _score += 10;
                        if (--_playerLives) {
                            setState(State::Killed);
                        }
                        else {
                            setState(State::GameOver);
                        }
                    }
                    if (alien._type == Sprite::Alien && _colonyShip._type == Sprite::ColonyShip && alien.isColliding(_colonyShip)) {
                        explosion({_colonyShip._pos.x + 7, _colonyShip._pos.y + 6}, 100);
                        explosion({_colonyShip._pos.x + 7, _colonyShip._pos.y + 24}, 100);
                        explosion({_colonyShip._pos.x + 7, _colonyShip._pos.y + 40}, 100);
                        SoundManager::instance()->playSound(SoundId::Explosion);
                        _colonyShip._type = Sprite::Unused;
                        alien._type = Sprite::Unused;
                        message("Oh noooo...", _colonyShip._pos.x - 20, _colonyShip._pos.y, 2000);
                        _colonists = 0;
                    }
                }
                ++countAliens;
            }
        }
        _numPlayerShots = 0;
        _numAlienBombs = 0;
        for (auto& projectile : _projectiles) {
            projectile._pos = Vector2Add(projectile._pos, projectile._velocity);
            if (projectile._pos.x > width() || projectile._pos.x < -7 || projectile._pos.y < -3 || projectile._pos.y > height()) {
                if(projectile._type == Sprite::AlienBomb && projectile._pos.y >= height() && std::abs(_colonyShip._velocity.y) < 0.001f) {
                    auto oldNum = _colonists;
                    _colonists = std::max(0, _colonists - GetRandomValue(20,50));
                    SoundManager::instance()->playSound(SoundId::Explosion);
                    message(TextFormat("Bomb killed %d colonists!", oldNum - _colonists), projectile._pos.x + 4, height() - 15, 2000);
                    if(_colonists > 0 && _colonists < 25 && _colonyShip._velocity.y == 0.0f) {
                        _colonyShip._pos.y = height() + 50;
                        _colonyShip._velocity.y = -75.0f;
                        message(TextFormat("Survivors started!", oldNum - _colonists), width() - 75, height() - 15, 2000);
                    }
                }
                projectile._type = Sprite::Unused;
            }
            else if (projectile._type == Sprite::PlayerShot) {
                _particles.emplace_back(Particle2::eFADE, Vector2{projectile._pos.x + 3, projectile._pos.y + 6}, Vector2{0.0f,0.0f}, BLACK, 500);
                _particles.back().gradient = &_playerLaserTrailColors;
                _particles.emplace_back(Particle2::eFADE, Vector2{projectile._pos.x + 4, projectile._pos.y + 7}, Vector2{0.0f,0.0f}, BLACK, 500);
                _particles.back().gradient = &_playerLaserTrailColors;
                for (auto& alien : _aliens) {
                    if (alien._type == Sprite::Alien && alien.isColliding(projectile)) {
                        alien._type = Sprite::Unused;
                        projectile._type = Sprite::Unused;
                        _score += 10;
                        explosion({alien._pos.x + 7, alien._pos.y + 3});
                        SoundManager::instance()->playSound(SoundId::Explosion);
                    }
                }
                for(auto& other : _projectiles) {
                    if(other._type == Sprite::AlienShot && other.isColliding(projectile)) {
                        projectile._type = Sprite::Unused;
                        other._type = Sprite::Unused;
                        _score += 100;
                        message("100", projectile._pos.x, projectile._pos.y, 1500);
                        explosion({projectile._pos.x + 3, projectile._pos.y + 1}, 50);
                        SoundManager::instance()->playSound(SoundId::Explosion);
                    }
                    else if(other._type == Sprite::AlienBomb && other.isColliding(projectile)) {
                        projectile._type = Sprite::Unused;
                        other._type = Sprite::Unused;
                        _score += 500;
                        message("500", projectile._pos.x, projectile._pos.y, 1500);
                        explosion({projectile._pos.x + 3, projectile._pos.y + 1}, 500);
                        SoundManager::instance()->playSound(SoundId::Explosion);
                    }
                }
            }
            else if (projectile._type == Sprite::AlienShot) {
                if(_frameCount & 1) {
                    _particles.emplace_back(Particle2::eFADE, Vector2{projectile._pos.x + 3, projectile._pos.y + 1}, Vector2{0.0f, 0.0f}, BLACK, 300);
                    _particles.back().gradient = &_alienLaserTrailColors;
                }
                else {
                    _particles.emplace_back(Particle2::eFADE, Vector2{projectile._pos.x + 4, projectile._pos.y }, Vector2{0.0f, 0.0f}, BLACK, 300);
                    _particles.back().gradient = &_alienLaserTrailColors;
                }
                if (_playerSprite.isColliding(projectile) && _playerSprite._type == Sprite::Player) {
                    explosion({_playerSprite._pos.x + 7, _playerSprite._pos.y + 6}, 500);
                    SoundManager::instance()->playSound(SoundId::Explosion);
                    _playerSprite._type = Sprite::Unused;
                    projectile._type = Sprite::Unused;
                    if(--_playerLives) {
                        setState(State::Killed);
                    }
                    else {
                        setState(State::GameOver);
                    }
                }
                if (_colonyShip.isColliding(projectile) && _colonyShip._type == Sprite::ColonyShip) {
                    explosion({_colonyShip._pos.x + 7, _colonyShip._pos.y + 6}, 100);
                    explosion({_colonyShip._pos.x + 7, _colonyShip._pos.y + 24}, 100);
                    explosion({_colonyShip._pos.x + 7, _colonyShip._pos.y + 40}, 100);
                    SoundManager::instance()->playSound(SoundId::Explosion);
                    _colonyShip._type = Sprite::Unused;
                    projectile._type = Sprite::Unused;
                    message("Oh noooo...", _colonyShip._pos.x - 20, _colonyShip._pos.y, 2000);
                    _colonists = 0;
                }
            }
            else if(projectile._type == Sprite::AlienBomb) {
                if(!(_frameCount & 3)) {
                    _particles.emplace_back(Particle2::eFADE, Vector2{projectile._pos.x + 4, projectile._pos.y + 3}, Vector2{(float)GetRandomValue(-30, 30), 0.1f}, BLACK, 1500);
                    _particles.back().gradient = &_alienLaserTrailColors;
                }
                if (_playerSprite.isColliding(projectile) && _playerSprite._type == Sprite::Player) {
                    explosion({_playerSprite._pos.x + 7, _playerSprite._pos.y + 6}, 500);
                    SoundManager::instance()->playSound(SoundId::Explosion);
                    _playerSprite._type = Sprite::Unused;
                    projectile._type = Sprite::Unused;
                    if(--_playerLives) {
                        setState(State::Killed);
                    }
                    else {
                        setState(State::GameOver);
                    }
                }
                else if (_colonyShip.isColliding(projectile) && _colonyShip._type == Sprite::ColonyShip) {
                    explosion({_colonyShip._pos.x + 7, _colonyShip._pos.y + 6}, 100);
                    explosion({_colonyShip._pos.x + 7, _colonyShip._pos.y + 24}, 100);
                    explosion({_colonyShip._pos.x + 7, _colonyShip._pos.y + 40}, 100);
                    SoundManager::instance()->playSound(SoundId::Explosion);
                    _colonyShip._type = Sprite::Unused;
                    projectile._type = Sprite::Unused;
                    message("Oh noooo...", _colonyShip._pos.x - 20, _colonyShip._pos.y, 2000);
                    _colonists = 0;
                }
            }
            if(projectile._type == Sprite::AlienBomb) {
                ++_numAlienBombs;
            }
            else if(projectile._type == Sprite::PlayerShot) {
                ++_numPlayerShots;
            }
        }
        for (auto& particle : _particles) {
            particle.update(dt_s);
        }
        _projectiles.erase(std::remove_if(_projectiles.begin(), _projectiles.end(), [](const Sprite& s) { return s._type == Sprite::Unused; }), _projectiles.end());
        _particles.erase(std::remove_if(_particles.begin(), _particles.end(), [](const Particle2& p) { return p.isActive(); }), _particles.end());
        _messages.erase(std::remove_if(_messages.begin(), _messages.end(), [](const Message& m) { return m.lifetime < _frameCount; }), _messages.end());

        if(_colonyShip._type == Sprite::ColonyShip) {
            _colonyShip.update(dt_s);
        }
        if(_playerSprite._type == Sprite::Player) {
            if (_playerSprite._pos.x > 2  && _pixelOverGround < 900 && (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A))) {
                _playerSprite._pos.x -= 2;
            }
            else if (_playerSprite._pos.x < width() - 18 && _pixelOverGround < 900 && (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D))) {
                _playerSprite._pos.x += 2;
            }
            if (_state == State::Playing && IsKeyPressed(KEY_SPACE) && (_weaponCharge > 0.5f || _numPlayerShots < 2)) {
                _weaponCharge = 0;
                _projectiles.push_back(SpriteManager::instance()->getSprite(sidPlayerLaser, {_playerSprite._pos.x + 4, _playerSprite._pos.y - 6}, {0.0f, -1.5f}, {0, 200, 255, 255}));
                SoundManager::instance()->playSound(SoundId::Laser);
            }
            _particles.emplace_back(Particle2::eFADE, Vector2{_playerSprite._pos.x + (float)GetRandomValue(6, 9), _playerSprite._pos.y + 15}, Vector2{0, 60.0f}, BLACK, 300);
            _particles.back().gradient = &_playerEngineTrailColors;
        }
        if(_state == State::GameOver && (_stateTime > GameOverStateTime || (_stateTime > 2 && IsKeyPressed(KEY_SPACE))) && _finishScreen != SceneId::SetupScene) {
            ScoreInfo scoreInfo = { _score, "YOU", _level, _colonistsSaved, _currentPlanet };
            setProperty("last_score"_h, _score);
            setProperty("last_level"_h, _level);
            setProperty("last_colonists"_h, _colonistsSaved);
            setProperty("last_planet"_h, _currentPlanet);
            _highscores.emplace(_score, scoreInfo);
            _finishScreen = SceneId::SetupScene;
        }
        if(_state == State::Killed && _stateTime > KilledStateTime) {
            _playerSprite._type = Sprite::Player;
            setState(State::Playing);
        }
        if(_state == State::LevelComplete) {
            //_alpha = std::clamp(1.0f - _stateTime / LevelCompleteStateTime, 0.0f, 1.0f);
            _alpha = _stateTime < LevelCompleteStateTime - 1.0f ? 1.0 : std::clamp(EaseSineIn(_stateTime - (LevelCompleteStateTime - 1.0f), 1.0f, -1.0f, 1.0f), 0.0f, 1.0f);
            //TraceLog(LOG_INFO, "LevelComplete: %0.3f / stateTime: %0.3f", _alpha, _stateTime);
            if(_stateTime > LevelCompleteStateTime) {
                setupLevel();
            }
        }
        if(_state == State::Init) {
            _alpha = _stateTime < 1.0f ? std::clamp(EaseSineIn(_stateTime, 0.0f, 1.0f, 1.0f), 0.0f, 1.0f) : 1.0f;
            if (_stateTime > InitStateTime) {
                setState(State::Playing);
            }
        }

        /*if (IsKeyPressed(KEY_E)) {
            SoundManager::instance()->generateAndPlaySound(NoiseType::EXPLOSION);
        }*/
    }

    void render() override
    {
        renderBackground();
        for (auto& particle : _particles) {
            if(!(particle.flags & Particle2::eEXPLODE)) {
                particle.draw();
            }
        }
        for (auto& projectile : _projectiles) {
            projectile.draw();
        }
        for (auto& alien : _aliens) {
            if (alien._type == Sprite::Unused) {
                DrawCircle(alien._pos.x + 6, alien._pos.y + 4, (float)GetRandomValue(5, 12), WHITE);
            }
            else {
                if(_pixelOverGround > 0) {
                    alien.draw({0, (float)-_pixelOverGround}, (static_cast<int>(alien._pos.x) & 16));
                }
                else {
                    alien.draw((static_cast<int>(alien._pos.x) & 16));
                }
            }
        }
        if(_playerSprite._type == Sprite::Player) {
            _playerSprite.draw();
        }
        if(_colonyShip._type == Sprite::ColonyShip) {
            if(std::fabs(_colonyShip._velocity.y) <= 0.001) {
                auto surface = 1000 - (float)_pixelOverGround + height() - 32;
                _colonyShip._pos.y = surface - 48;
            }
            _colonyShip.draw();
        }
        for (auto& particle : _particles) {
            if(particle.flags & Particle2::eEXPLODE) {
                particle.draw();
            }
        }

        renderTerrain();

        for(auto& m : _messages) {
            DrawText(m.text.c_str(), m.x, m.y, 10, {255,255,255,128});
        }

        if(_alpha < 1.0f) {
            DrawRectangle(0,0,width(), height(), {0,0,0,(uint8_t)(255 * (1.0f-_alpha))});
        }
        DrawText(TextFormat("SCORE: %d", _score), 16, 5, 10, RED);
        DrawText(TextFormat("LEVEL: %d", _level), width()/2 - 75, 5, 10, RED);
        DrawText(TextFormat("COLONISTS: %d", _colonists), width()/2 + 40, 5, 10, RED);
        DrawText(TextFormat("LIVES: %d", _playerLives), width() - 50, 5, 10, RED);
#ifndef NDEBUG
        //drawTextCentered(TextFormat("State:%d Time:%ds CS-Y: %0.3f", (int)_state, (int)_stateTime, _colonyShip._pos.y), height() - 10, 10, BLUE);
#endif

        if(_state == State::GameOver) {
            showGameOver(120);
        }
        else if(_state == State::LevelComplete) {
            if(_colonists) {
                drawTextCentered("GOOD WORK!", 120, 20, WHITE);
                drawTextCentered(TextFormat("You saved %d colonists", _colonists), 145, 10, WHITE);
            }
        }
        else if(_state == State::Init) {
            drawTextCentered("Get ready to cover the retreat from", 120, 10, WHITE);
            drawTextCentered(_currentPlanet.c_str(), 145, 20, WHITE);
            drawTextCentered(TextFormat("%d colonists", _colonists), 170, 10, WHITE);
        }
        // DrawText(TextFormat("Particles: %ld", _particles.size()), width()/2, 0, 10, WHITE);

#if 0
        auto segment = catmullRomSpline({20,0,0}, {20,40,0}, {100, 270, 0}, {100, 300, 0});
        for(float t = 0.0f; t <= 1.0f; t += 0.01f) {
            Vector3 point = segment.a * t * t * t + segment.b * t * t + segment.c * t + segment.d;
            drawPixel({point.x, point.y}, WHITE);
        }
#endif
    }

    void unload() override
    {
        unloadBackground();
    }

    void setState(State state)
    {
        if(state != _state) {
            _state = state;
            _stateTime = 0;
        }
    }

    void showGameOver(float y)
    {
        auto textWidth = _vectorFont.textWidth("THAT WAS CLOSE!", 20);
        _vectorFont.drawString("THAT WAS CLOSE!", {(width() - textWidth) / 2, y}, 20.0f, WHITE);
        _vectorFont.drawString("THAT WAS CLOSE!", {(width() - textWidth) / 2 + 1, y}, 20.0f, WHITE);
        _vectorFont.drawString("THAT WAS CLOSE!", {(width() - textWidth) / 2, y + 1}, 20.0f, WHITE);
        _vectorFont.drawString("THAT WAS CLOSE!", {(width() - textWidth) / 2 + 1, y + 1}, 20.0f, WHITE);
    }


    SceneId thisScene() const override
    {
        return SceneId::IngameScene;
    }

    SceneId nextScene() override
    {
        return _finishScreen;
    }

    std::string getRandomPlanetName()
    {
        static std::vector<std::string> starNames = {"Aldebaran", "Algol", "Altair", "Betelgeuse", "Bunda",   "Capella", "Castor", "Deneb", "Fafnir",  "Furud",  "Gomeisa", "Haedus",  "Intercrus", "Kochab",   "Larawag", "Maia", "Megrez", "Menkar",
                                                     "Mira",      "Nihal", "Nunki",  "Pherkad",    "Pleione", "Procyon", "Rigel",  "Sabik", "Seginus", "Sirius", "Subra",   "Tegmine", "Timir",     "Torcular", "Ukdah",   "Vega", "Zaurak"};
        static std::vector<std::string> numbers = {"II", "III", "IV", "V", "VI", "VII", "VIII", "IX", "X", "XI", "XII"};
        std::string result;
        do {
            auto star = _levelSequenceRandom.randomInt(0, (int)starNames.size() - 1);
            auto planet = _levelSequenceRandom.randomInt(0, (int)numbers.size() - 1);
            result = starNames[star] + " " + numbers[planet];
        } while(_savedPlanets.count(result));
        return result;
    }

    void renderTerrain()
    {
        static const int buildings[] = { 10, 14, 8,12 };
        int left = (int)width()/2;
        int right = left + (int)width()/3;
        auto dome = left + 75;
        auto surface = 1000 - _pixelOverGround + (int)height() - 32;
        if(_pixelOverGround > 900) {
            // Geo-Dome
            DrawCircle(dome, surface, 24, BasePalette[3]);
            DrawCircle(dome, surface, 22, BasePalette[4]);
            for (int i = 0; i < 4; ++i) {
                DrawRectangle(dome - 13 + i * 7, surface - buildings[i], 6, buildings[i], BasePalette[2]);
            }
            DrawCircle(dome, surface, 22, Fade(BasePalette[4], 0.5f));

            // Ground
            for (int i = 0; i < left; ++i) {
                DrawRectangle(i, surface - _terrainHeight[1024 - left + i], 1, 100, BasePalette[17]);
            }
            for (int i = right; i < (int)width(); ++i) {
                DrawRectangle(i, surface - _terrainHeight[i - right], 1, 100, BasePalette[17]);
            }
            DrawRectangle(left, surface, right - left, 48, BasePalette[17]);
            auto tw = MeasureText(_currentPlanet.c_str(), 10);
            DrawText(_currentPlanet.c_str(), dome - tw / 2, surface + 7, 10, WHITE);
        }
    }
    void generateTerrain(const std::string& name)
    {
        _random.seed(name);
        _terrainHeight.assign(1024,0);
        generateTerrain(0, 1023, 48);
    }
protected:
    void explosion(Vector2 pos, int particles = 250)
    {
        for (int i = 0; i < particles; ++i) {
            _particles.emplace_back(Particle2::eFADE, pos, _random.randomSphereVec2(1.0f) * _random.randomExpFloat(0.05f), BLACK, _random.randomInt(500, 1200));
            _particles.back().gradient = &_explosionColors;
        }
    }
    void generateTerrain(size_t left, size_t right, float disp)
    {
        if(left + 1 >= right) return;
        auto between = std::floor((left + right) / 2);
        auto delta = (_random.randomUFloat() * 2 - 1) * disp;
        _terrainHeight[between] = (_terrainHeight[left] + _terrainHeight[right]) / 2 + (int)delta;
        disp = disp * 0.7f;
        generateTerrain(left, between, disp);
        generateTerrain(between, right, disp);
    }
    void message(std::string msg, int x, int y, int time_ms)
    {
        auto w = MeasureText(msg.c_str(), 10);
        _messages.push_back({x - w/2, y, std::move(msg), _frameCount + int((float)time_ms/16.6667f), {255,255,255,128}});
    }
    State _state{State::Init};
    SceneId _finishScreen = SceneId::IngameScene;
    Random _random{1234};
    Random _levelSequenceRandom{3456};
    Sprite _playerSprite;
    Sprite _colonyShip;
    using Alien = Sprite;
    using Projectile = Sprite;
    std::vector<Alien> _aliens;
    std::vector<Projectile> _projectiles;
    std::vector<Particle2> _particles;
    std::vector<int> _terrainHeight{1024};
    std::vector<int> _buildings{4};
    struct Message {
        int x{0};
        int y{0};
        std::string text;
        int64_t lifetime{0};
        Color color;
    };
    std::vector<Message> _messages;
    float _speed{1.0f};
    float _weaponCharge{0.0f};
    int _playerLives{3};
    int _level{0};
    int _pixelOverGround{0};
    size_t _score{0};
    std::map<std::string, int> _savedPlanets;
    std::string _currentPlanet;
    int _colonists{0};
    int _colonistsSaved{0};
    int _nextAlienBombTime{0};
    int _numAlienBombs{0};
    int _numPlayerShots{0};
};
