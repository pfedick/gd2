#include <ppl7.h>
#include <ppl7-grafix.h>
#include "objectsystem.h"
#include "player.h"
#include "animation.h"
#include "game.h"
#include "objects/generic.h"

namespace Objects
{

static AnimationDefinition savepoint_animation(76, 106, true, 0);

Representation SavePoint::representation()
{
    return Representation(SpritesetId::GenericObjects, 76);
}

SavePoint::SavePoint()
    : Object(Type::Savepoint)
{
    sprite_set = SpritesetId::GenericObjects;
    animation.startRandom(savepoint_animation);
    next_animation = 0.0f;
    collisionDetection = true;
    alwaysUpdate = false;
    sprite_no_representation = 76;
}

void SavePoint::update(const GameClock& clock, TileTypePlane&, Player&)
{
    if (animation.update(clock.time)) {
        int new_sprite = animation.getFrame();
        if (new_sprite != sprite_no) {
            sprite_no = new_sprite;
            updateBoundary();
        }
    }
}

void SavePoint::handleCollision(Player* player, const Collision&)
{
    player->setSavePoint(p);
    enabled = false;
    collisionDetection = false;
    AudioPool& audio = getAudioPool();
    audio.playOnce(AudioClip::savepoint_collected, 0.7f);
}

} // namespace Objects
