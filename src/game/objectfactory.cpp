#include "objectsystem.h"
#include "objects/generic.h"

namespace Objects
{

ppl7::String ObjectName(Type type)
{
    switch (type) {
    case Type::PlayerStartpoint:
        return ppl7::String("PlayerStartpoint");
    case Type::SpawnPoint:
        return ppl7::String("SpawnPoint");
    case Type::SavePoint:
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
    case Type::ParticleEmitter:
        return Objects::ParticleEmitter::representation();
    case Type::SpawnPoint:
        return Objects::SpawnPoint::representation();
    case Type::Speaker:
        return Objects::Speaker::representation();
    case Type::SavePoint:
        return Objects::SavePoint::representation();
    case Type::Arrow:
        return Objects::Arrow::representation();
    case Type::Trigger:
        return Objects::Trigger::representation();
    case Type::TouchEmitter:
        return Objects::TouchEmitter::representation();
    case Type::ObjectWatcher:
        return Objects::ObjectWatcher::representation();

    default:
        return Object::representation();
    }
}

} // namespace Objects

Objects::Object* ObjectSystem::getInstance(Objects::Type object_type) const
{
    if (object_type == Objects::Type::Invalid) return NULL;
    switch (object_type) {
    case Objects::Type::PlayerStartpoint:
        return new Objects::PlayerStartPoint();
    case Objects::Type::Coin:
        return new Objects::Coin();
    case Objects::Type::Crystal:
        return new Objects::Crystal();
    case Objects::Type::ParticleEmitter:
        return new Objects::ParticleEmitter();
    case Objects::Type::SpawnPoint:
        return new Objects::SpawnPoint();
    case Objects::Type::Speaker:
        return new Objects::Speaker();
    case Objects::Type::SavePoint:
        return new Objects::SavePoint();
    case Objects::Type::Arrow:
        return new Objects::Arrow();
    case Objects::Type::Trigger:
        return new Objects::Trigger();
    case Objects::Type::TouchEmitter:
        return new Objects::TouchEmitter();
    case Objects::Type::ObjectWatcher:
        return new Objects::ObjectWatcher();

    default:
        break;
    }
    return NULL;
}
