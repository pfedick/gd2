#ifndef INCLUDE_TILE_H_
#define INCLUDE_TILE_H_
#include <ppl7.h>
#include <ppl7-grafix.h>
#include "sprite.h"
#include <vector>

#define MAX_TILE_LAYER 4

class Tile
{
public:
    enum TileOccupation
    {
        OccupationNone = 0,
        OccupationPlate0 = 1,
        OccupationPlate1 = 2,
        OccupationPlate2 = 4,
        OccupationBrick = 7
    };
    class Layer
    {
    public:
        int tileset;
        int tileno;
        int origin_x;
        int origin_y;
        int color_index;
        TileOccupation occupation;
        bool showStuds;
    };

    Layer layer[MAX_TILE_LAYER];
    bool block_background;
    Tile();
    void setSprite(int z, int tileset, int tileno, int color_index, bool showStuds);
    bool hasSprite(int z) const;
    bool hasSprite() const;
    void setOccupation(int z, TileOccupation o, int origin_x = -1, int origin_y = -1);
};

class BrickOccupation
{
public:
    class Item
    {
    public:
        Item(int x, int y, Tile::TileOccupation o);
        int x, y;
        Tile::TileOccupation o;
    };
    typedef std::list<BrickOccupation::Item> Matrix;

private:
    std::map<int, BrickOccupation::Matrix> tiles;
    BrickOccupation::Matrix empty;

public:
    void createFromSpriteTexture(const SpriteTexture& tex, int brick_width, int brick_height);
    void createFromImage(int id, const ppl7::grafix::Drawable& img, int brick_width, int brick_height);
    void set(int id, const BrickOccupation::Matrix& matrix);
    const BrickOccupation::Matrix& get(int id);
};

class Plane
{
    // friend class Level;
private:
    Tile** tilematrix;
    int width, height;
    // SpriteSystem spritessystem[2];
    bool bTilesVisible;

    ppl7::grafix::Rect plane_dimension;
    size_t tile_count;

public:
    Plane();
    ~Plane();
    void clear();
    void create(int width, int height);
    ppl7::grafix::Size getSize() const;
    void setTile(int x, int y, int z, int tileset, int tileno, int color_index, bool showStuds = true);
    void setBlockBackground(int x, int y, bool block);
    void setOccupation(int x, int y, int z, Tile::TileOccupation o, int origin_x = -1, int origin_y = -1);
    Tile::TileOccupation getOccupation(int x, int y, int z);
    ppl7::grafix::Point getOccupationOrigin(int x, int y, int z);
    void setOccupation(int x, int y, int z, const BrickOccupation::Matrix& matrix);
    void clearOccupation(int x, int y, int z, const BrickOccupation::Matrix& matrix);
    bool isOccupied(int x, int y, int z, const BrickOccupation::Matrix& matrix);
    void clearTile(int x, int y, int z);
    const Tile* get(int x, int y) const;
    void save(ppl7::FileObject& file, unsigned char id) const;
    void load(const ppl7::ByteArrayPtr& ba);
    void setVisible(bool visible);
    bool isVisible() const;
    int getTileNo(int x, int y, int z);
    int getTileSet(int x, int y, int z);
    int getColorIndex(int x, int y, int z);

    size_t tileCount() const;
    ppl7::grafix::Rect getOccupiedArea() const;
};

#endif