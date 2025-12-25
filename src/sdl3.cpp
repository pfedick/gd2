#include <stdio.h>
#include <stdlib.h>
#include "sdl3_if.h"

static ppl7::grafix::RGBFormat SDL2RGBFormat(const Uint32 f)
{
	switch (f) {
	case SDL_PIXELFORMAT_INDEX8: return ppl7::grafix::RGBFormat::Palette;
	case SDL_PIXELFORMAT_RGB332: return ppl7::grafix::RGBFormat::R3G3B2;
	case SDL_PIXELFORMAT_XRGB4444: return ppl7::grafix::RGBFormat::X4R4G4B4;
	case SDL_PIXELFORMAT_XRGB1555: return ppl7::grafix::RGBFormat::X1R5G5B5;
	case SDL_PIXELFORMAT_XBGR1555: return ppl7::grafix::RGBFormat::X1B5G5R5;
	case SDL_PIXELFORMAT_ARGB4444: return ppl7::grafix::RGBFormat::A4R4G4B4;
	case SDL_PIXELFORMAT_ABGR4444: return ppl7::grafix::RGBFormat::A4B4G4R4;
	case SDL_PIXELFORMAT_ARGB1555: return ppl7::grafix::RGBFormat::A1R5G5B5;
	case SDL_PIXELFORMAT_ABGR1555: return ppl7::grafix::RGBFormat::A1B5G5R5;
	case SDL_PIXELFORMAT_RGB565: return ppl7::grafix::RGBFormat::R5G6B5;
	case SDL_PIXELFORMAT_BGR565: return ppl7::grafix::RGBFormat::B5G6R5;
	case SDL_PIXELFORMAT_RGB24: return ppl7::grafix::RGBFormat::R8G8B8;
	case SDL_PIXELFORMAT_BGR24: return ppl7::grafix::RGBFormat::B8G8R8;
	case SDL_PIXELFORMAT_XRGB8888: return ppl7::grafix::RGBFormat::X8R8G8B8;
	case SDL_PIXELFORMAT_XBGR8888: return ppl7::grafix::RGBFormat::X8B8G8R8;
	case SDL_PIXELFORMAT_ARGB8888: return ppl7::grafix::RGBFormat::A8R8G8B8;
	case SDL_PIXELFORMAT_ABGR8888: return ppl7::grafix::RGBFormat::A8B8G8R8;
	}
	throw ppl7::grafix::UnsupportedColorFormatException("format=%d", f);
}

static SDL_PixelFormat RGBFormat2SDLFormat(const ppl7::grafix::RGBFormat& format)
{
	switch (format) {
	case ppl7::grafix::RGBFormat::Palette:
		return SDL_PIXELFORMAT_INDEX8;
	case ppl7::grafix::RGBFormat::A8R8G8B8:
		return SDL_PIXELFORMAT_ARGB8888;
	case ppl7::grafix::RGBFormat::X8R8G8B8:
		return SDL_PIXELFORMAT_ARGB8888;
	case ppl7::grafix::RGBFormat::A8B8G8R8:
		return SDL_PIXELFORMAT_ABGR8888;
	case ppl7::grafix::RGBFormat::X8B8G8R8:
		return SDL_PIXELFORMAT_ABGR8888;
	default:
		throw ppl7::grafix::UnsupportedColorFormatException();
	}
	throw ppl7::grafix::UnsupportedColorFormatException();
}


SDL3::VideoDisplay::VideoDisplay(int id, const ppl7::String& name)
{
	this->id = id;
	this->name = name;
}



SDL3::SDL3()
{
	/*
	if (0!=SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO)) {
		throw InitializationFailed("SDL");
	}
	*/
	//window=NULL;
	renderer = NULL;
	screensaver_enabled = SDL_ScreenSaverEnabled();
	if (screensaver_enabled) {
		SDL_DisableScreenSaver();
	}
}


SDL3::~SDL3()
{
	//destroyWindow();
	if (screensaver_enabled) {
		SDL_EnableScreenSaver();
	}
	//SDL_Quit();
}


SDL_Texture* SDL3::createStreamingTexture(int width, int height)
{
	SDL_Texture* texture;
	texture = SDL_CreateTexture(renderer,
		SDL_PIXELFORMAT_ARGB8888,
		SDL_TEXTUREACCESS_STREAMING,
		width,
		height);
	if (!texture) {
		ppl7::String err(SDL_GetError());
		throw SDLException("Couldn't create texture: " + err);
	}
	SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
	return texture;
}

SDL_Texture* SDL3::createStreamingTexture(const ppl7::String& filename)
{
	ppl7::grafix::Image img;
	img.load(filename);
	SDL_Texture* tex = createStreamingTexture(img.width(), img.height());
	ppl7::grafix::Drawable draw = lockTexture(tex);
	draw.blt(img);
	unlockTexture(tex);
	return tex;
}

SDL_Texture* SDL3::createRenderTargetTexture(int width, int height)
{
	SDL_Texture* texture;
	texture = SDL_CreateTexture(renderer,
		SDL_PIXELFORMAT_ARGB8888,
		SDL_TEXTUREACCESS_TARGET,
		width,
		height);
	if (!texture) {
		ppl7::String err(SDL_GetError());
		throw SDLException("Couldn't create texture: " + err);
	}
	SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
	return texture;
}

SDL_Texture* SDL3::createTexture(SDL_Renderer* renderer, const ppl7::grafix::Drawable& d)
{
	SDL_Surface* surface = SDL_CreateSurface(
		d.width(),
		d.height(),
		RGBFormat2SDLFormat(d.rgbformat()));
	SDL_SetSurfaceBlendMode(surface, SDL_BLENDMODE_BLEND);
	SDL_LockSurface(surface);
	ppl7::grafix::Drawable s(surface->pixels, surface->pitch,
		surface->w, surface->h, SDL2RGBFormat(surface->format));
	s.blt(d);
	//s.line(0,0,1024,1024,0xffffffff);
	//s.drawRect(0,0,1024,1024,0xffffffff);
	SDL_UnlockSurface(surface);
	SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surface);
	if (!tex) {
		ppl7::String err(SDL_GetError());
		SDL_DestroySurface(surface);
		throw SDLException("Couldn't create texture: " + err);
	}
	SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);
	SDL_DestroySurface(surface);
	return tex;
}

SDL_Texture* SDL3::createTexture(const ppl7::grafix::Drawable& d)
{
	return SDL3::createTexture(renderer, d);
}

void SDL3::destroyTexture(SDL_Texture* texture)
{
	if (texture) SDL_DestroyTexture(texture);
}

ppl7::grafix::Size SDL3::getDisplaySize(int display_no) const
{
	SDL_Rect desktop;
	SDL_GetDisplayBounds(display_no, &desktop);
	return ppl7::grafix::Size(desktop.w, desktop.h);
}

ppl7::grafix::Rect SDL3::getDisplayWindow(int display_no) const
{
	SDL_Rect desktop;
	SDL_GetDisplayBounds(display_no, &desktop);
	return ppl7::grafix::Rect(desktop.x, desktop.y, desktop.w, desktop.h);
}

void SDL3::startFrame(const ppl7::grafix::Color& background)
{
	SDL_SetRenderDrawColor(renderer, background.red(), background.green(), background.blue(), 255);
	SDL_RenderClear(renderer);
}

void SDL3::setRenderer(SDL_Renderer* r)
{
	renderer = r;
}

SDL_Renderer* SDL3::getRenderer()
{
	return renderer;
}

void SDL3::present()
{
	SDL_RenderPresent(renderer);
}

ppl7::grafix::Drawable SDL3::lockTexture(SDL_Texture* texture)
{
	void* pixels;
	int pitch;
	if (SDL_LockTexture(texture, NULL, &pixels, &pitch)) {
		throw SDLException((const char*)"Couldn't lock texture: %s", SDL_GetError());
	}
	return ppl7::grafix::Drawable(pixels, pitch, texture->w, texture->h,
		SDL2RGBFormat(texture->format));
}

void SDL3::unlockTexture(SDL_Texture* texture)
{
	SDL_UnlockTexture(texture);
}

ppl7::grafix::Size SDL3::getTextureSize(SDL_Texture* texture)
{
	return ppl7::grafix::Size(texture->w, texture->h);
}


SDL3::DisplayMode SDL3::desktopDisplayMode(int display_id)
{
	const SDL_DisplayMode* mode = SDL_GetDesktopDisplayMode(display_id);
	if (mode) {
		return SDL3::DisplayMode(SDL2RGBFormat(mode->format),
			mode->w, mode->h, mode->refresh_rate);
	}
	ppl7::String err(SDL_GetError());
	throw SDLException("Couldn't detrmine display mode for display %d: %s", display_id, (const char*)err);
}

void SDL3::getVideoDisplays(std::list<VideoDisplay>& display_list)
{
	int num_displays;
	SDL_DisplayID* displays = SDL_GetDisplays(&num_displays);


	display_list.clear();
	for (int i = 0;i < num_displays;i++) {
		display_list.push_back(SDL3::VideoDisplay(i, ppl7::String(SDL_GetDisplayName(displays[i]))));
	}
	SDL_free(displays);

}

void SDL3::getDisplayModes(int display_id, std::list<DisplayMode>& mode_list)
{
	mode_list.clear();
	int num_displays;
	SDL_DisplayID* displays = SDL_GetDisplays(&num_displays);
	if (display_id < num_displays) {
		int count;
		SDL_DisplayMode** modes = SDL_GetFullscreenDisplayModes(displays[display_id], &count);
		if (modes) {
			for (int i = 0; i < count; i++) {
				mode_list.push_back(SDL3::DisplayMode(SDL2RGBFormat(modes[i]->format),
					modes[i]->w, modes[i]->h, modes[i]->refresh_rate));
			}
			SDL_free(modes);
		}
	}
	SDL_free(displays);
}
