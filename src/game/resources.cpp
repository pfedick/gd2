#include <stdio.h>
#include <stdlib.h>
#include <SDL3/SDL.h>
#include "game.h"



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

}

void Resources::load(GPUContext& gpu)
{
	try {
		Cursor.load(gpu, "res/ui/cursor.tex");
		Player.load(gpu, "res/player.tex");
	}
	catch (const ppl7::Exception& exp) {
		exp.print();
		throw ResourceException("Couldn't load resources: %s", (const char*)exp.text());
	}
}