#include <ppl7.h>
#include <ppl7-grafix.h>
#include "objectsystem.h"
#include "player.h"
#include "constants.h"

bool IsBlocking(TileType::Type t, bool isEnemy)
{
    if (t == TileType::NonBlocking || (isEnemy == false && t == TileType::EnemyBlocker)) return false;
    if (t == TileType::Blocking || (isEnemy == true && t == TileType::EnemyBlocker)) return true;
    if ((int)t >= (int)TileType::ShallowRampCeilingRightUpper && (int)t <= (int)TileType::ShallowRampCeilingLeftUpper) return true;
    if ((int)t >= (int)TileType::SteepRampCeilingLeftUpper && (int)t <= (int)TileType::SteepRampCeilingRightUpper) return true;
    return false;
}

static inline TileType::Type FilterType(TileType::Type t, bool isEnemy)
{
    if (t == TileType::NonBlocking || (isEnemy == false && t == TileType::EnemyBlocker)) return TileType::Type::NonBlocking;
    return t;
}

WorldCollision::WorldCollision()
{
    left = right = top = bottom = false;
    leftGroundTile = middleGroundTile = rightGroundTile = TileType::Type::NonBlocking;
    leftPivotTile = middlePivotTile = rightPivotTile = TileType::Type::NonBlocking;
}

void WorldCollision::update(float x, float y)
{
    left = right = top = bottom = false;
    if (bounding_box.width()) {
        ppl7::grafix::Point new_pivot((int)x, (int)y);
        ppl7::grafix::Point delta = new_pivot - last_pivot;
        if (delta.x) {
            bounding_box.x1 += delta.x;
            bounding_box.x2 += delta.x;
        }
        if (delta.y) {
            bounding_box.y1 += delta.y;
            bounding_box.y2 += delta.y;
        }
        int h = TILE_HEIGHT / 2;
        for (int cy = bounding_box.y1 + h; cy < bounding_box.y2 - h; cy += TILE_HEIGHT / 2) {
            TileType::Type t = world->getType(ppl7::grafix::Point(bounding_box.x1, cy));
            if (!left) left = IsBlocking(t, isEnemy);
            t = world->getType(ppl7::grafix::Point(bounding_box.x2, cy));
            if (!right) right = IsBlocking(t, isEnemy);
        }
        for (int cx = bounding_box.x1 + h; cx < bounding_box.x2 - h; cx += TILE_WIDTH / 2) {
            TileType::Type t = world->getType(ppl7::grafix::Point(cx, bounding_box.y1));
            if (!top) top = IsBlocking(t, isEnemy);

            t = world->getType(ppl7::grafix::Point(cx, bounding_box.y2));
            if (!bottom) bottom = IsBlocking(t, isEnemy);
        }
    }
    last_pivot.setPoint((int)x, (int)y);
    leftGroundTile = FilterType(world->getType(ppl7::grafix::Point(x - (TILE_WIDTH / 4), y + 1)), isEnemy);
    middleGroundTile = FilterType(world->getType(ppl7::grafix::Point(x, y + 1)), isEnemy);
    rightGroundTile = FilterType(world->getType(ppl7::grafix::Point(x + (TILE_WIDTH / 4), y + 1)), isEnemy);

    leftPivotTile = FilterType(world->getType(ppl7::grafix::Point(x - (TILE_WIDTH / 4), y)), isEnemy);
    middlePivotTile = FilterType(world->getType(ppl7::grafix::Point(x, y)), isEnemy);
    rightPivotTile = FilterType(world->getType(ppl7::grafix::Point(x + (TILE_WIDTH / 4), y)), isEnemy);
}

WorldCollision GetWorldCollision(const GameClock& clock,
                                 const TileTypePlane& world,
                                 float x,
                                 float y,
                                 const SpriteTexture* sprite,
                                 int sprite_no,
                                 float scale,
                                 float rotation,
                                 bool isEnemy,
                                 int offset)
{
    WorldCollision collision;
    collision.clock = clock;
    collision.world = &world;
    collision.isEnemy = isEnemy;
    if (sprite) {
        collision.bounding_box = sprite->spriteBoundary(sprite_no, scale, scale, rotation, x, y);
        collision.bounding_box.x1 -= offset;
        collision.bounding_box.y1 -= offset;
        collision.bounding_box.x2 += offset;
        collision.bounding_box.y2 += offset;
    }
    collision.last_pivot.setPoint((int)x, (int)y);
    collision.update(x, y);
    return collision;
}

WorldCollision GetWorldCollision(
    const GameClock& clock, const TileTypePlane& world, float x, float y, const ppl7::grafix::Rect& bounding_box, bool isEnemy)
{
    WorldCollision collision;
    collision.clock = clock;
    collision.world = &world;
    collision.isEnemy = isEnemy;
    collision.bounding_box = bounding_box;
    collision.last_pivot.setPoint((int)x, (int)y);
    collision.update(x, y);
    return collision;
}

namespace Objects
{

Collision::Collision(const GameClock& clock)
    : clock(clock)
{
    object = NULL;
}

Collision::Collision(const Collision& other)
    : clock(other.clock)
{
    object = other.object;
    collision_points = other.collision_points;
    bounding_box_object = other.bounding_box_object;
    bounding_box_player = other.bounding_box_player;
    bounding_box_intersection = other.bounding_box_intersection;
}

Collision::Collision(const GameClock& clock, const Player* player, const Object* object)
    : clock(clock)
{
    bounding_box_player = player->getBoundingBox();
    bounding_box_object = object->boundary;
    bounding_box_intersection = bounding_box_player.intersected(bounding_box_object);
}

void Collision::detect(Object* object, const std::list<ppl7::grafix::Point>& checkpoints, const Player& player)
{
    collision_points.clear();
    this->object = object;
    std::list<ppl7::grafix::Point>::const_iterator p_it;
    const ppl7::grafix::Drawable draw = object->texture->getDrawable(object->sprite_no);
    for (p_it = checkpoints.begin(); p_it != checkpoints.end(); ++p_it) {
        if (draw.width()) {
            int x = (*p_it).x - object->boundary.x1;
            int y = (*p_it).y - object->boundary.y1;
            ppl7::grafix::Color c = draw.getPixel(x, y);
            if (c.alpha() > 92) {
                collision_points.push_back(ppl7::grafix::Point((*p_it).x - player.x, (*p_it).y - player.y));
            }
        }
    }
}

const std::list<ppl7::grafix::Point>& Collision::getCollisionPoints() const
{
    return collision_points;
}

Object* Collision::getObject() const
{
    return object;
}

bool Collision::onFoot() const
{
    int height = bounding_box_object.height();
    if (height > 2 * TILE_HEIGHT)
        height = height * 2 / 3;
    else
        height = height / 2;
    if (bounding_box_intersection.y2 <= bounding_box_object.y2 - height) {
        return true;
    }
    return false;
}

bool Collision::objectBottom() const
{
    if (bounding_box_player.y2 > bounding_box_object.y2 && bounding_box_player.y1 < bounding_box_object.y2 &&
        bounding_box_player.y1 > bounding_box_object.y1) {
        return true;
    }
    return false;
}

bool Collision::objectTop() const
{
    if (bounding_box_player.y2 < bounding_box_object.y2 && bounding_box_player.y2 > bounding_box_object.y1 &&
        bounding_box_player.y1 < bounding_box_object.y1) {
        return true;
    }
    return false;
}

bool Collision::objectLeft() const
{
    if (bounding_box_player.x2 < bounding_box_object.x2 && bounding_box_player.x2 > bounding_box_object.x1 &&
        bounding_box_player.x1 < bounding_box_object.x1) {
        return true;
    }
    return false;
}

bool Collision::objectRight() const
{
    if (bounding_box_player.x1 > bounding_box_object.x1 && bounding_box_player.x1 < bounding_box_object.x2 &&
        bounding_box_player.x2 > bounding_box_object.x2) {
        return true;
    }
    return false;
}

bool Collision::objectBottom(int t) const
{
    if (bounding_box_player.y1 > bounding_box_object.y2 - t && bounding_box_player.y1 < bounding_box_object.y2) {
        return true;
    }
    return false;
}

bool Collision::objectTop(int t) const
{
    if (bounding_box_player.y2 < bounding_box_object.y1 + t && bounding_box_player.y2 > bounding_box_object.y1) {
        return true;
    }
    return false;
}

bool Collision::objectLeft(int t) const
{
    if (bounding_box_player.x2 < bounding_box_object.x1 + t && bounding_box_player.x2 > bounding_box_object.x1) {
        return true;
    }
    return false;
}

bool Collision::objectRight(int t) const
{
    if (bounding_box_player.x1 > bounding_box_object.x2 - t && bounding_box_player.x1 < bounding_box_object.x2) {
        return true;
    }
    return false;
}

bool Collision::playerBottom(int t) const
{
    if (bounding_box_object.y2 > bounding_box_player.y2 && bounding_box_object.y1 < bounding_box_player.y2 &&
        bounding_box_object.y1 > bounding_box_player.y2 - t) {
        return true;
    }
    return false;
}

bool Collision::playerTop(int t) const
{
    if (bounding_box_object.y2 < bounding_box_player.y1 + t && bounding_box_object.y2 > bounding_box_player.y1 &&
        bounding_box_object.y1 < bounding_box_player.y1) {
        return true;
    }
    return false;
}

bool Collision::playerLeft(int t) const
{
    if (bounding_box_player.x1 > bounding_box_object.x2 - t && bounding_box_player.x1 < bounding_box_object.x2) {
        return true;
    }
    return false;
}

bool Collision::playerRight(int t) const
{
    if (bounding_box_object.x1 > bounding_box_player.x1 && bounding_box_object.x1 < bounding_box_player.x2 - t &&
        bounding_box_object.x2 > bounding_box_player.x2) {
        return true;
    }
    return false;
}

ObjectCollision::ObjectCollision(const Object* this_object, const Object* other_object)
{
    this->this_object = this_object;
    this->other_object = other_object;
    bounding_box_this_object = this_object->boundary;
    bounding_box_other_object = other_object->boundary;
    bounding_box_intersection = bounding_box_this_object.intersected(bounding_box_other_object);
}

void ObjectCollision::update()
{
    bounding_box_this_object = this_object->boundary;
    bounding_box_other_object = other_object->boundary;
    bounding_box_intersection = bounding_box_this_object.intersected(bounding_box_other_object);
}

bool ObjectCollision::objectTop(int tolerance) const
{
    if (bounding_box_this_object.y1 < bounding_box_other_object.y2 && bounding_box_this_object.y1 > bounding_box_other_object.y1) {
        return true;
    }
    return false;
}

bool ObjectCollision::objectBottom(int tolerance) const
{
    if (bounding_box_this_object.y2 > bounding_box_other_object.y1 && bounding_box_this_object.y2 < bounding_box_other_object.y2) {
        return true;
    }
    return false;
}

bool ObjectCollision::objectLeft(int tolerance) const
{
    if (bounding_box_this_object.x1 < bounding_box_other_object.x2 && bounding_box_this_object.x1 > bounding_box_other_object.x1) {
        return true;
    }
    return false;
}

bool ObjectCollision::objectRight(int tolerance) const
{
    if (bounding_box_this_object.x2 > bounding_box_other_object.x1 && bounding_box_this_object.x2 < bounding_box_other_object.x2) {
        return true;
    }
    return false;
}

} // namespace Objects
