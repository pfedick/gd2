#include <ppl7.h>
#include <ppl7-grafix.h>
#include "objectsystem.h"
#include "level.h"
#include "objects/generic.h"

namespace Objects
{

Representation::Representation(Objects::SpritesetId sprite_set, int sprite_no)
{
    this->sprite_set = sprite_set;
    this->sprite_no = sprite_no;
}

ppl7::String ObjectName(Type type)
{
    switch (type) {
    case Type::PlayerStartpoint:
        return ppl7::String("PlayerStartpoint");
    case Type::SpawnPoint:
        return ppl7::String("SpawnPoint");
    case Type::Savepoint:
        return ppl7::String("Savepoint");
    case Type::Medikit:
        return ppl7::String("Medikit");
    case Type::Crystal:
        return ppl7::String("Crystal");
    case Type::Diamond:
        return ppl7::String("Diamond");
    case Type::Coin:
        return ppl7::String("Coin");
    case Type::Speaker:
        return ppl7::String("Speaker");
    case Type::ExtraLife:
        return ppl7::String("ExtraLife");
    case Type::ParticleEmitter:
        return ppl7::String("ParticleEmitter");
    case Type::Projectile:
        return ppl7::String("Projectile");
    case Type::TouchEmitter:
        return ppl7::String("TouchEmitter");
    case Type::RainEmitter:
        return ppl7::String("RainEmitter");
    case Type::VoiceTrigger:
        return ppl7::String("VoiceTrigger");
    case Type::ObjectWatcher:
        return ppl7::String("ObjectWatcher");
    case Type::Trigger:
        return ppl7::String("Trigger");
    case Type::LightTrigger:
        return ppl7::String("LightTrigger");
    case Type::PlayerTrigger:
        return ppl7::String("PlayerTrigger");
    case Type::LevelModificator:
        return ppl7::String("LevelModificator");
    case Type::GlimmerNode:
        return ppl7::String("GlimmerNode");
    case Type::ItemTaker:
        return ppl7::String("ItemTaker");
    case Type::CameraControl:
        return ppl7::String("CameraControl");
    default:
        return ppl7::String("unknown object type: %d", static_cast<int>(type));
    }
}

Representation getRepresentation(Type object_type)
{
    switch (object_type) {
    case Type::PlayerStartpoint:
        return Objects::PlayerStartPoint::representation();
    case Type::Coin:
        return Objects::Coin::representation();
    case Type::Crystal:
        return Objects::Crystal::representation();

    default:
        return Object::representation();
    }
}

Object::Object(Type type)
{
    myParallaxLayer = ParallaxLayerId::Player;
    objectSystem = NULL;
    myType = type;
    sprite_set = SpritesetId::GenericObjects;
    sprite_no = 0;
    sprite_no_representation = 0;
    id = 0;
    texture = NULL;
    collisionDetection = false;
    visibleAtPlaytime = true;
    enabled = true;
    isInViewport = false;
    alwaysUpdate = true;
    // save_size=14;
    pixelExactCollision = true;
    spawned = false;
    deleteDefered = false;
    myLayer = Layer::BehindPlayer;
    scale = 1.0f;
    rotation = 0.0f;
    difficulty_matrix = 255;
    color_mod.set(255, 255, 255, 255);
}

Object::~Object()
{
}

Type Object::type() const
{
    return myType;
}

ppl7::String Object::typeName() const
{
    return ObjectName(myType);
}

void Object::updateBoundary()
{
    if (texture) {
        boundary = texture->spriteBoundary(sprite_no, 1.0f, p.x, p.y);
        initial_boundary = texture->spriteBoundary(sprite_no_representation, 1.0f, initial_p.x, initial_p.y);
    }
}

Representation Object::representation()
{
    return Representation(Objects::SpritesetId::GenericObjects, 0);
}

void Object::update(const GameClock&, TileTypePlane&, Player& player)
{
}

size_t Object::save(unsigned char* buffer, size_t size) const
{
    if (size < 17) return 0;
    ppl7::Poke8(buffer + 0, 1); // Object-Header-Version
    ppl7::Poke16(buffer + 1, static_cast<uint16_t>(myType));
    ppl7::Poke8(buffer + 3, static_cast<int>(myLayer));
    ppl7::Poke32(buffer + 4, id);
    ppl7::Poke32(buffer + 8, initial_p.x);
    ppl7::Poke32(buffer + 12, initial_p.y);
    ppl7::Poke8(buffer + 16, difficulty_matrix);
    return 17;
}

size_t Object::saveSize() const
{
    return 17;
}

size_t Object::load(const unsigned char* buffer, size_t size)
{

    if (size < 17) return 0;
    int version = ppl7::Peek8(buffer + 0);
    if (version != 1) return 0;
    myLayer = static_cast<Layer>(ppl7::Peek8(buffer + 3));
    id = ppl7::Peek32(buffer + 4);
    initial_p.x = ppl7::Peek32(buffer + 8);
    initial_p.y = ppl7::Peek32(buffer + 12);
    p = initial_p;
    difficulty_matrix = ppl7::Peek8(buffer + 16);
    return 17;
}

void Object::drawEditMode(GPUBatcher& batcher, const ppl7::grafix::Point& coords) const
{
    if (!spawned) {
        batcher.addSprite(*texture, sprite_no, initial_p.x + coords.x, initial_p.y + coords.y, scale, scale, rotation, color_mod);
    }

    ppl7::grafix::Color c = color_mod;
    c.setAlpha(90);
    batcher.addSprite(*texture, sprite_no, p.x + coords.x, p.y + coords.y, scale, scale, rotation, c);
}

void Object::draw(GPUBatcher& batcher, const ppl7::grafix::Point& coords) const
{
    if (myType == Type::PlayerStartpoint) {
        ppl7::PrintDebug("Drawing PlayerStartpoint at (%f, %f)\n", p.x, p.y);
    }
    batcher.addSprite(*texture, sprite_no, p.x + coords.x, p.y + coords.y, scale, scale, rotation, color_mod);
}

void Object::handleCollision(Player* player, const Collision& collision)
{
}

void Object::openUi()
{
}

void Object::reset()
{
    if (!spawned) enabled = true;
}

void Object::toggle(bool enabled, Object*)
{
    this->enabled = enabled;
}

void Object::trigger(Object*)
{
    enabled = !enabled;
}

bool Object::isEnabled() const
{
    return enabled;
}

void Object::updateSpriteset(SpritesetId spriteset)
{
    if (spriteset != this->sprite_set) {
        sprite_set = spriteset;
        texture = GetObjectSpritesets().getSpriteset(sprite_set);
    }
}

} // namespace Objects
