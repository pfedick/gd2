#include <stdio.h>
#include <ppl7.h>
#include "game.h"
#include "constants.h"

/*!\class GameViewport
 * \brief Spiel-Viewport Verwaltung
 *\header \#include "game.h"
 *\desc
 * Die Klasse GameViewport verwaltet den Viewport des Spiels, also den Bereich des
 * Fensters, in dem das Spiel gerendert wird. Dabei werden Größenanpassungen
 * und das Seitenverhältnis berücksichtigt.
 *
 * Ferner ermöglicht die Klasse die Übersetzung von Koordinaten zwischen
 * Fenster- und Spielwelt-Koordinatensystemen.
 *
 * Die Klasse erbt von ppl7::grafix::Rect und erweitert diese um spezifische
 * Funktionen für die Spielansicht. ppl7::grafix::Rect gibt die Größe der Offscreen-Renderfläche
 * an (z.B. 0,0,3840,2160)
 *
 * Die Default-Konstruktor initialisiert den Viewport mit einer Standardgröße
 * von 1920x1080 Pixeln und einem Seitenverhältnis von 16:9.
 *
 * Beispiel:
 * \code{.cpp}
 * GameViewport viewport;
 * viewport.setWindowSize(ppl7::grafix::Size(1920, 1080));
 * viewport.setAspectRatio(16.0f / 9.0f);
 * SDL_FRect renderRect;
 * viewport.getRenderRect(renderRect);
 * \endcode
 */

/*!\brief Konstruktor der Klasse
 *
 * \desc
 * Der Konstruktor der Klasse initialisiert den Viewport mit Standardwerten.
 */
GameViewport::GameViewport()
{
    aspect_ratio = (float)16 / (float)9;
    render_rect.x = 0;
    render_rect.y = 0;
    render_rect.w = 1920;
    render_rect.h = 1080;
    sprite_scale_factor = 1.0f;
    grid_size.x = TILE_WIDTH;
    grid_size.y = TILE_HEIGHT;
    // render_size = ppl7::grafix::Size(1920, 1080);
    render_size = ppl7::grafix::Size(3840, 2160);
    logical_size = ppl7::grafix::Size(3840, 2160);
}

/*!\brief Viewport aktualisieren
 *
 * \desc
 * Diese private Methode aktualisiert die internen Parameter des Viewports
 * basierend auf der aktuellen Fenstergröße, Rendergröße und dem Seitenverhältnis.
 */
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
    /*
    ppl7::PrintDebugTime("GameViewport::update: WindowSize=%d:%d, RenderRect=%0.1f:%0.1f,%0.1f:%0.1f\n", window_size.width,
                         window_size.height, render_rect.x, render_rect.y, render_rect.w, render_rect.h);
    */
}

/*!\brief Fenstergröße setzen
 *
 * \param size Neue Fenstergröße
 *
 * \desc
 * Diese Methode setzt die Größe des Fensters und aktualisiert den Viewport entsprechend.
 */
void GameViewport::setWindowSize(const ppl7::grafix::Size& size)
{
    window_size = size;
    update();
}

/*!\brief Rendergröße setzen
 *
 * \param size Neue Rendergröße
 *
 * \desc
 * Diese Methode setzt die Größe des Renderziels und aktualisiert den Viewport entsprechend.
 */
void GameViewport::setRenderSize(const ppl7::grafix::Size& size)
{
    render_size = size;
    sprite_scale_factor = (float)render_size.width / 3840.0f;
    grid_size.x = (float)(TILE_WIDTH)*sprite_scale_factor;
    grid_size.y = (float)(TILE_HEIGHT)*sprite_scale_factor;
}

const ppl7::grafix::Size& GameViewport::getRenderSize() const
{
    return render_size;
}

/*!\brief Seitenverhältnis setzen
 *
 * \param aspect_ratio Neues Seitenverhältnis (Breite/Höhe)
 *
 * \desc
 * Diese Methode setzt das Seitenverhältnis des Viewports und aktualisiert die internen Parameter.
 */
void GameViewport::setAspectRatio(float aspect_ratio)
{
    this->aspect_ratio = aspect_ratio;
    update();
}

void GameViewport::setViewport(const ppl7::grafix::Rect& rect)
{
    viewport = rect;
}

const ppl7::grafix::Rect& GameViewport::getViewport() const
{
    return viewport;
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
    event->p = translate(event->p);
}

ppl7::grafix::PointF GameViewport::translate(const ppl7::grafix::PointF& coords) const
{
    ppl7::grafix::PointF p;

    float scale_x = (float)logical_size.width / render_rect.w;
    float scale_y = (float)logical_size.height / render_rect.h;

    // ppl7::PrintDebug("scale_x=%0.3f, scale_y=%0.3f\n", scale_x, scale_y);

    p.x = (coords.x - render_rect.x) * scale_x;
    p.y = (coords.y - render_rect.y) * scale_y;

    // ppl7::PrintDebug("translate, renderrect: %0.1f:%0.1f, %0.1f:%0.1f, mouse: %01.f:%0.1f\n", render_rect.x, render_rect.y,
    // render_rect.w,
    //                  render_rect.h, p.x, p.y);

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

int GameViewport::width() const
{
    return logical_size.width;
}

int GameViewport::height() const
{
    return logical_size.height;
}

float GameViewport::scaledTileWidth() const
{
    return TILE_WIDTH * sprite_scale_factor;
}
float GameViewport::scaledTileHeight() const
{
    return TILE_HEIGHT * sprite_scale_factor;
}
float GameViewport::tileWidth() const
{
    return TILE_WIDTH;
}
float GameViewport::tileHeight() const
{
    return TILE_HEIGHT;
}

const ppl7::grafix::Size& GameViewport::getLogicalSize() const
{
    return logical_size;
}

void GameViewport::setLogicalSize(const ppl7::grafix::Size& size)
{
    logical_size = size;
}