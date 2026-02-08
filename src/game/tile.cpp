#include "tiles.h"
#include <stdio.h>
#include <stdlib.h>
#include <SDL3/SDL.h>
#include <ppl7-grafix.h>

Tile::Tile()
{
    for (int z = 0; z < MAX_LAYERS_PER_TILE; z++) {
        tileLayers[z].tileset = 0;
        tileLayers[z].tileno = 0;
        tileLayers[z].origin_x = 0;
        tileLayers[z].origin_y = 0;
        tileLayers[z].occupation = Occupation::None;
        tileLayers[z].showStuds = true;
        tileLayers[z].color_index = 0;
    }
    this->block_background = false;
}

void Tile::setSprite(int z, int tileset, int tileno, int color_index, bool showStuds)
{
    if (z < 0 || z >= MAX_LAYERS_PER_TILE) return;
    tileLayers[z].tileset = tileset;
    tileLayers[z].tileno = tileno;
    tileLayers[z].color_index = color_index;
    tileLayers[z].showStuds = showStuds;
}

void Tile::setOccupation(int z, Occupation o, int origin_x, int origin_y)
{
    if (z < 0 || z >= MAX_LAYERS_PER_TILE) return;
    tileLayers[z].occupation = o;
    tileLayers[z].origin_x = origin_x;
    tileLayers[z].origin_y = origin_y;
}

bool Tile::hasSprite(int z) const
{
    if (z < 0 || z >= MAX_LAYERS_PER_TILE) return false;
    if (tileLayers[z].tileset) return true;
    return false;
}

bool Tile::hasSprite() const
{
    for (int z = 0; z < MAX_LAYERS_PER_TILE; z++)
        if (tileLayers[z].tileset) return true;
    return false;
}
