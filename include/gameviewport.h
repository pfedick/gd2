#ifndef INCLUDE_GAMEVIEWPORT_H_
#define INCLUDE_GAMEVIEWPORT_H_
#include <ppl7.h>
#include <ppl7-grafix.h>
#include <ppltk.h>

class GameViewport
{
private:
    ppl7::grafix::Size window_size;
    ppl7::grafix::Size render_size;
    ppl7::grafix::Size logical_size;
    float aspect_ratio;
    SDL_FRect render_rect;
    float sprite_scale_factor;
    ppl7::grafix::PointF grid_size;
    ppl7::grafix::Rect viewport;

    void update();

public:
    GameViewport();
    void setWindowSize(const ppl7::grafix::Size& size);  // Die tatsächliche Fenstergröße
    void setRenderSize(const ppl7::grafix::Size& size);  // Die Größe des Render-Targets (2k)
    void setLogicalSize(const ppl7::grafix::Size& size); // Die logische Größe, immer 4k
    void setAspectRatio(float aspect_ratio);             // Das gewünschte Seitenverhältnis (z.B. 16.0f/9.0f)
    void setViewport(const ppl7::grafix::Rect&
                         rect); // Setzt den Renderbereich im Fenster. Ist identisch zur Fenstergröße, wenn kein UI-Bereich vorhanden ist
    const ppl7::grafix::Rect& getViewport() const;
    ppl7::grafix::PointF translate(const ppl7::grafix::PointF& coords) const;
    ppltk::MouseState translate(const ppltk::MouseState& mouse) const;
    void translateMouseEvent(ppltk::MouseEvent* event);
    void getRenderRect(SDL_FRect& rect) const;
    const SDL_FRect& getRenderRect() const;
    const ppl7::grafix::Size& getWindowSize() const;
    const ppl7::grafix::Size& getRenderSize() const;
    const ppl7::grafix::Size& getLogicalSize() const;
    float getSpriteScaleFactor() const;
    const ppl7::grafix::PointF& getGridSize() const;
    int width() const;              // returns width of render area
    int height() const;             // returns height of render area
    float scaledTileWidth() const;  // returns tile width scaled according to sprite scale factor
    float scaledTileHeight() const; // returns tile height scaled according to sprite scale factor
    float tileWidth() const;        // returns tile width (e.g. 64)
    float tileHeight() const;       // returns tile height (e.g. 64)
};

#endif /* INCLUDE_GAMEVIEWPORT_H_ */