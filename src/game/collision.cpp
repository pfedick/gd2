#include <ppl7.h>
#include <ppl7-grafix.h>
#include "objectsystem.h"
#include "player.h"
#include "constants.h"

WorldCollision::WorldCollision()
{
    left = right = top = bottom = false;
    leftTile = rightTile = TileType::Type::NonBlocking;
}

bool IsBlocking(TileType::Type t, bool isEnemy)
{
    if (t == TileType::NonBlocking || (isEnemy == false && t == TileType::EnemyBlocker)) return false;
    if (t == TileType::Ladder || t == TileType::Water || t == TileType::AirStream || t == TileType::Platform || t == TileType::Speer ||
        t == TileType::Fire)
        return false;
    return true;
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
    if (sprite) {
        int h = TILE_HEIGHT / 2;
        ppl7::grafix::Rect box = sprite->spriteBoundary(sprite_no, scale, scale, rotation, x, y);
        for (int cy = box.y1 - offset + h; cy < box.y2 + offset - h; cy += TILE_HEIGHT / 2) {
            TileType::Type t = world.getType(ppl7::grafix::Point(box.x1 - offset, cy));
            if (!collision.left) collision.left = IsBlocking(t, isEnemy);
            t = world.getType(ppl7::grafix::Point(box.x2 + offset, cy));
            if (!collision.right) collision.right = IsBlocking(t, isEnemy);
        }
        for (int cx = box.x1 - offset; cx < box.x2 + offset; cx += TILE_WIDTH / 2) {
            TileType::Type t = world.getType(ppl7::grafix::Point(cx, box.y1 - offset));
            if (!collision.top) collision.top = IsBlocking(t, isEnemy);

            t = world.getType(ppl7::grafix::Point(cx, box.y2 + offset));
            if (!collision.bottom) collision.bottom = IsBlocking(t, isEnemy);
        }
        collision.leftTile = world.getType(ppl7::grafix::Point(x - (TILE_WIDTH / 2), y + 1));
        collision.rightTile = world.getType(ppl7::grafix::Point(x + (TILE_WIDTH / 2), y + 1));
    }

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
