
#include <raylib.h>
#include <array>
#include <cstdint>
#include <iostream>
#include <lostcolonies/version.hpp>
#include <memory>
#include "cli.hpp"
#include "resources.h"
#include "scene_generators.hpp"
#include "scene_ingame.hpp"
#include "scene_setup.hpp"
#include "scene_startup.hpp"
#include "sound.hpp"
#include "spritemanager.hpp"

#include <rlgl.h>

#ifdef WITH_FFMPEG
#include "ffmpeg_encode.hpp"
#endif

#ifndef NDEBUG
#include "font8x8.h"
#include "scene_generators.hpp"
#endif

#ifdef PLATFORM_DESKTOP
#if defined(PLATFORM_DESKTOP)
#ifdef __APPLE__
#define GLFW_INCLUDE_NONE  // Disable the standard OpenGL header inclusion on GLFW3
                           // NOTE: Already provided by rlgl implementation (on glad.h)
#include "GLFW/glfw3.h"
#endif
#endif
#elif defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#endif

class InvadeApp
{
public:
    InvadeApp(int argc, char* argv[])
    {
        CLI cli{argc, argv};
#ifndef NDEBUG
        int64_t logLevel{LOG_INFO};
#else
        int64_t logLevel{LOG_NONE};
#endif
        cli.option({"--log"}, logLevel, "Set log level (0=all, 7=none)");

#ifndef NDEBUG
        bool generator = false;
        std::string alienSheetFile;
        cli.option({"-g", "--generator"}, generator, "run with generator ui instead of game");
        cli.option({"--generate-alien-sheet"}, alienSheetFile, "generate a bunch of random sprites and export");
#endif
#ifdef WITH_FFMPEG
        std::string movieFile;
        cli.option({"-r", "--record"}, movieFile, "record video under given name");
#endif
        // 👾 LOST COLONIES 🌘
        // Score: 5010, Level: 4
        cli.parse();

        SetTraceLogLevel(logLevel);

#ifndef NDEBUG
        if (!alienSheetFile.empty()) {
            auto params = Procedural::randomAlienSpaceshipParams(4, 0);
            auto sheet = GenImageColor(2048, 2048, Scene::BasePalette[0]);
            auto tileWidth = std::max(5 * 8, params.generatedWidth * (params.mirrorHorizontal ? 2 : 1)) + 4;
            auto tileHeight = params.generatedHeight * (params.mirrorVertical ? 2 : 1) + 16;
            Random::instance()->seed(4711);
            for (int y = 0; y < 2048 / tileHeight; ++y) {
                for (int x = 0; x < 2048 / tileWidth; ++x) {
                    auto xpos = x * tileWidth;
                    auto ypos = y * tileHeight;
                    auto shipParams = Procedural::randomAlienSpaceshipParams(4, 0);
                    // shipParams.randSeed = rg.randomInt(1,0xffff);
                    auto img = Procedural::generateSpaceship(shipParams);
                    imageDrawText8x8(sheet, TextFormat("%04x", shipParams.randSeed), xpos + (tileWidth - 4 * 8) / 2,
                                     ypos + 2, WHITE);
                    ImageDraw(&sheet, img, {0, 0, img.width, img.height},
                              {xpos + (tileWidth - img.width) / 2, ypos + 12, img.width, img.height}, WHITE);
                    UnloadImage(img);
                }
            }
            ExportImage(sheet, alienSheetFile.c_str());
            UnloadImage(sheet);
            exit(0);
        }
#endif
        InitWindow(screenWidth, screenHeight, "Lost Colonies v" LOSTCOLONIES_VERSION_STRING_LONG);
        // ToggleFullscreen();
        InitAudioDevice();

        auto* sm = SoundManager::instance();
        sm->generateSounds(
            SoundId::Explosion,
            {{0.3f, {36415,     3,         0.000000f,  0.286630f, 0.277880f, 0.458200f, 0.071974f, 0.000000f,
                     0.000160f, 0.000000f, 0.000000f,  0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f,
                     0.000000f, 0.441780f, -0.230100f, 1.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f}},
             {0.3f, {3854,       3,         0.000000f,  0.363130f, 0.517100f, 0.355000f, 0.123707f, 0.000000f,
                     -0.057160f, 0.000000f, 0.000000f,  0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f,
                     0.000000f,  0.182670f, -0.138390f, 1.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f}},
             {0.4f, {58754,     3,         0.000000f, 0.347710f, 0.590840f, 0.073750f, 0.012841f, 0.000000f,
                     0.120600f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f,
                     0.000000f, 0.000000f, 0.000000f, 1.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f}}});
        sm->generateSounds(
            SoundId::Laser,
            {{1.0f, {9301,      0,         0.000000f,  0.187960f, 0.019440f, 0.196960f, 0.224141f, 0.347600f,
                     0.294766f, 0.000000f, 0.000000f,  0.000000f, 0.000000f, 0.000000f, 0.891650f, -0.585410f,
                     0.000000f, 0.000260f, -0.184540f, 1.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f}}});
        // auto mecha = ResourceManager::instance().resourceForName("mecha.png");
        // auto image = LoadImageFromMemory(".png", mecha.data(), int(mecha.size()));
        //_font = LoadFontFromImage(image, {255,0,255,255}, ' ');

        _renderTexture = LoadRenderTexture(GetScreenWidth(), GetScreenHeight());
#ifndef NDEBUG
        if (generator) {
            _currentScene = std::make_unique<GeneratorsScene>();
        }
        else {
            _currentScene = std::make_unique<StartupScene>();
        }
#else
        _currentScene = std::make_unique<StartupScene>();
#endif
        _currentScene->load();
#ifdef WITH_FFMPEG
        if (!movieFile.empty()) {
            if (!_encoder.Open(movieFile.c_str(), {screenWidth, screenHeight, 60, 200000, nullptr, 22, AV_PIX_FMT_RGBA,
                                                   AV_PIX_FMT_YUV420P})) {
                TraceLog(LOG_ERROR, "Couldn't open video stream");
            }
        }
#endif
        // msf_gif_begin(&_gifState, 450, 300);

        // DisableCursor();
        // SetMouseScale(0.5,0.5);
    }

    ~InvadeApp()
    {
        /*
        MsfGifResult result = msf_gif_end(&_gifState);
        if (result.data) {
            FILE * fp = fopen("MyGif.gif", "wb");
            fwrite(result.data, result.dataSize, 1, fp);
            fclose(fp);
        }
        msf_gif_free(result);
         */
        UnloadRenderTexture(_renderTexture);
        SpriteManager::instance()->unloadSprites();
        SoundManager::instance()->unloadSounds();
        CloseAudioDevice();
        CloseWindow();
    }

    int run()
    {
#if defined(PLATFORM_WEB)
        emscripten_set_main_loop_arg(InvadeApp::updateAndDrawFrame, this, 60, 1);
#else
        SetTargetFPS(60);
        while (!WindowShouldClose()) {
            updateFrame();
        }
#endif
        return 0;
    }

    static void updateAndDrawFrame(void* self)
    {
        static_cast<InvadeApp*>(self)->updateFrame();
    }

    void handleActivity()
    {
        // auto dt = GetFrameTime();
    }

    void updateFrame()
    {
        if (IsKeyPressed(KEY_P)) {
            _pause = !_pause;
        }
        if (IsKeyPressed(KEY_M)) {
            _mute = !_mute;
            if (_mute) {
                SetMasterVolume(0.0f);
            }
            else {
                SetMasterVolume(1.0f);
            }
        }
        if (IsKeyPressed(KEY_T)) {
            takeScreenshot("testscreenshot.png");
        }
        if (!_pause) {
            auto t = std::min(GetFrameTime(), 2.0f / 60);
            _currentScene->update(t);
        }
        if (_inTransition) {
            updateTransition();
        }
        else {
            if (_currentScene->nextScene() != _currentScene->thisScene()) {
                transitionToScreen(_currentScene->nextScene());
            }
        }

        switch (_currentScene->renderMode()) {
            case RenderMode::Unscaled:
            case RenderMode::Upscale2: {
                BeginTextureMode(_renderTexture);
                ClearBackground(RAYWHITE);
                _currentScene->render();
                if (_mute) {
                    DrawText("[MUTE]", 8, _currentScene->height() - 10, 10, WHITE);
                }
                auto fps = GetFPS();
                if (fps < 55 || fps > 64) {
                    DrawText(TextFormat("%3d", GetFPS()), _currentScene->width() - 20, _currentScene->height() - 10, 10,
                             GREEN);
                }
                EndTextureMode();
                BeginDrawing();
                DrawTexturePro(_renderTexture.texture,
                               {0, (float)GetScreenHeight() - _currentScene->height(), (float)_currentScene->width(),
                                (float)-_currentScene->height()},
                               {0, 0, (float)GetScreenWidth(), (float)GetScreenHeight()}, {0.0f, 0.0f}, 0.0f, WHITE);
                if (_inTransition) {
                    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(BLACK, _transAlpha));
                }
                EndDrawing();

#ifdef WITH_FFMPEG
                if (_encoder.IsOpen()) {
                    auto frame = grabScreenImage();
                    ImageResizeNN(&frame, screenWidth, screenHeight);
                    _encoder.Write((uint8_t*)frame.data);
                    UnloadImage(frame);
                }
#endif
                break;
            }
            default: {
                BeginDrawing();
                ClearBackground(RAYWHITE);
                _currentScene->render();
                if (_inTransition) {
                    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(BLACK, _transAlpha));
                }
                EndDrawing();
                break;
            }
        }
    }

    void transitionToScreen(SceneId scene)
    {
        _inTransition = true;
        _transitionFadeOut = false;
        switch (scene) {
            case SceneId::StartupScene:
                _nextScene = std::make_unique<StartupScene>();
                break;
            case SceneId::SetupScene:
                _nextScene = std::make_unique<SetupScene>();
                break;
            case SceneId::IngameScene:
                _nextScene = std::make_unique<IngameScene>();
                break;
#ifndef NDEBUG
            case SceneId::GeneratorsScene:
                _nextScene = std::make_unique<GeneratorsScene>();
                break;
#endif
            default:
                break;
        }
        _transAlpha = 0.0f;
    }

    void updateTransition()
    {
        if (!_transitionFadeOut) {
            _transAlpha += 0.05f;

            // NOTE: Due to float internal representation, condition jumps on 1.0f instead of 1.05f
            // For that reason we compare against 1.01f, to avoid last frame loading stop
            if (_transAlpha > 1.01f) {
                _transAlpha = 1.0f;

                // Unload current screen
                _currentScene->unload();
                _currentScene = std::move(_nextScene);
                _currentScene->load();

                // Activate fade out effect to next loaded screen
                _transitionFadeOut = true;
            }
        }
        else  // Transition fade out logic
        {
            _transAlpha -= 0.02f;

            if (_transAlpha < -0.01f) {
                _transAlpha = 0.0f;
                _transitionFadeOut = false;
                _inTransition = false;
            }
        }
    }

    void takeScreenshot(const char* fileName)
    {
#ifndef PLATFORM_WEB
        auto image = grabScreenImage();
        ExportImage(image, fileName);
        UnloadImage(image);
        TRACELOG(LOG_INFO, "SYSTEM: [%s] Screenshot taken successfully", fileName);
#endif
    }

    Image grabScreenImage()
    {
#if defined(PLATFORM_DESKTOP) && defined(__APPLE__)
        int width, height;
        glfwGetFramebufferSize(glfwGetCurrentContext(), &width, &height);
        unsigned char* imgData = rlReadScreenPixels(width, height);
        return Image{imgData, width, height, 1, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8};
#else
        return LoadImageFromScreen();
#endif
    }

private:
    // Font _font{0};
    RenderTexture2D _renderTexture{};
    std::unique_ptr<Scene> _currentScene;
    std::unique_ptr<Scene> _nextScene;
    bool _inTransition{false};
    bool _transitionFadeOut{true};
    bool _pause{false};
    bool _mute{false};
    float _transAlpha{0.0f};
    // int64_t _frameCount{0};
#ifdef WITH_FFMPEG
    FfmpegEncoder _encoder;
#endif
};

int main(int argc, char* argv[])
{
    InvadeApp app{argc, argv};
    return app.run();
}
