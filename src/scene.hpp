#pragma once

#include <raylib.h>

#include "config.hpp"
#include "sprite.hpp"
#include "particles.hpp"
#include "hash.hpp"

#include <cmath>
#include <variant>
#include <string>
#include <tuple>
#include <type_traits>
#include <unordered_map>

inline static const int screenWidth{900};
inline static const int screenHeight{600};

enum SoundId { None, Laser, Explosion };

enum class SceneId { StartupScene, SetupScene, IngameScene, GameOverScene, GeneratorsScene };

enum class RenderMode { Default, Unscaled, Upscale2 };

using Value = std::variant<std::nullptr_t,bool,int64_t,float,std::string,Color,Vector2,Vector3>;

class Scene
{
public:
    inline static const Color BasePalette[] = {
        // bluish black                     dark gray                       light grey                      bluish white
        {15,15,32,255}, {80,83,89,255}, {182,191,188,255}, {242,251,255,255},
        // turquoise,                       light blue                      medium blue                     dark blue
        {94,231,255,255}, {0,161,219,255}, {29,91,184,255}, {31,44,102,255},
        // dark green                       medium green                    light green                     yellow-green
        {27,82,69,255}, {46,143,70,255}, {88,217,46,255}, {203,255,112,255},
        // light yellow                     medium yellow                   orange                          red
        {255,255,143,255}, {255,223,43,255}, {240,119,26,255}, {227,34,57,255},
        // wine red                         chocolate                       brick                           dark orange
        {133,21,64,255}, {64,26,36,255}, {156,59,48,255}, {201,93,60,255},
        // light orange                     apricot                         violet                          purple
        {237,138,95,255}, {255,188,166,255}, {235,117,190,255}, {119,56,140,255}
    };
    inline static const size_t ColorTriples[] = {
        3,2,1,
        4,5,6,
        5,6,7,
        10,9,8,
        11,10,9,
        12,13,14,
        13,14,15,
        14,15,16,
        20,19,18,
        21,22,23,
        15,16,17,
        22,23,7
    };
    virtual ~Scene() = default;
    virtual void load() {}
    virtual void update(float /*dt_s*/) {}
    virtual RenderMode renderMode() const { return RenderMode::Default; }
    virtual void render() = 0;
    virtual void unload() {}
    virtual SceneId thisScene() const = 0;
    virtual SceneId nextScene() = 0;

    float width() const { return float(renderMode() == RenderMode::Upscale2 ? GetScreenWidth()/2 : GetScreenWidth()); }
    float height() const { return float(renderMode() == RenderMode::Upscale2 ? GetScreenHeight()/2 : GetScreenHeight()); }

    void drawTextCentered(const char *text, int y, int fontSize, Color tint);
    static void drawTextBoxed(const char *text, Rectangle rec, float fontSize, float spacing, bool wordWrap, Color tint);
    static void drawTextBoxedSelectable(const char *text, Rectangle rec, float fontSize, float spacing, bool wordWrap, Color tint, int selectStart, int selectLength, Color selectTint, Color selectBackTint);

    static void drawPixel(Vector2 pos, Color col)
    {
        DrawRectangle(static_cast<int>(pos.x + 0.5f), static_cast<int>(pos.y+0.5f), 1, 1, col);
    }

    static void setProperty(uint64_t id, const Value& value)
    {
        _properties.insert_or_assign(id, value);
    }

    template<typename T>
    static void setProperty(uint64_t id, const T& value)
    {
        if constexpr (std::is_integral<T>::value && !std::is_same<T, bool>::value) {
            _properties.insert_or_assign(id, Value{int64_t(value)});
        }

        else {
            _properties.insert_or_assign(id, Value{value});
        }
    }

    static const Value& getProperty(uint64_t id)
    {
        static Value dummy{nullptr};
        auto iter = _properties.find(id);
        return iter != _properties.end() ? iter->second : dummy;
    }

    template<typename T>
    static T getProperty(uint64_t id, const T& defaultValue)
    {
        auto iter = _properties.find(id);
        if(iter == _properties.end()) {
#ifndef NDEBUG
            TraceLog(LOG_ERROR, "Property '%ld' not found!", id);
#endif
            return defaultValue;
        }
        if constexpr (std::is_integral<T>::value && !std::is_same<T, bool>::value) {
            // std::nullptr_t,bool,int64_t,float,std::string,Color,Vector2,Vector3
            return std::visit(visitor{[defaultValue](const auto&) { return defaultValue; },
                                      [](const std::int64_t& val) { return (T)val; }}, iter->second);
        }
        else {
            return std::holds_alternative<T>(iter->second) ? std::get<T>(iter->second) : defaultValue;
        }
    }

 

    static Color getColor(int idx)
    {
        return BasePalette[idx];
    }

    static Color getTripleColor(int triple, int idx)
    {
        auto tripleOffset = triple * 3;
        return BasePalette[ColorTriples[tripleOffset + idx]];
    }
protected:
    inline static std::unordered_map<uint64_t, Value> _properties;
    inline static bool _clicked{false};
    inline static int64_t _frameCount{0};
};

template<>
inline uint64_t Scene::getProperty<uint64_t>(uint64_t id, const uint64_t& defaultValue)
{
    return static_cast<uint64_t>(getProperty(id, static_cast<int64_t>(defaultValue)));
}
