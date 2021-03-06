#pragma once

#include <lostcolonies/version.hpp>
#include "scene_base.hpp"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>

extern "C" {

EM_JS(void, jsWriteClipboard, (const char* c_str), {
    var str = UTF8ToString(c_str);
    navigator.clipboard.writeText(str).then(() = > {console.log('Copied score info to clipboard.')}).catch((error) = > {console.log('Copy failed! ${error}')});
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
        while (_highscores.size() > 10) {
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
        if (getProperty("last_score"_h, 0) && IsKeyPressed(KEY_S)) {
#ifdef __EMSCRIPTEN__
            jsWriteClipboard((std::string("\xf0\x9f\x8c\x98 LOST COLONIES v" LOSTCOLONIES_VERSION_STRING_SHORT " \xf0\x9f\x91\xbe\nScore: ") + std::to_string(getProperty("last_score"_h, 0lu)) +
                              ", Level: " + std::to_string(getProperty("last_level"_h, 0)) +
                              ", Colonists: " + std::to_string(getProperty("last_colonists"_h, 0)))
                                 .c_str());
#else
            SetClipboardText((std::string("\xf0\x9f\x8c\x98 LOST COLONIES v" LOSTCOLONIES_VERSION_STRING_SHORT " \xf0\x9f\x91\xbe\nScore: ") + std::to_string(getProperty("last_score"_h, 0lu)) +
                              ", Level: " + std::to_string(getProperty("last_level"_h, 0)) +
                              ", Colonists: " + std::to_string(getProperty("last_colonists"_h, 0)))
                                 .c_str());
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
        using namespace std::string_literals;
        renderBackground();
        switch (_state) {
            case State::Title: {
                showTitle(100.0f);
                drawTextCentered(R"(A game originally for the "raylib 5K gamejam")", 150, 10, WHITE);
                drawTextCentered("Post-jam version v" LOSTCOLONIES_VERSION_STRING_SHORT, 165, 10, RAYWHITE);
                drawTextCentered(R"(by Steffen "Gulrak" Schuemann)", 180, 10, WHITE);
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
                int count = 0;
                auto lastScore =  getProperty("last_score"_h, 0lu);
                auto lastLevel = getProperty("last_level"_h, 0);
                auto lastPlanet = getProperty("last_planet"_h, ""s);
                for (const auto& [score, info] : _highscores) {
                    ++count;
                    if (info.score == lastScore && info.level == lastLevel && info.planet == lastPlanet) {
                        drawTextCentered(TextFormat("> %02ld.  %07d  %3s  L:%02d  C:%5d <", count, (int)score, info.name.c_str(), info.level, info.colonists), count * 20 + 50, 10, BasePalette[colorIndices[((int)(_stateTime * 10) + count) % 4]]);
                    }
                    else {
                        drawTextCentered(TextFormat("%02d.  %07d  %3s  L:%02d  C:%5d", count, (int)score, info.name.c_str(), info.level, info.colonists), count * 20 + 50, 10, BasePalette[colorIndices[((int)(_stateTime * 10) + count) % 4]]);
                    }
                }
                if (getProperty("last_score"_h, 0lu)) {
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
