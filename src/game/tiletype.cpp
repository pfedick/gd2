#include "tiletypes.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

TileTypePlane::TileTypePlane()
{
    tiletypes = NULL;
    tilematrix = NULL;
    width = height = 0;
    tile_width = 32.0f;
    tile_height = 32.0f;
}

TileTypePlane::~TileTypePlane()
{
    clear();
}

void TileTypePlane::setTileSizes(float tile_width, float tile_height)
{
    this->tile_width = tile_width;
    this->tile_height = tile_height;
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
            tilematrix[y * width + x] = TileType::Type::NonBlocking;
        }
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
    int tx = player.x / tile_width;
    int ty = player.y / tile_height;
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

void TileTypePlane::draw(GPUBatcher& batcher, const ppl7::grafix::Rect& viewport, const ppl7::grafix::Point& worldcoords) const
{
    if (!tiletypes) return;

    const float TILE_WORLD_SIZE = 32.0f;  // Basisgröße eines Tiles in Welt-Koordinaten
    const float SPRITE_BASE_SIZE = 64.0f; // Basisgröße der Sprite-Grafik

    // Effektive Kamera-Position und Skalierung für DIESE Ebene berechnen
    // speed_factor ist in diesem Fall immer 1.0f
    float effective_cam_x = worldcoords.x;
    float effective_cam_y = worldcoords.y;

    float base_scale = viewport.width() / 1920.0f;
    float effective_scale = base_scale;

    // B. Culling: Sichtbare Tile-Indizes berechnen
    // ============================================

    // Wie viele Welt-Einheiten sind auf dem aktuellen Viewport sichtbar?
    float visible_world_width = viewport.width() / effective_scale;
    float visible_world_height = viewport.height() / effective_scale;

    // Start- und End-Indizes für die Schleifen
    int start_tile_x = static_cast<int>(floor(effective_cam_x / TILE_WORLD_SIZE));
    int end_tile_x = static_cast<int>(ceil((effective_cam_x + visible_world_width) / TILE_WORLD_SIZE));

    int start_tile_y = static_cast<int>(floor(effective_cam_y / TILE_WORLD_SIZE));
    int end_tile_y = static_cast<int>(ceil((effective_cam_y + visible_world_height) / TILE_WORLD_SIZE));

    // C. Rendern: Schleife über die sichtbaren Tiles
    // =============================================

    for (int y = start_tile_y; y < end_tile_y; y++) {
        for (int x = start_tile_x; x < end_tile_x; x++) {

            // Falls die Map "kachelt" (unendlich scrollt), hier den Modulo anwenden:
            // int wrapped_x = (x % map_width_in_tiles + map_width_in_tiles) % map_width_in_tiles;
            // int wrapped_y = (y % map_height_in_tiles + map_height_in_tiles) % map_height_in_tiles;
            // Tile& tile = layer.tiles.get(wrapped_x, wrapped_y);
            TileType::Type type = getType(x, y);
            if (type == 0) continue;
            //

            // --- Berechne finale Position und Skalierung für den GPUBatcher ---

            // Welt-Position des aktuellen Tiles
            float tile_world_x = x * TILE_WORLD_SIZE;
            float tile_world_y = y * TILE_WORLD_SIZE;

            // Bildschirm-Position (linke obere Ecke des Tiles)
            float screen_x = (tile_world_x - effective_cam_x) * effective_scale;
            float screen_y = (tile_world_y - effective_cam_y) * effective_scale;

            // Skalierungsfaktor für den Sprite. Wir wollen von 64px auf die finale Größe auf dem Schirm.
            float final_tile_size_on_screen = TILE_WORLD_SIZE * effective_scale;
            float sprite_scale_for_batcher = final_tile_size_on_screen / SPRITE_BASE_SIZE; // z.B. 32/64 = 0.5

            batcher.addSprite(*tiletypes, type, screen_x, screen_y, sprite_scale_for_batcher, sprite_scale_for_batcher);
        }
    }
}
/*
void TileTypePlane::draw(GPUBatcher& batcher, const ppl7::grafix::Rect& viewport, const ppl7::grafix::Point& worldcoords) const
{
    if (!tiletypes) return;
    int tiles_width = viewport.width() / tile_width + 2;
    int tiles_height = viewport.height() / tile_height + 2;
    int offset_x = worldcoords.x % tile_width;
    int offset_y = worldcoords.y % tile_height;
    int start_x = worldcoords.x / tile_width;
    int start_y = worldcoords.y / tile_height;
    int x1 = viewport.x1 - offset_x;
    int y1 = viewport.y1 - offset_y;

    for (int y = 0; y < tiles_height; y++) {
        for (int x = 0; x < tiles_width; x++) {
            TileType::Type type = getType(x + start_x, y + start_y);
            if (type > 0) {
                batcher.addSprite(*tiletypes, type, x1 + x * tile_width, y1 + y * tile_height);
            }
        }
    }
}
    */

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
