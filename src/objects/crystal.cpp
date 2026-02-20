#include <ppl7.h>
#include <ppl7-grafix.h>
#include "objectsystem.h"
#include "audiopool.h"
#include "player.h"
#include "objects/generic.h"
#include "game.h"

namespace Objects
{

static AnimationDefinition crystal_rotate(45, 75, true, 0, 1.0f / 18.0f);

Representation Crystal::representation()
{
    return Representation(SpritesetId::GenericObjects, 45);
}

Crystal::Crystal()
    : Object(Type::Crystal)
{
    sprite_set = SpritesetId::GenericObjects;
    animation.startRandom(crystal_rotate);
    collisionDetection = true;
    alwaysUpdate = false;
    /*TODO

    int r = ppl7::rand(0, 4);
    light_glow.color.set(random_colors[r].r, random_colors[r].g, random_colors[r].b, 255);

    light_glow.sprite_no = 0;
    light_glow.scale_x = 0.3f;
    light_glow.scale_y = 0.3f;
    light_glow.intensity = 192;
    light_glow.plane = static_cast<int>(LightPlaneId::Player);
    light_glow.playerPlane = static_cast<int>(LightPlayerPlaneMatrix::Player) | static_cast<int>(LightPlayerPlaneMatrix::Back);
    */
}

void Crystal::update(const GameClock& clock, TileTypePlane&, Player&)
{
    if (animation.update(clock.time)) {
        int new_sprite = animation.getFrame();
        if (new_sprite != sprite_no) {
            sprite_no = new_sprite;
            updateBoundary();
        }
    }
    /* TODO
    light_glow.x = p.x;
    int yy = sprite_no - 120;
    if (yy > 15) yy = 30 - yy;
    light_glow.y = p.y - 15 - yy * 2;

    light_glow.scale_x = 0.4f + yy * 0.02;
    light_glow.scale_y = 0.4f;

    LightSystem& lights = GetGame().getLightSystem();
    lights.addObjectLight(&light_glow);
    */
}

void Crystal::handleCollision(Player* player, const Collision&)
{
    enabled = false;
    if (spawned) deleteDefered = true;
    player->addPoints(100);
    player->countObject(type());
    AudioPool& audio = getAudioPool();
    int sample = ppl7::rand(0, 2);
    switch (sample) {
    case 0:
        audio.playOnce(AudioClip::crystal2, 0.4f);
        break;
    case 1:
        audio.playOnce(AudioClip::crystal3, 0.4f);
        break;
    case 2:
        audio.playOnce(AudioClip::crystal1, 0.4f);
    }
}

} // namespace Objects