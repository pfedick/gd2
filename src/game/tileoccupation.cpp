#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ppl7-grafix.h>
#include "tiles.h"

TileOccupation::Item::Item(int x, int y, Tile::Occupation o)
{
    this->x = x;
    this->y = y;
    this->o = o;
}

void TileOccupation::createFromSpriteTexture(const SpriteTexture& tex, int brick_width, int brick_height)
{
    tiles.clear();
    for (int i = 0; i < tex.numSprites(); i++) {
        createFromImage(i, tex.getDrawable(i), brick_width, brick_height);
    }
}

void TileOccupation::createFromImage(int id, const ppl7::grafix::Drawable& img, int brick_width, int brick_height)
{
    int height = img.height();
    int plate_height = brick_height / 3 - 1;
    int rows = img.height() / brick_height;
    int studs = img.width() / brick_width;
    int hw = brick_width / 2;
    if (!rows) rows = 1;

    // printf ("Find occupation of brick %d, ",id);
    // printf ("img(%d:%d), studs(%d:%d)\n",img.width(),img.height(),studs,rows);
    TileOccupation::Matrix m;
    ppl7::grafix::Color pixel;
    for (int row = 0; row < rows; row++) {
        for (int stud = 0; stud < studs; stud++) {
            Tile::Occupation occupation = Tile::Occupation::None;
            pixel = img.getPixel(stud * brick_width + hw, height - row * brick_height - plate_height);
            if (pixel.alpha() > 200) occupation |= Tile::Occupation::Plate0;
            pixel = img.getPixel(stud * brick_width + hw, height - row * brick_height - plate_height * 2);
            if (pixel.alpha() > 200) occupation |= Tile::Occupation::Plate1;
            pixel = img.getPixel(stud * brick_width + hw, height - row * brick_height - plate_height * 3);
            if (pixel.alpha() > 200) occupation |= Tile::Occupation::Plate2;
            if (occupation != Tile::Occupation::None) {
                TileOccupation::Item item(stud, row, (Tile::Occupation)occupation);
                m.push_back(item);
            }
        }
    }
    if (m.size()) {
        tiles.insert(std::pair<int, TileOccupation::Matrix>(id, m));
    }
    // printf("\n");
}

void TileOccupation::set(int id, const TileOccupation::Matrix& matrix)
{
    tiles.insert(std::pair<int, TileOccupation::Matrix>(id, matrix));
}

const TileOccupation::Matrix& TileOccupation::get(int id) const
{
    std::map<int, TileOccupation::Matrix>::const_iterator it;
    it = tiles.find(id);
    if (it != tiles.end()) return (it->second);
    return empty;
}
