#ifndef INCLUDE_SPRITESYSTEM_H
#define INCLUDE_SPRITESYSTEM_H
#include <ppl7.h>
#include <ppl7-grafix.h>
#include <vector>
#include "gamerenderer.h"
#include "colorpalette.h"
#include "sprite.h"

class GameViewport;
class SpriteSystem
{
public:
    class Item
    {
    public:
        Item();
        SpriteSystem* spritesystem;
        SpriteTexture* texture;
        int id;
        int x;                // 4 Byte
        int y;                // 4 Byte
        int z;                // 1 Byte
        uint32_t color_index; // 1 Byte
        int sprite_set;       // 2 Byte
        int sprite_no;        // 2 Byte
        float scale;          // 4 Byte	==> 18 Byte
        float rotation;       // 4 Byte   ==> 22 Byte
        ppl7::grafix::Rect boundary;
    };

private:
    const ColorPalette* palette;
    ppl7::Mutex mutex;
    int maxid;
    std::map<int, SpriteSystem::Item> sprite_list;
    std::map<uint64_t, const SpriteSystem::Item&> visible_sprite_map;
    std::vector<SpriteTexture*> spriteset;
    // SpriteTexture* spriteset[MAX_SPRITESETS + 1];
    float scale_factor;
    bool bSpritesVisible;

public:
    SpriteSystem();
    ~SpriteSystem();
    void clear();
    int addSprite(int x, int y, int z, int spriteset, int sprite_no, float sprite_scale, float sprite_rotation, uint32_t color_index);
    int addSprite(const Item& sprite);
    bool getSprite(int id, SpriteSystem::Item& sprite);
    void deleteSprite(int id);
    void modifySprite(const SpriteSystem::Item& item);
    void setVisible(bool visible);
    void setScaleFactor(float scale_factor);
    float getScaleFactor() const;
    bool isVisible() const;
    void setSpriteset(int no, SpriteTexture* spriteset);
    void setColorPalette(const ColorPalette& palette);
    void updateVisibleSpriteList(const ppl7::grafix::Point& worldcoords, const ppl7::grafix::Size& render_target_size);
    void draw(GameRenderer& renderer, const ppl7::grafix::Point& worldcoords) const;
    void save(ppl7::FileObject& file, unsigned char chunkid, unsigned char layer, unsigned char position) const;
    void load(const ppl7::ByteArrayPtr& ba);
    bool findMatchingSprite(const ppl7::grafix::Point& p, SpriteSystem::Item& sprite) const;

    void drawSelectedSpriteOutline(GameRenderer& renderer, const ppl7::grafix::Point& worldcoords, int id);

    size_t count() const;
    size_t countVisible() const;
};

#endif