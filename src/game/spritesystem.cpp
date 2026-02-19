#include <stdio.h>
#include <stdlib.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include "spritesystem.h"

SpriteSystem::Item::Item()
{
    spritesystem = NULL;
    texture = NULL;
    id = 0;
    x = 0;
    y = 0;
    z = 0;
    sprite_set = 0;
    sprite_no = 0;
    scale = 1.0f;
    rotation = 0.0f;
    color_index = 0;
}

SpriteSystem::SpriteSystem()
{
    palette = NULL;
    bSpritesVisible = true;
    maxid = 0;
    scale_factor = 1.0f;
}

SpriteSystem::~SpriteSystem()
{
}

void SpriteSystem::setScaleFactor(float scale_factor)
{
    if (scale_factor != this->scale_factor) {
        this->scale_factor = scale_factor;
        // Update boundaries of all sprites
        for (auto& pair : sprite_list) {
            SpriteSystem::Item& item = pair.second;
            if (item.texture) {
                item.boundary = item.texture->spriteBoundary(item.sprite_no, item.scale * scale_factor, item.scale * scale_factor,
                                                             item.rotation, item.x, item.y);
            }
        }
    } else {
        this->scale_factor = scale_factor;
    }
}

float SpriteSystem::getScaleFactor() const
{
    return scale_factor;
}

void SpriteSystem::setColorPalette(const ColorPalette& palette)
{
    this->palette = &palette;
}

void SpriteSystem::setVisible(bool visible)
{
    bSpritesVisible = visible;
}

bool SpriteSystem::isVisible() const
{
    return bSpritesVisible;
}

void SpriteSystem::clear()
{
    sprite_list.clear();
    maxid = 0;
}

void SpriteSystem::setSpriteset(int no, SpriteTexture* spriteset)
{
    if (no < 0) return;
    /* OLD:
    this->spriteset[no] = spriteset;
    */
    if (no < (int)this->spriteset.size())
        this->spriteset[no] = spriteset;
    else {
        this->spriteset.resize(no + 1);
        this->spriteset[no] = spriteset;
    }
}

int SpriteSystem::addSprite(
    int x, int y, int z, int spriteset, int sprite_no, float sprite_scale, float sprite_rotation, uint32_t color_index)
{
    SpriteSystem::Item item;
    maxid++;
    item.id = maxid;
    item.x = x;
    item.y = y;
    item.z = z;
    item.sprite_no = sprite_no;
    item.sprite_set = spriteset;
    item.scale = sprite_scale;
    item.spritesystem = this;
    item.color_index = color_index;
    item.rotation = sprite_rotation;
    if (item.sprite_set < (int)this->spriteset.size() && this->spriteset[item.sprite_set] != NULL) {
        item.texture = this->spriteset[item.sprite_set];
        item.boundary = this->spriteset[item.sprite_set]->spriteBoundary(sprite_no, sprite_scale * scale_factor,
                                                                         sprite_scale * scale_factor, sprite_rotation, x, y);
    }

    sprite_list.insert(std::pair<int, SpriteSystem::Item>(item.id, item));
    return item.id;
}

int SpriteSystem::addSprite(const Item& sprite)
{
    return addSprite(sprite.x, sprite.y, sprite.z, sprite.sprite_set, sprite.sprite_no, sprite.scale, sprite.rotation, sprite.color_index);
}

bool SpriteSystem::getSprite(int id, SpriteSystem::Item& sprite)
{
    auto it = sprite_list.find(id);
    if (it != sprite_list.end()) {
        sprite = it->second;
        return true;
    }
    sprite.id = -1;
    return false;
}

void SpriteSystem::updateVisibleSpriteList(const ppl7::grafix::Point& worldcoords, const ppl7::grafix::Size& render_target_size)
{
    if (!bSpritesVisible) return;
    visible_sprite_map.clear();
    int view_x1 = worldcoords.x;
    int view_y1 = worldcoords.y;
    int view_x2 = worldcoords.x + render_target_size.width;
    int view_y2 = worldcoords.y + render_target_size.height;

    for (auto it = sprite_list.begin(); it != sprite_list.end(); ++it) {
        const SpriteSystem::Item& item = (it->second);
        if (item.texture) {
            if (item.boundary.x2 > view_x1 && item.boundary.x1 < view_x2 && item.boundary.y2 > view_y1 && item.boundary.y1 < view_y2) {
                // uint64_t id = ((uint64_t)item.z << 32 & 0x0000ffff00000000) | (uint64_t)(item.y << 16) | (uint64_t)item.x;
                uint64_t id =
                    ((uint64_t)item.z << 48 & 0x00ffff0000000000) | (uint64_t)((item.y + 0xffff) << 24) | (uint64_t)(item.x + 0xffff);

                visible_sprite_map.insert(std::pair<uint64_t, const SpriteSystem::Item&>(id, item));
            }
        }
    }
}

size_t SpriteSystem::count() const
{
    return sprite_list.size();
}

size_t SpriteSystem::countVisible() const
{
    return visible_sprite_map.size();
}

void SpriteSystem::draw(GPUBatcher& batcher, const ppl7::grafix::Point& worldcoords, float scale) const
{
    if (!bSpritesVisible) return;
    std::map<uint64_t, const SpriteSystem::Item&>::const_iterator it;
    // ppl7::PrintDebug("Drawing %zu sprites from %zu\n", visible_sprite_map.size(), sprite_list.size());
    for (it = visible_sprite_map.begin(); it != visible_sprite_map.end(); ++it) {
        const SpriteSystem::Item& item = (it->second);
        if (item.texture) {
            batcher.addSprite(*item.texture, item.sprite_no, item.x - worldcoords.x, item.y - worldcoords.y, item.scale * scale,
                              item.scale * scale, item.rotation, palette->getColor(item.color_index));
        }
    }
}

void SpriteSystem::drawSelectedSpriteOutline(GPUBatcher& batcher, const ppl7::grafix::Point& worldcoords, int id, float scale)
{
    if (!bSpritesVisible) return;
    std::map<int, SpriteSystem::Item>::const_iterator it;
    it = sprite_list.find(id);
    if (it != sprite_list.end()) {
        const SpriteSystem::Item& item = (it->second);
        if (item.texture) {
            batcher.addSpriteOutline(*item.texture, item.sprite_no, item.x - worldcoords.x, item.y - worldcoords.y, item.scale * scale,
                                     item.scale * scale, item.rotation);
        }
    }
}

void SpriteSystem::deleteSprite(int id)
{
    std::map<int, SpriteSystem::Item>::const_iterator it;
    it = sprite_list.find(id);
    if (it != sprite_list.end()) {
        sprite_list.erase(it);
    }
}

void SpriteSystem::modifySprite(const SpriteSystem::Item& item)
{
    std::map<int, SpriteSystem::Item>::iterator it;
    it = sprite_list.find(item.id);
    if (it != sprite_list.end()) {
        SpriteSystem::Item& intitem = (it->second);
        intitem.x = item.x;
        intitem.y = item.y;
        intitem.z = item.z;
        intitem.scale = item.scale;
        intitem.rotation = item.rotation;
        intitem.color_index = item.color_index;
        if (intitem.texture) {
            intitem.boundary = intitem.texture->spriteBoundary(intitem.sprite_no, intitem.scale * scale_factor,
                                                               intitem.scale * scale_factor, intitem.rotation, intitem.x, intitem.y);
        }
    }
}

static inline ppl7::grafix::Point rotate_point(const ppl7::grafix::Point& p, const SpriteSystem::Item& item)
{
    float s = sin(item.rotation * M_PI / 180.0f);
    float c = cos(item.rotation * M_PI / 180.0f);

    ppl7::grafix::Point pr = p;
    pr.x -= item.x;
    pr.y -= item.y;
    // rotate point
    float xnew = (float)pr.x * c + (float)pr.y * s;
    float ynew = (float)-pr.x * s + (float)pr.y * c;
    pr.x = xnew + item.x;
    pr.y = ynew + item.y;
    return pr;
}

bool SpriteSystem::findMatchingSprite(const ppl7::grafix::Point& p, SpriteSystem::Item& sprite, float scale) const
{
    bool found_match = false;
    sprite.id = -1;
    if (!bSpritesVisible) return false;
    // printf ("Try to find sprite\n");
    std::map<uint64_t, const SpriteSystem::Item&>::const_iterator it;
    for (it = visible_sprite_map.begin(); it != visible_sprite_map.end(); ++it) {
        const SpriteSystem::Item& item = (it->second);
        if (p.inside(item.boundary)) {
            // printf ("possible match: %d\n", item.id);
            if (item.texture) {
                const ppl7::grafix::Drawable draw = item.texture->getDrawable(item.sprite_no);
                if (draw.width()) {
                    // rotate and scale selected point
                    ppl7::grafix::Point rp = rotate_point(p, item);
                    ppl7::grafix::Point of = item.texture->spriteOffset(item.sprite_no);
                    int x = rp.x - item.x - ((float)of.x * item.scale * scale);
                    int y = rp.y - item.y - ((float)of.y * item.scale * scale);

                    /*ppl7::PrintDebugTime("point: %d:%d, pivot: %d:%d, rotated point: %d:%d, Item: %d:%d\n",
                        p.x, p.y, item.x, item.y, rp.x, rp.y, x, y
                    );
                    */

                    ppl7::grafix::Color c = draw.getPixel(x / (item.scale * scale), y / (item.scale * scale));
                    if (c.alpha() > 40) {
                        sprite = item;
                        found_match = true;
                    }
                }
            }
        }
    }
    return found_match;
}

void SpriteSystem::save(ppl7::FileObject& file, unsigned char chunkid, unsigned char layer, unsigned char position) const
{
    if (sprite_list.size() == 0) return;
    unsigned char* buffer = (unsigned char*)malloc(sprite_list.size() * 22 + 8);
    ppl7::Poke32(buffer + 0, 0);
    ppl7::Poke8(buffer + 4, chunkid);
    ppl7::Poke8(buffer + 5, layer);
    ppl7::Poke8(buffer + 6, position);
    ppl7::Poke8(buffer + 7, 1); // Version

    size_t p = 8;
    std::map<int, SpriteSystem::Item>::const_iterator it;
    for (it = sprite_list.begin(); it != sprite_list.end(); ++it) {
        const SpriteSystem::Item& item = (it->second);
        ppl7::Poke32(buffer + p, item.x);
        ppl7::Poke32(buffer + p + 4, item.y);
        ppl7::Poke8(buffer + p + 8, item.z);
        ppl7::Poke8(buffer + p + 9, item.color_index);
        ppl7::Poke16(buffer + p + 10, item.sprite_set);
        ppl7::Poke16(buffer + p + 12, item.sprite_no);
        ppl7::PokeFloat(buffer + p + 14, item.scale);
        ppl7::PokeFloat(buffer + p + 18, item.rotation);
        p += 22;
    }
    ppl7::Poke32(buffer + 0, p);
    file.write(buffer, p);
    free(buffer);
}

void SpriteSystem::load(const ppl7::ByteArrayPtr& ba)
{
    clear();
    const char* buffer = ba.toCharPtr();
    int version = ppl7::Peek8(buffer + 2);
    size_t p = 3;
    if (version == 1) {
        while (p < ba.size()) {
            addSprite(ppl7::Peek32(buffer + p), ppl7::Peek32(buffer + p + 4), ppl7::Peek8(buffer + p + 8), ppl7::Peek16(buffer + p + 10),
                      ppl7::Peek16(buffer + p + 12), ppl7::PeekFloat(buffer + p + 14), ppl7::PeekFloat(buffer + p + 18),
                      ppl7::Peek8(buffer + p + 9));
            p += 22;
        }

    } else {
        printf("Can't load SpriteSystem, unknown version! [%d]\n", version);
    }
}
