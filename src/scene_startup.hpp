#pragma once

#include "scene_base.hpp"

class StartupScene : public Scene
{
public:
    StartupScene() = default;
    ~StartupScene() override = default;

    void load() override
    {
        _finishScreen = SceneId::StartupScene;
        _framesCounter = 0;
        _lettersCount = 0;

        _logoPositionX = width() / 2 - 128;
        _logoPositionY = height() / 2 - 128;

        _topSideRecWidth = 16;
        _leftSideRecHeight = 16;
        _bottomSideRecWidth = 16;
        _rightSideRecHeight = 16;

        _state = 0;
        _alpha = 1.0f;
    }

    void update(float) override
    {
        switch (_state) {
            case 0:  // State 0: Top-left square corner blink logic
                ++_framesCounter;
                if (_framesCounter == 80) {
                    _state = 1;
                    _framesCounter = 0;  // Reset counter... will be used later...
                }
                break;

            case 1:  // State 1: Bars animation logic: top and left
                _topSideRecWidth += 8;
                _leftSideRecHeight += 8;
                if (_topSideRecWidth == 256) {
                    _state = 2;
                }
                break;

            case 2:  // State 2: Bars animation logic: bottom and right
                _bottomSideRecWidth += 8;
                _rightSideRecHeight += 8;
                if (_bottomSideRecWidth == 256) {
                    _state = 3;
                }
                break;

            case 3:  // State 3: "raylib" text-write animation logic
                ++_framesCounter;
                if (_lettersCount < 10) {
                    if (_framesCounter / 12)  // Every 12 frames, one more letter!
                    {
                        ++_lettersCount;
                        _framesCounter = 0;
                    }
                }
                if (_framesCounter > 200) {
                    _alpha -= 0.02f;

                    if (_alpha <= 0.0f) {
                        _alpha = 0.0f;
                        _finishScreen = SceneId::SetupScene;  // Jump to next screen
                    }
                }
                break;
        }
        if (IsMouseButtonPressed(0) || (_clicked && IsKeyPressed(KEY_SPACE))) {
            _clicked = true;
            _finishScreen = SceneId::SetupScene;
        }
    }

    void render() override
    {
        switch(_state) {
            case 0:  // Draw blinking top-left square corner
                if ((_framesCounter / 10) % 2)
                    DrawRectangle(_logoPositionX, _logoPositionY, 16, 16, BLACK);
                break;
            case 1:  // Draw bars animation: top and left
                DrawRectangle(_logoPositionX, _logoPositionY, _topSideRecWidth, 16, BLACK);
                DrawRectangle(_logoPositionX, _logoPositionY, 16, _leftSideRecHeight, BLACK);
                break;
            case 2:  // Draw bars animation: bottom and right
                DrawRectangle(_logoPositionX, _logoPositionY, _topSideRecWidth, 16, BLACK);
                DrawRectangle(_logoPositionX, _logoPositionY, 16, _leftSideRecHeight, BLACK);
                DrawRectangle(_logoPositionX + 240, _logoPositionY, 16, _rightSideRecHeight, BLACK);
                DrawRectangle(_logoPositionX, _logoPositionY + 240, _bottomSideRecWidth, 16, BLACK);
                break;
            case 3:  // Draw "raylib" text-write animation + "powered by"
                DrawRectangle(_logoPositionX, _logoPositionY, _topSideRecWidth, 16, Fade(BLACK, _alpha));
                DrawRectangle(_logoPositionX, _logoPositionY + 16, 16, _leftSideRecHeight - 32, Fade(BLACK, _alpha));
                DrawRectangle(_logoPositionX + 240, _logoPositionY + 16, 16, _rightSideRecHeight - 32, Fade(BLACK, _alpha));
                DrawRectangle(_logoPositionX, _logoPositionY + 240, _bottomSideRecWidth, 16, Fade(BLACK, _alpha));
                DrawRectangle(width() / 2 - 112, height() / 2 - 112, 224, 224, Fade(RAYWHITE, _alpha));
                DrawText(TextSubtext("raylib", 0, _lettersCount), width() / 2 - 44, height() / 2 + 48, 50, Fade(BLACK, _alpha));
                if (_framesCounter > 20)
                    DrawText("powered by", _logoPositionX, _logoPositionY - 27, 20, Fade(DARKGRAY, _alpha));
                break;
            default:
                break;
        }
    }

    void unload() override
    {
    }

    SceneId thisScene() const override
    {
        return SceneId::StartupScene;
    }

    SceneId nextScene() override
    {
        return _finishScreen;
    }

private:
    int _framesCounter = 0;
    SceneId _finishScreen = SceneId::StartupScene;

    int _logoPositionX = 0;
    int _logoPositionY = 0;

    int _lettersCount = 0;

    int _topSideRecWidth = 0;
    int _leftSideRecHeight = 0;

    int _bottomSideRecWidth = 0;
    int _rightSideRecHeight = 0;

    int _state = 0;       // Logo animation states
    float _alpha = 1.0f;  // Useful for fading
};
