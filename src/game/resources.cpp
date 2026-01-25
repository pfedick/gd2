#include <stdio.h>
#include <stdlib.h>
#include "resources.h"
#include "objectsystem.h"

static Resources* resources = NULL;

Resources& getResources()
{
    if (!resources) throw ResourceException("Resources not initialized");
    return *resources;
}

Resources::Resources()
{
    resources = this;
    background_images.push_back(ppl7::String("res/backgrounds/sky2.png"));
    background_images.push_back(ppl7::String("res/backgrounds/Cloudy_sky1.jpg"));
    background_images.push_back(ppl7::String("res/backgrounds/sunset-sky-1455125487HWs.jpg"));
    background_images.push_back(ppl7::String("res/backgrounds/IMG_20220726_125250.jpg"));
    background_images.push_back(ppl7::String("res/backgrounds/IMG_20220726_125308.jpg"));
    background_images.push_back(ppl7::String("res/backgrounds/night1.jpg"));
    object_spritesets = new ObjectSpritesets();
}

Resources::~Resources()
{
    delete object_spritesets;
}

void Resources::load(GPUContext& gpu)
{
    try {
        Cursor.load(gpu, "res/ui/cursor.tex", SpriteBuffer::Memory);
        Hud.load(gpu, "res/ui/hud.tex", SpriteBuffer::Memory);

        // TODO TileTypes resource hinzufügen
        Tiles.load(gpu, "res/tiles.tex", SpriteBuffer::GPU | SpriteBuffer::Memory);
        TilesUi.load(gpu, "res/ui/tiles.tex", SpriteBuffer::Memory);
        Player.load(gpu, "res/player.tex", SpriteBuffer::GPU | SpriteBuffer::Memory);
        Particles.load(gpu, "res/particles.tex", SpriteBuffer::GPU);
        Font24.load(gpu, "res/fonts/scp_24.tex", SpriteBuffer::GPU);
        object_spritesets->setFontTexture(&Font24);
        object_spritesets->loadAll(gpu);
    }
    catch (const ppl7::Exception& exp) {
        exp.print();
        throw ResourceException("Couldn't load resources: %s", (const char*)exp.text());
    }
}