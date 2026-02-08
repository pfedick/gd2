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
        TwoThirdBlockLower,
        ThirdBlockLower,
        TwoThirdBlockUpper,
        ThirdBlockUpper,
        EnemyBlocker,
        Platform,
        RampRight,
        RampLeft,
        CeilingRampRight,
        CeilingRampLeft,
        ShallowRampRightUpper,
        ShallowRampRightLower,
        ShallowRampLeftLower,
        ShallowRampLeftUpper,

        ShallowRampCeilingRightUpper,
        ShallowRampCeilingRightLower,
        ShallowRampCeilingLeftLower,
        ShallowRampCeilingLeftUpper,

        SteepRampLeftLower,
        SteepRampLeftUpper,
        SteepRampRightUpper,
        SteepRampRightLower,

        SteepRampCeilingLeftUpper,
        SteepRampCeilingLeftLower,
        SteepRampCeilingRightLower,
        SteepRampCeilingRightUpper,
        Ladder,
        Water,
        AirStream,
        Speer,
        Fire,
        MaxType // used for arrays
    };
};

class GameViewport;

class TileTypePlane
{
private:
    SpriteTexture* tiletypes;
    TileType::Type* tilematrix;
    int width, height;

public:
    TileTypePlane();
    TileTypePlane(const TileTypePlane&) = delete;
    TileTypePlane& operator=(const TileTypePlane&) = delete;
    ~TileTypePlane();
    void clear();
    void create(int width, int height);
    void setType(int x, int y, TileType::Type type);
    TileType::Type getType(int x, int y) const;
    TileType::Type getType(const ppl7::grafix::Point& player) const;
    // int getPlayerGround(const ppl7::grafix::Point& player) const;
    void setTileTypesSprites(SpriteTexture* sprites);
    void draw(GPUBatcher& batcher, const GameViewport& viewport, const ppl7::grafix::PointF& worldcoords, float scale) const;
    void save(ppl7::FileObject& file, unsigned char chunkid, unsigned char layer) const;
    void load(const ppl7::ByteArrayPtr& ba);
    ppl7::grafix::Rect getOccupiedArea() const;
    ppl7::grafix::Size size() const;
};

#endif