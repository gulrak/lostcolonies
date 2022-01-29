#pragma once

#include "scene_base.hpp"
#include "procedural.hpp"

class GeneratorsScene : public Scene
{
public:
    GeneratorsScene();
    ~GeneratorsScene() override;

    void load() override
    {
    }

    void update(float) override
    {
    }

    RenderMode renderMode() const override { return RenderMode::Unscaled; }

    void render() override;

    void unload() override
    {
    }

    SceneId thisScene() const override
    {
        return SceneId::GeneratorsScene;
    }

    SceneId nextScene() override
    {
        return _finishScreen;
    }

private:
    void renderImage(Image& img, int xpos, int ypos, int scale);
    Random _random{1234};
    SceneId _finishScreen = SceneId::GeneratorsScene;
    int _focus = 0;
    int _scroll = 0;
    SpaceshipParams _spaceshipParams{};
    Image _currentSpaceship{};
    int _pixelCellSize{10};
    int _colorsUsedForSpaceship{0};
    WaveParams _waveParams{};
    //Wave _noise{};
};
