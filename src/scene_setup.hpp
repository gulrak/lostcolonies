#pragma once

#include <lostcolonies/version.hpp>
#include "scene_base.hpp"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>

extern "C" {

EM_JS(void, jsWriteClipboard, (const char* c_str), {

    var str = UTF8ToString(c_str);
    navigator.clipboard.writeText(str)
        .then(() => { console.log('Copied score info to clipboard.') })
        .catch((error) => { console.log('Copy failed! ${error}') });
})

}
#endif

class SetupScene : public SceneBase
{
public:
    enum class State { Title, Story, Usage, Highscore };
    SetupScene() = default;
    ~SetupScene() override = default;

    void load() override
    {
        loadBase();
        setState(_played ? State::Highscore : State::Title);
        while(_highscores.size() > 10) {
            _highscores.erase(std::prev(_highscores.end()));
        }
    }

    void update(float dt_s) override
    {
        updateBackground(dt_s);

        switch (_state) {
            case State::Title: {
                if (_stateTime > 10) {
                    setState(State::Story);
                }
                break;
            }
            case State::Story: {
                if (_stateTime > 15) {
                    setState(State::Usage);
                }
                break;
            }
            case State::Usage: {
                if (_stateTime > 10) {
                    setState(State::Highscore);
                }
                break;
            }
            case State::Highscore: {
                if (_stateTime > 10) {
                    setState(State::Title);
                }
                break;
            }
        }
        if(_lastScoreInfo.score && IsKeyPressed(KEY_S)) {
#ifdef __EMSCRIPTEN__
            jsWriteClipboard(TextFormat("\xf0\x9f\x8c\x98 LOST COLONIES v" LOSTCOLONIES_VERSION_STRING_SHORT " \xf0\x9f\x91\xbe\nScore: %d, Level: %d, Colonists: %d", _lastScoreInfo.score, _lastScoreInfo.level, _lastScoreInfo.colonists));
#else
            SetClipboardText(TextFormat("\xf0\x9f\x8c\x98 LOST COLONIES v" LOSTCOLONIES_VERSION_STRING_SHORT "\xf0\x9f\x91\xbe\nScore: %d, Level: %d, Colonists: %d", _lastScoreInfo.score, _lastScoreInfo.level, _lastScoreInfo.colonists));
#endif
        }
        if (IsMouseButtonPressed(0) || (_clicked && IsKeyPressed(KEY_SPACE))) {
            _clicked = true;
            _played = true;
            _finishScreen = SceneId::IngameScene;
        }
    }

    void render() override
    {
        renderBackground();
        switch (_state) {
            case State::Title: {
                showTitle(100.0f);
                drawTextCentered(R"(A game for the "raylib 5K gamejam")", 150, 10, WHITE);
                drawTextCentered(R"(by Steffen "Gulrak" Schuemann)", 165, 10, WHITE);
                if (!_clicked) {
                    drawTextCentered(R"(<<CLICK TO START>>)", 220, 10, WHITE);
                }
                else {
                    drawTextCentered(R"(<<CLICK OR SPACE TO START>>)", 220, 10, WHITE);
                }
                break;
            }
            case State::Story: {
                showTitle(60.0f);
                drawTextBoxed(
                    "In the year 2374 humanity has reached for the start and colonized many worlds. They believed to be alone in the universe but that year proved them wrong!\n\n"
                    "It was the year humanity crossed path with the alien group they named Cicadas, as science revealed they where dormant for a few thousand years but reappeared and claimed "
                    "their area of space back.\n\n"
                    "Humanity fought hard, but failed, so they decided to evacuate those colonies and you are chosen to protect their retreat...",
                    {25, 90, 410, 200}, 10, 2, true, WHITE);
                break;
            }
            case State::Usage: {
                drawTextCentered("Attention!", 50, 20, WHITE);
                drawTextCentered("Control your ship with the LEFT/RIGHT cursor", 100, 10, WHITE);
                drawTextCentered("or the 'A' and 'D' keys.", 120, 10, WHITE);
                drawTextCentered("Use SPACE to fire at your enemies.", 140, 10, WHITE);
                drawTextCentered("Avoid getting hit by aliens or projectiles.", 160, 10, WHITE);
                drawTextCentered("Stop alien bombs to protect boarding colonists.", 180, 10, WHITE);
                drawTextCentered("Protect the launching colony ship!", 200, 10, WHITE);
                break;
            }
            case State::Highscore: {
                static int colorIndices[4] = {13, 14, 15, 14};
                drawTextCentered("HIGHSCORES", 40, 20, WHITE);
                size_t count = 0;
                for (const auto& [score, info] : _highscores) {
                    ++count;
                    if (info.score == _lastScoreInfo.score && info.level == _lastScoreInfo.level && info.planet == _lastScoreInfo.planet) {
                        drawTextCentered(TextFormat("> %02d.  %07d  %3s  L:%02d  C:%5d <", count, (int)score, info.name.c_str(), info.level, info.colonists), count * 20 + 50, 10, BasePalette[colorIndices[((int)(_stateTime * 10) + count) % 4]]);
                    }
                    else {
                        drawTextCentered(TextFormat("%02d.  %07d  %3s  L:%02d  C:%5d", count, (int)score, info.name.c_str(), info.level, info.colonists), count * 20 + 50, 10, BasePalette[colorIndices[((int)(_stateTime * 10) + count) % 4]]);
                    }
                }
                if (_lastScoreInfo.score) {
                    drawTextCentered("[press 'S' to share last result via clipboard]", 280, 10, WHITE);
                }
                break;
            }
        }
        // DrawText("Lost Colonies v" LOSTCOLONIES_VERSION_STRING_SHORT, 10, 10, 20, GREEN);
        // DrawTextureRec(_spriteTexture, {0, 0, 16, 8}, {100, 100}, {0,255,0,255});
    }

    void unload() override
    {
        unloadBackground();
    }

    void setState(State state)
    {
        _state = state;
        _stateTime = 0;
    }

    SceneId thisScene() const override
    {
        return SceneId::SetupScene;
    }

    SceneId nextScene() override
    {
        return _finishScreen;
    }

    void showTitle(float y)
    {
        auto titleWidth = _vectorFont.textWidth("LOST COLONIES", 30);
        _vectorFont.drawString("LOST COLONIES", {(width() - titleWidth) / 2, y}, 30.0f, WHITE);
        _vectorFont.drawString("LOST COLONIES", {(width() - titleWidth) / 2 + 1, y}, 30.0f, WHITE);
        _vectorFont.drawString("LOST COLONIES", {(width() - titleWidth) / 2, y + 1}, 30.0f, WHITE);
        _vectorFont.drawString("LOST COLONIES", {(width() - titleWidth) / 2 + 1, y + 1}, 30.0f, WHITE);
    }

private:
    SceneId _finishScreen = SceneId::SetupScene;
    State _state{State::Title};
    inline static bool _played{false};
};
