#include <ppl7.h>
#include <ppl7-grafix.h>
#include "objects.h"
#include "audiopool.h"
#include "player.h"
#include "objects/generic.h"

namespace Objects
{
AnimationDefinition coin_rotate(84, 113, true, 0);

Representation CoinReward::representation()
{
    return Representation(SpritesetId::GenericObjects, 84);
}

CoinReward::CoinReward()
    : Object(Type::ObjectType::Coin)
{
    sprite_set = Spriteset::GenericObjects;
    animation.startRandom(coin_rotate, sizeof(coin_rotate) / sizeof(int), true, 0);
    next_animation = 0.0f;
    collisionDetection = true;
    sprite_no_representation = 84;
    alwaysUpdate = false;
}

void CoinReward::update(double time, TileTypePlane&, Player&, float)
{
    if (time > next_animation) {
        next_animation = time + 0.056f;
        animation.update();
        int new_sprite = animation.getFrame();
        if (new_sprite != sprite_no) {
            sprite_no = new_sprite;
            updateBoundary();
        }
    }
}

void CoinReward::handleCollision(Player* player, const Collision&)
{
    enabled = false;
    if (spawned) deleteDefered = true;
    player->addPoints(10);
    player->countObject(type());
    AudioPool& audio = getAudioPool();

    int sample = ppl7::rand(0, 1);
    switch (sample) {
    case 1:
        audio.playOnce(AudioClip::coin3, 0.3f);
        break;
    default:
        audio.playOnce(AudioClip::coin1, 0.3f);
        break;
    }
}

} // namespace Objects
