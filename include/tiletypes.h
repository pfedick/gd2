#ifndef INCLUDE_TILETYPES_H_
#define INCLUDE_TILETYPES_H_
#include <ppl7.h>
#include <ppl7-grafix.h>
#include "sprite.h"
#include "gpu.h"

class TileType
{
public:
    enum Type
    {
        NonBlocking,
        Blocking,
        SteepRampLeft,
        SteepRampRight,
        ShallowRampLeftLower,
        ShallowRampLeftUpper,
        ShallowRampRightUpper,
        ShallowRampRightLower,
        Ladder,
        Water,
        Plate1h,
        Plate2h,
        Speer,
        Fire,
        AirStream,
        EnemyBlocker,
        BlockFromTop,
        MaxType // used for arrays
    };
};

class TileTypePlane
{
private:
    SpriteTexture* tiletypes;
    TileType::Type* tilematrix;
    int width, height;
    int tile_width;
    int tile_height;

public:
    TileTypePlane();
    ~TileTypePlane();
    void clear();
    void setTileSizes(float tile_width, float tile_height);
    void create(int width, int height);
    void setType(int x, int y, TileType::Type type);
    TileType::Type getType(int x, int y) const;
    TileType::Type getType(const ppl7::grafix::Point& player) const;
    int getPlayerGround(const ppl7::grafix::Point& player) const;
    void setTileTypesSprites(SpriteTexture* sprites);
    void draw(GPUBatcher& batcher, const ppl7::grafix::Rect& viewport, const ppl7::grafix::Point& worldcoords) const;
    void save(ppl7::FileObject& file, unsigned char id) const;
    void load(const ppl7::ByteArrayPtr& ba);
    ppl7::grafix::Rect getOccupiedArea() const;
    ppl7::grafix::Size size() const;
};

#endif