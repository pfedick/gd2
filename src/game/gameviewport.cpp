#include <stdio.h>
#include <ppl7.h>
#include "game.h"
#include "constants.h"

GameViewport::GameViewport()
{
    menu_offset_x = 0;
    aspect_ratio = (float)16 / (float)9;
    render_rect.x = 0;
    render_rect.y = 0;
    render_rect.w = 1920;
    render_rect.h = 1080;
    setRect((int)render_rect.x, (int)render_rect.y, (int)render_rect.w, (int)render_rect.h);
    sprite_scale_factor = 1.0f;
    grid_size.x = TILE_WIDTH;
    grid_size.y = TILE_HEIGHT;
}

void GameViewport::update()
{
    int w = window_size.width;
    int h = window_size.height;

    render_rect.w = w;
    render_rect.h = (float)w / aspect_ratio;
    if (render_rect.h > h) {
        render_rect.h = h;
        render_rect.w = (float)render_rect.h * aspect_ratio;
    }

    if (render_rect.w < window_size.width) {
        render_rect.x = (window_size.width - render_rect.w) / 2;
    } else {
        render_rect.x = 0;
    }
    if (render_rect.h < window_size.height) {
        render_rect.y = (window_size.height - render_rect.h) / 2;
    } else {
        render_rect.y = 0;
    }
    sprite_scale_factor = (float)render_rect.w / 3840.0f;
    grid_size.x = (float)(TILE_WIDTH * 2) * sprite_scale_factor;
    grid_size.y = (float)(TILE_HEIGHT * 2) * sprite_scale_factor;

    setRect((int)render_rect.x, (int)render_rect.y, (int)render_rect.w, (int)render_rect.h);

    // if (menu_offset_x) render_rect.x+=(menu_offset_x / 2);
    // ppl7::PrintDebugTime("vp width=%d, height=%d\n", render_rect.w, render_rect.h);
}

void GameViewport::setWindowSize(const ppl7::grafix::Size& size)
{
    window_size = size;
    update();
}

void GameViewport::setAspectRatio(float aspect_ratio)
{
    this->aspect_ratio = aspect_ratio;
    update();
}

void GameViewport::setMenuOffset(int x)
{
    menu_offset_x = x;
    update();
}

const ppl7::grafix::Size& GameViewport::getWindowSize() const
{
    return window_size;
}

float GameViewport::getSpriteScaleFactor() const
{
    return sprite_scale_factor;
}

const ppl7::grafix::PointF& GameViewport::getGridSize() const
{
    return grid_size;
}

void GameViewport::translateMouseEvent(ppltk::MouseEvent* event)
{
    ppltk::MouseState mouse = ppltk::GetWindowManager()->getMouseState();
    mouse.p.x = mouse.p.x - render_rect.x;
    mouse.p.y = mouse.p.y - render_rect.y;
    /*
    ppl7::PrintDebugTime("translateMouseEvent, renderrect: %d:%d, %d:%d, mouse: %d:%d\n",
        render_rect.x, render_rect.y, render_rect.w, render_rect.h,
        mouse.p.x, mouse.p.y);
    */
    event->p = mouse.p;
}

ppl7::grafix::PointF GameViewport::translate(const ppl7::grafix::PointF& coords) const
{
    ppl7::grafix::PointF p;
    p.x = coords.x - render_rect.x;
    p.y = coords.y - render_rect.y;
    /*
    ppl7::PrintDebugTime("translate, renderrect: %d:%d, %d:%d, mouse: %d:%d\n",
        render_rect.x, render_rect.y, render_rect.w, render_rect.h,
        p.x, p.y);
    */
    return p;
}

void GameViewport::getRenderRect(SDL_FRect& rect) const
{
    rect.x = render_rect.x;
    rect.y = render_rect.y;
    rect.w = render_rect.w;
    rect.h = render_rect.h;
}

const SDL_FRect& GameViewport::getRenderRect() const
{
    return render_rect;
}
