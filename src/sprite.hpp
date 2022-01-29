#pragma once

#include <raylib.h>
#include <raymath.h>
#include <algorithm>
#include <vector>
#include "math.hpp"

struct Sprite
{
    enum Type { Unused, Alien, Player, ColonyShip, AlienShot, AlienBomb, PlayerShot, DotParticle };
    Type _type{Unused};
    Vector2 _pos{};
    Vector2 _velocity{};
    Rectangle _sprite{};
    Rectangle _sprite2{};
    Color _col{WHITE};
    float _age{0};

    Texture* _texture{nullptr};
    Image* _image{nullptr};
    Vector2 _posAlt{-1000,-1000};
    std::vector<Vector3> _flightPath{};

    void update(float dt_s)
    {
        _pos.x += _velocity.x * dt_s;
        _pos.y += _velocity.y * dt_s;
        //if(!_flightPath)
    }

    void draw(bool drawAlternative = false) const
    {
        if(drawAlternative && _sprite2.width) {
            DrawTextureRec(*_texture, _sprite2, _pos, _col);
        }
        else {
            DrawTextureRec(*_texture, _sprite, _pos, _col);
        }
    }

    void draw(Vector2 offset, bool drawAlternative = false) const
    {
        if(offset.y + _pos.y + _sprite.height > 0 && offset.y + _pos.y < 400) {
            if(drawAlternative && _sprite2.width) {
                DrawTextureRec(*_texture, _sprite2, offset + _pos, _col);
            }
            else {
                DrawTextureRec(*_texture, _sprite, offset + _pos, _col);
            }
        }
    }

    Vector2 screenToSpriteCoordinates(const Vector2& screenCoordinates) const
    {
        return Vector2Add(Vector2Subtract(screenCoordinates, _pos), {_sprite.x, _sprite.y});
    }

    bool isCollidingRect(const Sprite& other)
    {
        return CheckCollisionRecs({_pos.x, _pos.y, _sprite.width, _sprite.height}, {other._pos.x, other._pos.y, other._sprite.width, other._sprite.height});
    }

    bool isColliding(const Sprite& other)
    {
        if(!isCollidingRect(other)) {
            return false;
        }
        Rectangle overlap{};
        overlap.x = std::max(_pos.x, other._pos.x);
        overlap.width = std::min(_pos.x + _sprite.width, other._pos.x + other._sprite.width) - overlap.x;
        overlap.y = std::max(_pos.y, other._pos.y);
        overlap.height = std::min(_pos.y + _sprite.height, other._pos.y + other._sprite.height) - overlap.y;
        for (int x = overlap.x; x < int(overlap.x + overlap.width); ++x) {
            for (int y = overlap.y; y < int(overlap.y + overlap.height); ++y) {
                auto pos1 = screenToSpriteCoordinates({float(x), float(y)});
                auto c1 = GetImageColor(*_image, pos1.x, pos1.y);
                auto pos2 = other.screenToSpriteCoordinates({float(x), float(y)});
                auto c2 = GetImageColor(*_image, pos2.x, pos2.y);
                if (c1.a > 128 && c2.a > 128) {
                    return true;
                }
            }
        }
        return false;
    }
};