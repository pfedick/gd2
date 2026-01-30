#ifndef INCLUDE_GAMEVIEWPORT_H_
#define INCLUDE_GAMEVIEWPORT_H_
#include <ppl7.h>
#include <ppl7-grafix.h>
#include <ppltk.h>

class GameViewport : public ppl7::grafix::Rect
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
    void translateMouseEvent(ppltk::MouseEvent* event);
    void getRenderRect(SDL_FRect& rect) const;
    const SDL_FRect& getRenderRect() const;
    const ppl7::grafix::Size& getWindowSize() const;
    const ppl7::grafix::Size& getRenderSize() const;
    float getSpriteScaleFactor() const;
    const ppl7::grafix::PointF& getGridSize() const;
    void setMenuOffset(int x);
    int getMenuOffset() const;
};

#endif /* INCLUDE_GAMEVIEWPORT_H_ */