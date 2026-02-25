#ifndef INCLUDE_TILE_H_
#define INCLUDE_TILE_H_
#include <ppl7.h>
#include <ppl7-grafix.h>
#include "sprite.h"
#include "gamerenderer.h"
#include "colorpalette.h"
#include "gameviewport.h"
#include <vector>

#define MAX_LAYERS_PER_TILE 5

class TileOccupation;

class Tile
{
public:
    enum class Occupation
    {
        None = 0,
        Plate0 = 1,
        Plate1 = 2,
        Plate2 = 4,
        Full = 7
    };
    // Bitweise OR Operator
    friend inline Occupation operator|(Occupation a, Occupation b)
    {
        return static_cast<Occupation>(static_cast<int>(a) | static_cast<int>(b));
    }

    // Bitweise AND Operator (hilfreich für Abfragen)
    friend inline Occupation operator&(Occupation a, Occupation b)
    {
        return static_cast<Occupation>(static_cast<int>(a) & static_cast<int>(b));
    }

    friend inline Occupation& operator|=(Occupation& a, Occupation b)
    {
        a = a | b;
        return a;
    }

    class TileLayer
    {
    public:
        int tileset;
        int tileno;
        int origin_x;
        int origin_y;
        int color_index;
        Occupation occupation;
        bool showStuds;
    };

    TileLayer tileLayers[MAX_LAYERS_PER_TILE];
    bool block_background;
    Tile();
    void setSprite(int z, int tileset, int tileno, int color_index, bool showStuds);
    bool hasSprite(int z) const;
    bool hasSprite() const;
    void setOccupation(int z, Occupation o, int origin_x = -1, int origin_y = -1);
};

class TileOccupation
{
public:
    class Item
    {
    public:
        Item(int x, int y, Tile::Occupation o);
        int x, y;
        Tile::Occupation o;
    };
    typedef std::list<TileOccupation::Item> Matrix;

private:
    std::map<int, TileOccupation::Matrix> tiles;
    TileOccupation::Matrix empty;

public:
    void createFromSpriteTexture(const SpriteTexture& tex, int brick_width, int brick_height);
    void createFromImage(int id, const ppl7::grafix::Drawable& img, int brick_width, int brick_height);
    void set(int id, const TileOccupation::Matrix& matrix);
    const TileOccupation::Matrix& get(int id) const;
};

class TileGrid
{
    // friend class Level;
private:
    Tile** tilematrix;
    int width, height;
    // SpriteSystem spritessystem[2];
    bool bTilesVisible;

    ppl7::grafix::Rect plane_dimension;
    size_t tile_count;

    std::vector<SpriteTexture*> tileset;
    ColorPalette* palette;

public:
    TileGrid();
    TileGrid(const TileGrid&) = delete;
    TileGrid& operator=(const TileGrid&) = delete;
    ~TileGrid();
    void clear();
    void create(int width, int height);
    ppl7::grafix::Size getSize() const;
    void setTile(int x, int y, int z, int tileset, int tileno, int color_index, bool showStuds = true);
    void setBlockBackground(int x, int y, bool block);
    void setOccupation(int x, int y, int z, Tile::Occupation o, int origin_x = -1, int origin_y = -1);
    Tile::Occupation getOccupation(int x, int y, int z);
    ppl7::grafix::Point getOccupationOrigin(int x, int y, int z);
    void setOccupation(int x, int y, int z, const TileOccupation::Matrix& matrix);
    void clearOccupation(int x, int y, int z, const TileOccupation::Matrix& matrix);
    bool isOccupied(int x, int y, int z, const TileOccupation::Matrix& matrix);
    void clearTile(int x, int y, int z);
    const Tile* get(int x, int y) const;
    void save(ppl7::FileObject& file, int chunk_id, int layer) const;
    void load(const ppl7::ByteArrayPtr& ba);
    void setVisible(bool visible);
    bool isVisible() const;
    int getTileNo(int x, int y, int z);
    int getTileSet(int x, int y, int z);
    int getColorIndex(int x, int y, int z);

    size_t tileCount() const;
    ppl7::grafix::Rect getOccupiedArea() const;

    void setTileset(int no, SpriteTexture* tileset);
    bool hasTileset(int no) const;
    void setColorPalette(ColorPalette& palette);
    void draw(GameRenderer& renderer, const GameViewport& viewport, const ppl7::grafix::PointF& worldcoords, float scale) const;
};

#endif