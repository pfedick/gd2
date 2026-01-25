#include "gpu.h"
#include "objectsystem.h"
#include "sprite.h"

static ObjectSpritesets* global_spritesets;

ObjectSpritesets& GetObjectSpritesets()
{
    return *global_spritesets;
}

ObjectSpritesets::ObjectSpritesets()
{
    global_spritesets = this;
    gpu = NULL;
}

ObjectSpritesets::~ObjectSpritesets()
{
    if (gpu) {
        for (auto it = spriteset_vector.begin(); it != spriteset_vector.end(); ++it) {
            delete *it;
        }
    }
    spriteset_vector.clear();
    global_spritesets = NULL;
}

void ObjectSpritesets::addSpriteset(Objects::SpritesetId id, SpriteTexture* spriteset)
{
    size_t index = static_cast<size_t>(id);
    if (index >= spriteset_vector.size()) {
        spriteset_vector.resize(index + 1, NULL);
    }
    spriteset_vector[index] = spriteset;
}

void ObjectSpritesets::loadAll(GPUContext& gpu)
{
    this->gpu = &gpu;
    addSpriteset(Objects::SpritesetId::GenericObjects,
                 new SpriteTexture(gpu, "res/objects/generic.tex", SpriteBuffer::GPU | SpriteBuffer::Memory));
}

SpriteTexture* ObjectSpritesets::getSpriteset(int id) const
{
    size_t index = static_cast<size_t>(id);
    if (index < spriteset_vector.size()) {
        return spriteset_vector[index];
    }
    return NULL;
}

SpriteTexture* ObjectSpritesets::getSpriteset(Objects::SpritesetId id) const
{
    size_t index = static_cast<size_t>(id);
    if (index < spriteset_vector.size()) {
        return spriteset_vector[index];
    }
    return NULL;
}

bool ObjectSpritesets::exists(int id) const
{
    size_t index = static_cast<size_t>(id);
    if (index < spriteset_vector.size() && spriteset_vector[index] != NULL) {
        return true;
    }
    return false;
}

bool ObjectSpritesets::exists(Objects::SpritesetId id) const
{
    size_t index = static_cast<size_t>(id);
    if (index < spriteset_vector.size() && spriteset_vector[index] != NULL) {
        return true;
    }
    return false;
}

SpriteTexture* ObjectSpritesets::operator[](int id) const
{
    size_t index = static_cast<size_t>(id);
    if (index < spriteset_vector.size()) {
        return spriteset_vector[index];
    }
    return NULL;
}

void ObjectSpritesets::setFontTexture(SpriteTexture* font_texture)
{
    this->fonts = font_texture;
}
