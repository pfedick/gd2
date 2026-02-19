#include <ppl7.h>
#include <ppl7-grafix.h>
#include "objectsystem.h"
#include "objects/generic.h"

namespace Objects
{

Representation PlayerStartPoint::representation()
{
    return Representation(SpritesetId::GenericObjects, 1);
}

PlayerStartPoint::PlayerStartPoint()
    : Object(Type::PlayerStartpoint)
{
    sprite_set = SpritesetId::GenericObjects;
    sprite_no = 1;
    collisionDetection = false;
    visibleAtPlaytime = false;
    sprite_no_representation = 1;
}

} // namespace Objects
