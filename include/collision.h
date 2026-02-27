#ifndef INCLUDE_COLLISION_H_
#define INCLUDE_COLLISION_H_

#include "tiletypes.h"

class WorldCollision
{
public:
    GameClock clock;
    bool left;
    bool right;
    bool top;
    bool bottom;

    TileType::Type leftTile;
    TileType::Type rightTile;
    WorldCollision();
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

#endif /* INCLUDE_COLLISION_H_ */