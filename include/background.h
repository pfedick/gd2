#ifndef INCLUDE_BACKGROUND_H
#define INCLUDE_BACKGROUND_H
#include <ppl7.h>
#include <ppl7-grafix.h>
#include "gpu.h"

class Background
{
public:
    enum class Type
    {
        Image,
        Color
    };

private:
    GPUContext* gpu;
    SDL_Texture* tex_sky;
    ppl7::grafix::Size tex_size;
    ppl7::grafix::Color color;
    Type t;
    ppl7::String last_image;
    ppl7::grafix::Rect level_dimension;

    Type fade_target_type;
    ppl7::grafix::Color fade_target_color;
    ppl7::String fade_target_image_filename;
    SDL_Texture* fade_target_tex;
    ppl7::grafix::Size fade_tex_size;
    float fade_progress;

    void drawFade(SDL_Renderer* renderer, const ppl7::grafix::Rect& viewport, const ppl7::grafix::Point& WorldCoords);

public:
    Background();
    void init(GPUContext& gpu);
    void clear();
    void setImage(const ppl7::String& filename);
    void setColor(const ppl7::grafix::Color& color);
    void setBackgroundType(Type t);
    void setLevelDimension(const ppl7::grafix::Rect& tiles);
    void setFadeTargetColor(const ppl7::grafix::Color& color);
    void setFadeTargetImage(const ppl7::String& filename);
    void setFadeProgress(float progress);

    void draw(GPUBatcher& batcher, const ppl7::grafix::Rect& viewport, const ppl7::grafix::Point& WorldCoords);
};

#endif // INCLUDE_BACKGROUND_H