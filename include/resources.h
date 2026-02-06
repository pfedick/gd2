#ifndef INCLUDE_RESOURCES_H
#define INCLUDE_RESOURCES_H

#include "sprite.h"
#include "gpu.h"
#include "tiles.h"

class ResourceException : public ppl7::Exception
{
public:
    using ppl7::Exception::Exception;

    ResourceException(const char* msg, ...) noexcept
    {
        va_list args;
        va_start(args, msg);
        copyText(msg, args);
        va_end(args);
    }

    const char* what() const noexcept override
    {
        return "ResourceException";
    }
};

class ObjectSpritesets;

class TileResource
{
public:
    SpriteTexture Sprites;
    SpriteTexture SpritesUi;
    TileOccupation Occupation;
};

class Resources
{
private:
    void loadTiles(GPUContext& gpu);

public:
    enum class TileSets
    {
        Granit = 1,
        MaxTileSet,
    };
    SpriteTexture Cursor;
    SpriteTexture Hud;
    SpriteTexture Player;
    SpriteTexture TileTypes;
    TileResource Tiles[static_cast<int>(TileSets::MaxTileSet)];
    SpriteTexture Trees;
    SpriteTexture Font24;

    SpriteTexture Particles;
    SpriteTexture ObjectsUi;

    ObjectSpritesets* object_spritesets;

    std::list<ppl7::String> background_images;

    Resources();
    ~Resources();
    void load(GPUContext& gpu);
};

Resources& getResources();

#endif // INCLUDE_RESOURCES_H