#include <ppl7.h>
#include <ppl7-grafix.h>
#include "objectsystem.h"
#include "level.h"

namespace Objects
{

Representation::Representation(int sprite_set, int sprite_no)
{
    this->sprite_set = sprite_set;
    this->sprite_no = sprite_no;
}

ppl7::String Type::name(Type::ObjectType type)
{
    switch (type) {
    case ObjectType::PlayerStartpoint:
        return ppl7::String("PlayerStartpoint");
    case ObjectType::Savepoint:
        return ppl7::String("Savepoint");
    case ObjectType::Medikit:
        return ppl7::String("Medikit");

    case ObjectType::Crystal:
        return ppl7::String("Crystal");
    case ObjectType::Diamond:
        return ppl7::String("Diamond");
    case ObjectType::Coin:
        return ppl7::String("Coin");
    case ObjectType::ParticleEmitter:
        return ppl7::String("ParticleEmitter");
    case ObjectType::Speaker:
        return ppl7::String("Speaker");
    case ObjectType::Projectile:
        return ppl7::String("Projectile");
    case ObjectType::ExtraLife:
        return ppl7::String("ExtraLife");
    default:
        return ppl7::String("unknown object type: %d", static_cast<int>(type));
    }
}

Representation getRepresentation(int object_type)
{
    switch (object_type) {
    case Type::PlayerStartpoint:
        // return PlayerStartPoint::representation(); //TODO
        return Object::representation();

    default:
        return Object::representation();
    }
}

Object::Object(Type::ObjectType type)
{
    myPlane = ParallaxLayerId::Player;
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

Type::ObjectType Object::type() const
{
    return myType;
}

ppl7::String Object::typeName() const
{
    return Type::name(myType);
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
    return Representation(-1, 0);
}

void Object::update(double, TileTypePlane&, Player&, float)
{
}

size_t Object::save(unsigned char* buffer, size_t size) const
{
    if (size < 17) return 0;
    ppl7::Poke8(buffer + 0, 3); // Object-Header-Version
    ppl7::Poke16(buffer + 1, myType);
    ppl7::Poke8(buffer + 3, static_cast<int>(myLayer));
    ppl7::Poke32(buffer + 4, id);
    ppl7::Poke32(buffer + 8, initial_p.x);
    ppl7::Poke32(buffer + 12, initial_p.y);
    ppl7::Poke8(buffer + 16, difficulty_matrix);
    ppl7::Poke8(buffer + 17, static_cast<int>(myPlane));
    return 18;
}

size_t Object::saveSize() const
{
    return 18;
}

size_t Object::load(const unsigned char* buffer, size_t size)
{

    if (size < 16) return 0;
    int version = ppl7::Peek8(buffer + 0);
    if (version < 1 || version > 3) return 0;
    myLayer = static_cast<Layer>(ppl7::Peek8(buffer + 3));
    id = ppl7::Peek32(buffer + 4);
    if (static_cast<int>(myLayer) > 2) {
        ppl7::PrintDebug("Waring, found object with id %d and obsolete layer: %d\n", id, static_cast<int>(myLayer));
        return 0;
    }
    initial_p.x = ppl7::Peek32(buffer + 8);
    initial_p.y = ppl7::Peek32(buffer + 12);
    p = initial_p;
    if (version == 1) return 16;
    difficulty_matrix = ppl7::Peek8(buffer + 16);
    if (version == 2) return 17;
    myPlane = static_cast<ParallaxLayerId>(ppl7::Peek8(buffer + 17));
    return 18;
}

void Object::drawEditMode(GPUBatcher& batcher, const ppl7::grafix::Point& coords) const
{
    if (!spawned) {
        batcher.addSprite(*texture, sprite_no_representation, initial_p.x + coords.x, initial_p.y + coords.y, scale, scale, rotation,
                          color_mod);
    }

    ppl7::grafix::Color c = color_mod;
    c.setAlpha(90);
    batcher.addSprite(*texture, sprite_no_representation, p.x + coords.x, p.y + coords.y, scale, scale, rotation, c);
}

void Object::draw(GPUBatcher& batcher, const ppl7::grafix::Point& coords) const
{
    batcher.addSprite(*texture, sprite_no_representation, p.x + coords.x, p.y + coords.y + coords.y, scale, scale, rotation, color_mod);
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
