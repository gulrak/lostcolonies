#include "spritemanager.hpp"


SpriteManager* SpriteManager::instance()
{
    static SpriteManager _instance;
    return &_instance;
}

SpriteManager::SpriteManager()
{
    _spriteSheet = GenImageColor(512,512, {0,0,0,0});
    _spriteTexture = LoadTextureFromImage(_spriteSheet);
    _rowInfos.reserve(128);
}

void SpriteManager::unloadSprites()
{
    UnloadTexture(_spriteTexture);
    UnloadImage(_spriteSheet);
}

Sprite SpriteManager::getSprite(int id, Vector2 position, Vector2 velocity, Color color) const
{
    static Sprite dummy{};
    auto iter = _sprites.find(id);
    if(iter != _sprites.end()) {
        auto result = iter->second;
        result._pos = position;
        result._velocity = velocity;
        result._col = color;
        return result;
    }
    return dummy;
}

void SpriteManager::generateAlien(int id, const SpaceshipParams& params, Sprite::Type type)
{
    auto img = Procedural::generateSpaceship(params);
    insertImage(id, img, type);
}

void SpriteManager::generateAlien(int id, int minColors)
{
    auto params = Procedural::randomAlienSpaceshipParams(minColors, 0);
    generateAlien(id, params);
}

void SpriteManager::insertImage(int id, Image image, Sprite::Type type)
{
    RowInfo* row = nullptr;
    auto roundedHeight = ((image.height + 7)&0x7ff8);
    for(auto& rowInfo : _rowInfos) {
        if(rowInfo.height == roundedHeight) {
            if(rowInfo.slots.empty() || _spriteSheet.width - rowInfo.slots.back().xPos - rowInfo.slots.back().width > image.width) {
                row = &rowInfo;
            }
        }
    }
    if(!row) {
        if(!_rowInfos.empty() && _spriteSheet.height - _rowInfos.back().yPos - _rowInfos.back().height < roundedHeight) {
            UnloadImage(image);
            TraceLog(LOG_ERROR, "SpriteManager: Couldn't allocate another sprite slot on sheet!");
            return;
        }
        _rowInfos.push_back({_rowInfos.empty() ? 0 : _rowInfos.back().yPos + _rowInfos.back().height, roundedHeight});
        row = &_rowInfos.back();
    }
    auto slot = SlotInfo{row->slots.empty() ? 0 : row->slots.back().xPos + row->slots.back().width, image.width, true};
    TraceLog(LOG_INFO, "SpriteManager: Allocated image slot at {%d,%d,%d,%d}", slot.xPos, row->yPos, image.width, image.height);
    row->slots.push_back(slot);
    ImageDraw(&_spriteSheet, image, {0, 0, image.width, image.height}, {slot.xPos, row->yPos, image.width, image.height}, WHITE);
    UnloadImage(image);
    UpdateTexture(_spriteTexture, _spriteSheet.data);
    _sprites.insert_or_assign(id, Sprite{type, {0,0}, {0,0}, {slot.xPos, row->yPos, image.width, image.height}, {0,0,0,0}, WHITE, 0, &_spriteTexture, &_spriteSheet});
}

void SpriteManager::dump() const
{
#ifndef NDEBUG
    ExportImage(_spriteSheet, "spritemanager-sheet.png");
#endif
}
