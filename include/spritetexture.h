#ifndef SPRITETEXTURE_H
#define SPRITETEXTURE_H

#include <ppl7.h>
#include <ppl7-grafix.h>
#include "sdl3_if.h"

class SpriteTexture
{
public:
    class SpriteIndexItem
    {
    public:
        int id;
        int textureId;
        SDL_Texture* tex;
        const ppl7::grafix::Drawable* drawable;
        SDL_Rect r;
        ppl7::grafix::Point Pivot;
        ppl7::grafix::Point Offset;

        SpriteIndexItem()
        {
            id = 0;
            textureId = 0;
            tex = NULL;
            drawable = NULL;
        }
        SpriteIndexItem(const SpriteIndexItem& other)
            :r(other.r), Pivot(other.Pivot), Offset(other.Offset)
        {
            id = other.id;
            textureId = other.textureId;
            tex = other.tex;
            drawable = other.drawable;
        }
    };
private:
    std::map<int, SDL_Texture*> TextureMap;
    std::map<int, ppl7::grafix::Image> InMemoryTextureMap;
    std::map<int, SpriteIndexItem> SpriteList;

    SDL_Texture* current_outline_texture;
    int current_outline_sprite_id;

    bool bSDLBufferd;
    bool bMemoryBufferd;
    bool bOutlinesEnabled;
    bool bCollisionDetectionEnabled;
    SDL_BlendMode defaultBlendMode;

    void loadTexture(SDL3& sdl, ppl7::PFPChunk* chunk, const ppl7::grafix::Color& tint);
    void loadIndex(ppl7::PFPChunk* chunk);
    SDL_Texture* postGenerateOutlines(SDL_Renderer* renderer, int sprite_id);
    SDL_Texture* findTexture(int id) const;
    const ppl7::grafix::Drawable* findInMemoryTexture(int id) const;

public:
    SpriteTexture();
    ~SpriteTexture();
    void load(SDL3& sdl, const ppl7::String& filename, const ppl7::grafix::Color& tint = ppl7::grafix::Color());
    void load(SDL3& sdl, ppl7::FileObject& ff, const ppl7::grafix::Color& tint = ppl7::grafix::Color());
    void clear();
    void draw(ppl7::grafix::Drawable& target, int x, int y, int id) const;
    void draw(ppl7::grafix::Drawable& target, int x, int y, int id, const ppl7::grafix::Color& color_modulation) const;
    void draw(SDL_Renderer* renderer, int x, int y, int id) const;
    void draw(SDL_Renderer* renderer, int x, int y, int id, const ppl7::grafix::Color& color_modulation) const;
    void draw(SDL_Renderer* renderer, int x, int y, int id, const SDL_Color& color_modulation) const;
    void drawBoundingBox(SDL_Renderer* renderer, int x, int y, int id) const;
    void drawBoundingBoxWithAngle(SDL_Renderer* renderer, int x, int y, int id, float scale_x, float scale_y, float angle) const;
    void draw(SDL_Renderer* renderer, int id, const SDL_Rect& source, const SDL_Rect& target) const;
    void drawScaled(SDL_Renderer* renderer, int x, int y, int id, float scale_factor) const;
    void drawScaled(SDL_Renderer* renderer, int x, int y, int id, float scale_factor, const ppl7::grafix::Color& color_modulation) const;
    void drawScaledWithAngle(SDL_Renderer* renderer, int x, int y, int id, float scale_x, float scale_y, float angle, const ppl7::grafix::Color& color_modulation) const;
    void drawOutlines(SDL_Renderer* renderer, int x, int y, int id, float scale_factor);
    void drawOutlinesWithAngle(SDL_Renderer* renderer, int x, int y, int id, float scale_x, float scale_y, float angle);


    ppl7::grafix::Size spriteSize(int id, float scale_factor) const;
    ppl7::grafix::Rect spriteBoundary(int id, float scale_factor, int x, int y) const;
    ppl7::grafix::Rect spriteBoundary(int id, float scale_factor_x, float scale_factor_y, float rotation, int x, int y) const;
    const ppl7::grafix::Drawable getDrawable(int id) const;
    void enableMemoryBuffer(bool enabled);
    void enableSDLBuffer(bool enabled);
    void enableCollisionDetection(bool enabled);
    void enableOutlines(bool enabled);
    int numTextures() const;
    int numSprites() const;
    void setTextureBlendMode(SDL_BlendMode blendMode);
    SDL_BlendMode getTextureBlendMode() const;
    void setPivot(int id, int x, int y);

    SDL_Rect getSpriteSource(int id) const;
    const SpriteIndexItem* getSpriteIndex(int id) const;
    ppl7::grafix::Point spriteOffset(int id) const;
};

#endif