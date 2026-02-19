#include <ppl7.h>
#include <ppl7-grafix.h>
#include "objectsystem.h"
#include "audiopool.h"
#include "player.h"
#include "objects/generic.h"
#include "game.h"

namespace Objects
{
AnimationDefinition coin_rotate(14, 44, true, 0);

Representation Coin::representation()
{
    return Representation(SpritesetId::GenericObjects, 14);
}

Coin::Coin()
    : Object(Type::Coin)
{
    sprite_set = SpritesetId::GenericObjects;
    animation.startRandom(coin_rotate);
    collisionDetection = true;
    sprite_no_representation = 14;
    alwaysUpdate = false;
}

void Coin::update(const GameClock& clock, TileTypePlane&, Player&)
{
    if (animation.update(clock.time)) {
        int new_sprite = animation.getFrame();
        if (new_sprite != sprite_no) {
            sprite_no = new_sprite;
            updateBoundary();
        }
    }
}

void Coin::handleCollision(Player* player, const Collision&)
{
    enabled = false;
    if (spawned) deleteDefered = true;
    player->addPoints(10);
    player->countObject(type());
    AudioPool& audio = getAudioPool();

    int sample = ppl7::rand(0, 1);
    switch (sample) {
    case 1:
        audio.playOnce(AudioClip::coin2, 0.3f);
        break;
    default:
        audio.playOnce(AudioClip::coin1, 0.3f);
        break;
    }
}

} // namespace Objects
