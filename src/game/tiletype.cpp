#include "tiletypes.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "gameviewport.h"
#include "constants.h"

TileTypePlane::TileTypePlane()
{
    tiletypes = NULL;
    tilematrix = NULL;
    width = height = 0;
}

TileTypePlane::~TileTypePlane()
{
    clear();
}

ppl7::grafix::Size TileTypePlane::size() const
{
    return ppl7::grafix::Size(width, height);
}

void TileTypePlane::clear()
{
    free(tilematrix);
    tilematrix = NULL;
    width = 0;
    height = 0;
}

void TileTypePlane::create(int width, int height)
{
    clear();
    this->width = width;
    this->height = height;
    tilematrix = (TileType::Type*)calloc(1, sizeof(TileType::Type) * (width + 1) * (height + 1));
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            tilematrix[y * width + x] = TileType::Type::Blocking;
        }
    }

    for (int i = 0; i < 10000; i++) {
        int tx = rand() % width;
        int ty = rand() % height;
        tilematrix[ty * width + tx] = (TileType::Type)(rand() % ((int)TileType::Type::MaxType - 1) + 1);
    }
}

void TileTypePlane::setType(int x, int y, TileType::Type type)
{
    if (x < 0 || x >= width || y < 0 || y >= height || tilematrix == NULL) return;
    tilematrix[y * width + x] = type;
}

TileType::Type TileTypePlane::getType(int x, int y) const
{
    if (x < 0 || x >= width || y < 0 || y >= height || tilematrix == NULL) return TileType::Type::NonBlocking;
    return tilematrix[y * width + x];
}

TileType::Type TileTypePlane::getType(const ppl7::grafix::Point& player) const
{
    // TODO: Wie wird das aussehen, wenn die Tile-Größe nicht 32x32 ist,
    // weil der Screen-Scale anders ist?
    int tx = player.x / (float)TILE_WIDTH;
    int ty = player.y / (float)TILE_HEIGHT;
    return getType(tx, ty);
}

/*
int TileTypePlane::getPlayerGround(const ppl7::grafix::Point& player) const
{
    int tx = player.x / tile_width;
    int ty = player.y / tile_height;
    TileType::Type type = getType(tx, ty);
    if (type != TileType::Type::NonBlocking) {

        if (type == TileType::Type::SteepRampLeft) {
            // int x=player.x%TILE_WIDTH;
        }

    }
    type = getType(tx, ty + 1);
    if (type != TileType::Type::NonBlocking) {
    }
    return 0;
}
*/

void TileTypePlane::setTileTypesSprites(SpriteTexture* sprites)
{
    this->tiletypes = sprites;
}

/*!\brief Draws the tile type plane using the provided GPUBatcher and GameViewport.
 *
 * This function calculates which tiles are visible within the current viewport
 * based on the world coordinates and draws only those tiles using the GPUBatcher.
 *
 * \param batcher The GPUBatcher used for rendering the tiles.
 * \param viewport The GameViewport that defines the visible area.
 * \param worldcoords The world coordinates representing the top-left corner of the viewport.
 */
void TileTypePlane::draw(GPUBatcher& batcher, const GameViewport& viewport, const ppl7::grafix::PointF& worldcoords, float scale) const
{
    if (!tiletypes) return;
    const ppl7::grafix::Size& render_target_size = viewport.getRenderSize();
    float scaled_tile_width = TILE_WIDTH * scale;
    float scaled_tile_height = TILE_HEIGHT * scale;

    int tiles_num_x = render_target_size.width / scaled_tile_width + 2;
    int tiles_num_y = render_target_size.height / scaled_tile_height + 2;

    // Start-Index in der Matrix berechnen
    int start_x = static_cast<int>(worldcoords.x / scaled_tile_width);
    int start_y = static_cast<int>(worldcoords.y / scaled_tile_height);

    // Den Pixel-Versatz berechnen (Modulo für Floats)
    float offset_x = worldcoords.x - (start_x * scaled_tile_width);
    float offset_y = worldcoords.y - (start_y * scaled_tile_height);

    float x1 = -offset_x;
    float y1 = -offset_y;

    for (int y = 0; y < tiles_num_y; y++) {
        for (int x = 0; x < tiles_num_x; x++) {
            TileType::Type type = getType(x + start_x, y + start_y);
            if (type > 0) {
                batcher.addSprite(*tiletypes, type, x1 + x * scaled_tile_width, y1 + y * scaled_tile_height, scale, scale);
            }
        }
    }
}

void TileTypePlane::save(ppl7::FileObject& file, unsigned char chunkid, unsigned char layer) const
{
    // We only save tiles with type>0
    if (tilematrix == NULL) return;
    unsigned char* buffer = (unsigned char*)malloc(10 + (width * height * 5));
    ppl7::Poke32(buffer + 0, 0);
    ppl7::Poke8(buffer + 4, chunkid);
    ppl7::Poke8(buffer + 5, layer);
    ppl7::Poke8(buffer + 6, 1); // Version
    ppl7::Poke16(buffer + 7, width);
    ppl7::Poke16(buffer + 9, height);
    size_t p = 11;
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            TileType::Type type = tilematrix[y * width + x];
            if ((int)type > 0) {
                ppl7::Poke16(buffer + p, x);
                ppl7::Poke16(buffer + p + 2, y);
                ppl7::Poke8(buffer + p + 4, (int)type);
                p += 5;
            }
        }
    }
    ppl7::Poke32(buffer + 0, p);
    file.write(buffer, p);
    free(buffer);
}

void TileTypePlane::load(const ppl7::ByteArrayPtr& ba)
{
    const char* buffer = ba.toCharPtr();
    int version = ppl7::Peek8(buffer + 1);
    size_t p = 2;
    width = ppl7::Peek16(buffer + p);
    height = ppl7::Peek16(buffer + p + 2);
    create(width, height);
    p += 4;
    if (version == 1) {
        while (p < ba.size()) {
            int x = ppl7::Peek16(buffer + p);
            int y = ppl7::Peek16(buffer + p + 2);
            int type = ppl7::Peek8(buffer + p + 4);
            setType(x, y, (TileType::Type)type);
            p += 5;
        }
    } else {
        printf("Can't load TileTypePlane, unknown version! [%d]\n", version);
    }
}

ppl7::grafix::Rect TileTypePlane::getOccupiedArea() const
{
    ppl7::grafix::Rect r;
    if (tilematrix) {
        size_t count = 0;
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                TileType::Type type = tilematrix[y * width + x];
                if ((int)type > 0) {
                    if (!count) {
                        r.x1 = x;
                        r.y1 = y;
                        r.x2 = x;
                        r.y2 = y;
                    } else {
                        if (x > r.x2) r.x2 = x;
                        if (x < r.x1) r.x1 = x;
                        if (y > r.y2) r.y2 = y;
                        if (y < r.y1) r.y1 = y;
                    }
                    count++;
                }
            }
        }
    }
    return r;
}
