#pragma once

#include "procedural.hpp"
#include "sprite.hpp"

#include <unordered_map>
#include <vector>

class SpriteManager
{
public:
    static SpriteManager* instance();

    void unloadSprites();

    Sprite getSprite(int id, Vector2 position = {0,0}, Vector2 velocity = {0,0}, Color color = WHITE) const;

    void generateAlien(int id, const SpaceshipParams& params, Sprite::Type type = Sprite::Alien);
    void generateAlien(int id, int minColors);

    void insertImage(int id, Image image, Sprite::Type type);

    void dump() const;

private:
    SpriteManager();
    ~SpriteManager() = default;
    struct SlotInfo {
        int xPos{0};
        int width{0};
        bool used{false};
    };
    struct RowInfo {
        int yPos{0};
        int height{0};
        std::vector<SlotInfo> slots;
    };
    Image _spriteSheet;
    Texture2D _spriteTexture;
    std::unordered_map<uint64_t,Sprite> _sprites;
    std::vector<RowInfo> _rowInfos;
};