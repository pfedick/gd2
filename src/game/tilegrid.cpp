#include "tiles.h"
#include <stdio.h>
#include <stdlib.h>
#include <ppl7-grafix.h>
#include "gameviewport.h"
#include "gpu.h"
#include "constants.h"

TileGrid::TileGrid()
{
    tilematrix = NULL;
    width = 0;
    height = 0;
    bTilesVisible = true;
    tile_count = 0;
    palette = NULL;
}

TileGrid::~TileGrid()
{
    clear();
}

void TileGrid::clear()
{
    if (tilematrix) {
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                if (tilematrix[y * width + x]) delete tilematrix[y * width + x];
            }
        }

        free(tilematrix);
        tilematrix = NULL;
    }
    width = 0;
    height = 0;
    tile_count = 0;
}

void TileGrid::create(int width, int height)
{
    clear();
    this->width = width;
    this->height = height;
    tilematrix = (Tile**)calloc(1, sizeof(Tile*) * (width + 1) * (height + 1));
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            tilematrix[y * width + x] = (Tile*)NULL;
        }
    }
}

ppl7::grafix::Size TileGrid::getSize() const
{
    return ppl7::grafix::Size(width, height);
}

void TileGrid::setTile(int x, int y, int z, int tileset, int tileno, int color_index, bool showStuds)
{
    if (x < 0 || x >= width || y < 0 || y >= height || tilematrix == NULL) return;
    if (z < 0 || z >= MAX_LAYERS_PER_TILE) return;

    if (tilematrix[y * width + x] == NULL) {
        tilematrix[y * width + x] = new Tile();
    }
    if (!tilematrix[y * width + x]->hasSprite(z)) tile_count++;
    tilematrix[y * width + x]->setSprite(z, tileset, tileno, color_index, showStuds);
}

void TileGrid::setOccupation(int x, int y, int z, Tile::Occupation o, int origin_x, int origin_y)
{
    if (x < 0 || x >= width || y < 0 || y >= height || tilematrix == NULL) return;
    if (tilematrix[y * width + x] == NULL) {
        tilematrix[y * width + x] = new Tile();
    }
    tilematrix[y * width + x]->setOccupation(z, o, origin_x, origin_y);
}

void TileGrid::setBlockBackground(int x, int y, bool block)
{
    if (x < 0 || x >= width || y < 0 || y >= height || tilematrix == NULL) return;
    if (tilematrix[y * width + x] == NULL) {
        tilematrix[y * width + x] = new Tile();
    }
    tilematrix[y * width + x]->block_background = block;
}

void TileGrid::setOccupation(int x, int y, int z, const TileOccupation::Matrix& matrix)
{
    TileOccupation::Matrix::const_iterator it;
    for (it = matrix.begin(); it != matrix.end(); ++it) {
        const TileOccupation::Item& item = (*it);
        setOccupation(x + item.x, y - item.y, z, item.o, x, y);
    }
}

void TileGrid::clearOccupation(int x, int y, int z, const TileOccupation::Matrix& matrix)
{
    TileOccupation::Matrix::const_iterator it;
    for (it = matrix.begin(); it != matrix.end(); ++it) {
        const TileOccupation::Item& item = (*it);
        setOccupation(x + item.x, y - item.y, z, Tile::Occupation::None);
    }
    setOccupation(x, y, z, Tile::Occupation::None);
}

Tile::Occupation TileGrid::getOccupation(int x, int y, int z)
{
    if (x < 0 || x >= width || y < 0 || y >= height || tilematrix == NULL) return Tile::Occupation::None;
    if (tilematrix[y * width + x] == NULL || z < 0 || z >= MAX_LAYERS_PER_TILE) return Tile::Occupation::None;
    return tilematrix[y * width + x]->tileLayers[z].occupation;
}

bool TileGrid::isOccupied(int x, int y, int z, const TileOccupation::Matrix& matrix)
{
    TileOccupation::Matrix::const_iterator it;
    for (it = matrix.begin(); it != matrix.end(); ++it) {
        const TileOccupation::Item& item = (*it);
        Tile::Occupation o = getOccupation(x + item.x, y - item.y, z);
        if (o != Tile::Occupation::None) return true;
    }
    return false;
}

void TileGrid::clearTile(int x, int y, int z)
{
    if (x < 0 || x >= width || y < 0 || y >= height || tilematrix == NULL) return;
    if (z < 0 || z >= MAX_LAYERS_PER_TILE) return;
    if (tilematrix[y * width + x] != NULL) {
        if (!tilematrix[y * width + x]->hasSprite(z) && tile_count > 0) tile_count--;
        tilematrix[y * width + x]->setSprite(z, 0, 0, 0, true);
        tilematrix[y * width + x]->setOccupation(z, Tile::Occupation::None);
    }
}

const Tile* TileGrid::get(int x, int y) const
{
    if (x < 0 || x >= width || y < 0 || y >= height || tilematrix == NULL) return NULL;
    return tilematrix[y * width + x];
}

int TileGrid::getTileNo(int x, int y, int z)
{
    if (z < 0 || z >= MAX_LAYERS_PER_TILE) return -1;
    const Tile* t = get(x, y);
    if (!t) return -1;
    return t->tileLayers[z].tileno;
}

int TileGrid::getTileSet(int x, int y, int z)
{
    if (z < 0 || z >= MAX_LAYERS_PER_TILE) return -1;
    const Tile* t = get(x, y);
    if (!t) return -1;
    return t->tileLayers[z].tileset;
}

int TileGrid::getColorIndex(int x, int y, int z)
{
    if (z < 0 || z >= MAX_LAYERS_PER_TILE) return -1;
    const Tile* t = get(x, y);
    if (!t) return -1;
    return t->tileLayers[z].color_index;
}

ppl7::grafix::Point TileGrid::getOccupationOrigin(int x, int y, int z)
{
    const Tile* t = get(x, y);
    if (t == NULL || z < 0 || z >= MAX_LAYERS_PER_TILE) return ppl7::grafix::Point(-1, -1);
    return ppl7::grafix::Point(t->tileLayers[z].origin_x, t->tileLayers[z].origin_y);
}

void TileGrid::setVisible(bool visible)
{
    bTilesVisible = visible;
}

bool TileGrid::isVisible() const
{
    return bTilesVisible;
}

void TileGrid::save(ppl7::FileObject& file, int chunk_id, int parallax_layer) const
{
    if (tilematrix == NULL) return;
    // calculate required size
    size_t buffersize = 11;
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            const Tile* t = tilematrix[y * width + x];
            if (t) {
                buffersize += (5 + MAX_LAYERS_PER_TILE * 11);
            }
        }
    }
    unsigned char* buffer = (unsigned char*)malloc(buffersize);
    ppl7::Poke32(buffer + 0, 0);
    ppl7::Poke8(buffer + 4, chunk_id);
    ppl7::Poke8(buffer + 5, parallax_layer); // ParallaxLayer
    ppl7::Poke8(buffer + 6, 2);              // Version
    ppl7::Poke16(buffer + 7, width);
    ppl7::Poke16(buffer + 9, height);
    ppl7::Poke8(buffer + 11, MAX_LAYERS_PER_TILE);
    size_t p = 12;
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            const Tile* t = tilematrix[y * width + x];
            if (t) {
                ppl7::Poke16(buffer + p, x);
                ppl7::Poke16(buffer + p + 2, y);
                ppl7::Poke8(buffer + p + 4, (int)t->block_background);
                p += 5;
                for (int z = 0; z < MAX_LAYERS_PER_TILE; z++) {
                    ppl7::Poke16(buffer + p, t->tileLayers[z].tileset);
                    ppl7::Poke16(buffer + p + 2, t->tileLayers[z].tileno);
                    ppl7::Poke16(buffer + p + 4, t->tileLayers[z].origin_x);
                    ppl7::Poke16(buffer + p + 6, t->tileLayers[z].origin_y);
                    ppl7::Poke8(buffer + p + 8, static_cast<uint8_t>(t->tileLayers[z].occupation));
                    ppl7::Poke8(buffer + p + 9, t->tileLayers[z].showStuds);
                    ppl7::Poke8(buffer + p + 10, t->tileLayers[z].color_index);
                    p += 11;
                }
            }
        }
    }
    // ppl7::PrintDebugTime("saving plane, rquired size: %d, realsize: %d\n", buffersize,p);
    ppl7::Poke32(buffer + 0, p);
    file.write(buffer, p);
    free(buffer);
}

void TileGrid::load(const ppl7::ByteArrayPtr& ba)
{
    // ppl7::PrintDebug("Loading TileGrid from ByteArray of size %zu\n", ba.size());
    const char* buffer = ba.toCharPtr();
    int version = ppl7::Peek8(buffer + 1);
    size_t p = 2;
    if (version == 2) {
        width = ppl7::Peek16(buffer + p);
        height = ppl7::Peek16(buffer + p + 2);
        int max_tile_layers = ppl7::Peek8(buffer + p + 4);
        p += 5;
        // printf ("width: %d, height: %d\n",width,height);
        create(width, height);
        while (p < ba.size()) {
            int x = ppl7::Peek16(buffer + p);
            int y = ppl7::Peek16(buffer + p + 2);
            bool block_background = (bool)ppl7::Peek8(buffer + p + 4);
            p += 5;
            for (int z = 0; z < max_tile_layers; z++) {
                int tileset = ppl7::Peek16(buffer + p);
                int tileno = ppl7::Peek16(buffer + p + 2);
                int origin_x = ppl7::Peek16(buffer + p + 4);
                int origin_y = ppl7::Peek16(buffer + p + 6);
                int occupation = ppl7::Peek8(buffer + p + 8);
                bool showStuds = ppl7::Peek8(buffer + p + 9);
                int color_index = ppl7::Peek8(buffer + p + 10);
                if (tileset > 2) tileset = 2;
                setTile(x, y, z, tileset, tileno, color_index, showStuds);
                setOccupation(x, y, z, (Tile::Occupation)occupation, origin_x, origin_y);
                p += 11;
            }
            setBlockBackground(x, y, block_background);
        }
    } else {
        printf("Can't load Plane, unknown version! [%d]\n", version);
    }
    // printf("Plane hat %zd tiles\n", tileCount());
}

ppl7::grafix::Rect TileGrid::getOccupiedArea() const
{
    ppl7::grafix::Rect r;
    r.x1 = width;
    r.y1 = height;
    r.x2 = 0;
    r.y2 = 0;
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            const Tile* t = tilematrix[y * width + x];
            if (t && t->hasSprite()) {
                if (x > r.x2) r.x2 = x;
                if (x < r.x1) r.x1 = x;
                if (y > r.y2) r.y2 = y;
                if (y < r.y1) r.y1 = y;
            }
        }
    }
    return r;
}

size_t TileGrid::tileCount() const
{
    return tile_count;
}

void TileGrid::setTileset(int no, SpriteTexture* tileset)
{
    if (no >= (int)this->tileset.size()) {
        this->tileset.resize(no + 1, nullptr);
    }
    this->tileset[no] = tileset;
}

void TileGrid::setColorPalette(ColorPalette& palette)
{
    this->palette = &palette;
}

bool TileGrid::hasTileset(int no) const
{
    return no >= 0 && no < (int)this->tileset.size() && this->tileset[no] != nullptr;
}

void TileGrid::draw(GPUBatcher& batcher, const GameViewport& viewport, const ppl7::grafix::PointF& worldcoords, float scale) const
{
    // ppl7::PrintDebugTime("Drawing TileGrid at worldcoords %.2f/%.2f, scale %.2f\n", worldcoords.x, worldcoords.y, scale);
    if (!palette) return;
    if (!bTilesVisible) return;

    const ppl7::grafix::Size& render_target_size = viewport.getLogicalSize();
    float scaled_tile_width = TILE_WIDTH * scale;
    float scaled_tile_height = TILE_HEIGHT * scale;

    int tiles_num_x = render_target_size.width / scaled_tile_width + 4;
    int tiles_num_y = render_target_size.height / scaled_tile_height + 4;

    // Start-Index in der Matrix berechnen
    int start_x = static_cast<int>(worldcoords.x / scaled_tile_width) - 2;
    int start_y = static_cast<int>(worldcoords.y / scaled_tile_height) - 2;
    // if (start_x < 0) start_x = 0;
    // if (start_y < 0) start_y = 0;

    // Den Pixel-Versatz berechnen (Modulo für Floats)
    float offset_x = worldcoords.x - (start_x * scaled_tile_width);
    float offset_y = worldcoords.y - (start_y * scaled_tile_height);

    float x1 = -offset_x;
    float y1 = scaled_tile_height - offset_y;
    // ppl7::PrintDebug("Drawing TileGrid from %d/%d, num tiles %d/%d, offset %.2f/%.2f\n", start_x, start_y, tiles_num_x, tiles_num_y,
    //                  offset_x, offset_y);
    for (int z = 0; z < MAX_LAYERS_PER_TILE; z++) {
        for (int y = tiles_num_y - 1; y >= 0; y--) {
            for (int x = 0; x < tiles_num_x; x++) {
                const Tile* tile = get(x + start_x, y + start_y);
                if (tile != NULL && tile->tileLayers[z].tileset) {
                    batcher.addSprite(*tileset[tile->tileLayers[z].tileset], tile->tileLayers[z].tileno, x1 + x * scaled_tile_width,
                                      y1 + y * scaled_tile_height, scale, scale, 0.0f, palette->getColor(tile->tileLayers[z].color_index));
                }
            }
        }
    }
}