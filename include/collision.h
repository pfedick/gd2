#ifndef INCLUDE_COLLISION_H_
#define INCLUDE_COLLISION_H_

#include "tiletypes.h"

class WorldCollision
{
public:
    ppl7::grafix::Rect bounding_box;
    GameClock clock;
    const TileTypePlane* world;
    bool left;
    bool right;
    bool top;
    bool bottom;
    bool isEnemy;

    TileType::Type leftGroundTile;
    TileType::Type middleGroundTile;
    TileType::Type rightGroundTile;

    TileType::Type leftPivotTile;
    TileType::Type middlePivotTile;
    TileType::Type rightPivotTile;

    WorldCollision();

    void update(float x, float y);
};

WorldCollision GetWorldCollision(const GameClock& clock,
                                 const TileTypePlane& world,
                                 float x,
                                 float y,
                                 const SpriteTexture* sprite,
                                 int sprite_no,
                                 float scale,
                                 float rotation,
                                 bool isEnemy = false,
                                 int offset = 0);
WorldCollision GetWorldCollision(
    const GameClock& clock, const TileTypePlane& world, float x, float y, const ppl7::grafix::Rect& bounding_box, bool isEnemy = false);

#endif /* INCLUDE_COLLISION_H_ */