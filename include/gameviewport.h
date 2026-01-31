#ifndef INCLUDE_GAMEVIEWPORT_H_
#define INCLUDE_GAMEVIEWPORT_H_
#include <ppl7.h>
#include <ppl7-grafix.h>
#include <ppltk.h>

class GameViewport
{
private:
    int menu_offset_x;
    ppl7::grafix::Size window_size;
    ppl7::grafix::Size render_size;
    float aspect_ratio;
    SDL_FRect render_rect;
    float sprite_scale_factor;
    ppl7::grafix::PointF grid_size;

    void update();

public:
    GameViewport();
    void setWindowSize(const ppl7::grafix::Size& size);
    void setRenderSize(const ppl7::grafix::Size& size);
    void setAspectRatio(float aspect_ratio);
    ppl7::grafix::PointF translate(const ppl7::grafix::PointF& coords) const;
    ppltk::MouseState translate(const ppltk::MouseState& mouse) const;
    void translateMouseEvent(ppltk::MouseEvent* event);
    void getRenderRect(SDL_FRect& rect) const;
    const SDL_FRect& getRenderRect() const;
    const ppl7::grafix::Size& getWindowSize() const;
    const ppl7::grafix::Size& getRenderSize() const;
    float getSpriteScaleFactor() const;
    const ppl7::grafix::PointF& getGridSize() const;
    void setMenuOffset(int x);
    int getMenuOffset() const;
    int width() const;              // returns width of render area
    int height() const;             // returns height of render area
    float scaledTileWidth() const;  // returns tile width scaled according to sprite scale factor
    float scaledTileHeight() const; // returns tile height scaled according to sprite scale factor
    float tileWidth() const;        // returns tile width (e.g. 64)
    float tileHeight() const;       // returns tile height (e.g. 64)
};

#endif /* INCLUDE_GAMEVIEWPORT_H_ */