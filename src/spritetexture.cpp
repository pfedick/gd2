#include <stdio.h>
#include <stdlib.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include "gpu.h"
#include "sprite.h"

using namespace ppl7;

SpriteTexture::SpriteTexture()
{
    bMemoryBufferd = false;
    bOutlinesEnabled = false;
    bCollisionDetectionEnabled = false;
    bHasNormals = true;
    bHasSpeculars = true;
    bGPUBufferd = true;
    defaultBlendMode = SDL_BLENDMODE_BLEND;
    current_outline_texture = NULL;
    current_outline_sprite_id = -1;
    gpu = NULL;
}

SpriteTexture::SpriteTexture(GPUContext& gpu, const ppl7::String& filename, SpriteBuffer buffer)
    : SpriteTexture()
{
    load(gpu, filename, buffer);
}

SpriteTexture::~SpriteTexture()
{
    clear();
}

void SpriteTexture::enableMemoryBuffer(bool enabled)
{
    bMemoryBufferd = enabled;
}

void SpriteTexture::enableGPUBuffer(bool enabled)
{
    bGPUBufferd = enabled;
}

void SpriteTexture::enableSDLBuffer(bool enabled)
{
    bGPUBufferd = enabled;
}

void SpriteTexture::enableCollisionDetection(bool enabled)
{
    bCollisionDetectionEnabled = enabled;
}

void SpriteTexture::enableOutlines(bool enabled)
{
    bOutlinesEnabled = enabled;
}

void SpriteTexture::clear()
{
    if (gpu) {
        std::map<int, SDL_GPUTexture*>::const_iterator it;
        for (it = TextureMap.begin(); it != TextureMap.end(); ++it) {
            gpu->destroyGPUTexture(it->second);
        }
        if (current_outline_texture) gpu->destroyGPUTexture(current_outline_texture);
    }
    current_outline_texture = NULL;
    current_outline_sprite_id = -1;
    TextureMap.clear();
    InMemoryTextureMap.clear();
    SpriteList.clear();
}

SDL_GPUTexture* SpriteTexture::findTexture(int id) const
{
    if (bGPUBufferd) {
        std::map<int, SDL_GPUTexture*>::const_iterator it;
        it = TextureMap.find(id);
        if (it != TextureMap.end()) return it->second;
    }
    return NULL;
}

const ppl7::grafix::Drawable* SpriteTexture::findInMemoryTexture(int id) const
{
    if (bMemoryBufferd) {
        std::map<int, ppl7::grafix::Image>::const_iterator it;
        it = InMemoryTextureMap.find(id);
        if (it != InMemoryTextureMap.end()) return &it->second;
    }
    return NULL;
}

void SpriteTexture::loadIndex(ppl7::PFPChunk* chunk)
{
    char* buffer = (char*)chunk->data();
    int num = Peek32(buffer); // Anzahl Einträge in der Tabelle
    // ppl7::PrintDebugTime("SpriteTexture::loadIndex: %d sprites\n", num);
    char* p = buffer + 4;
    SpriteIndexItem item;
    for (int i = 0; i < num; i++) {
        item.id = Peek32(p + 0);
        item.textureId = Peek16(p + 4);
        item.tex = findTexture(item.textureId);
        // item.outlines=findOutlines(item.textureId);
        item.drawable = findInMemoryTexture(item.textureId);
        item.r.x = Peek16(p + 6 + 0);
        item.r.y = Peek16(p + 6 + 2);
        item.r.w = Peek16(p + 6 + 4) + 1 - item.r.x;
        item.r.h = Peek16(p + 6 + 6) + 1 - item.r.y;

        const ppl7::grafix::Size& s = textureSize(item.textureId);
        if (s.width > 0 && s.height > 0) {
            item.uv.x = (float)item.r.x / (float)s.width;
            item.uv.y = (float)item.r.y / (float)s.height;
            item.uv.w = (float)item.r.w / (float)s.width;
            item.uv.h = (float)item.r.h / (float)s.height;
        } else {
            ppl7::PrintDebugTime("  WARNING: sprite %d has texture %d with size 0x0\n", item.id, item.textureId);
            item.uv.x = 0.0f;
            item.uv.y = 0.0f;
            item.uv.w = 0.0f;
            item.uv.h = 0.0f;
        }

        if (!item.tex) {
            ppl7::PrintDebugTime("  WARNING: sprite %d has NULL texture (textureId=%d)\n", item.id, item.textureId);
        }

        item.Pivot.x = Peek16(p + 14 + 0);
        item.Pivot.y = Peek16(p + 14 + 2);
        item.Offset.x = Peek16(p + 18 + 0);
        item.Offset.y = Peek16(p + 18 + 2);
        SpriteList.insert(std::pair<int, SpriteIndexItem>(item.id, item));
        // ppl7::PrintDebugTime("pivot x=%d, y=%d\n", item.Pivot.x, item.Pivot.y);
        p += 22;
    }
}

ppl7::grafix::Image SpriteTexture::loadTexture(PFPChunk* chunk)
{
    Compression Comp;
    Comp.usePrefix(Compression::Prefix_V2);
    char* buffer = (char*)chunk->data();

    // Zunächst lesen wir dem Header
    // int id = Peek16(buffer + 0);
    ppl7::grafix::RGBFormat rgbformat;
    switch (Peek8(buffer + 2)) {
    case 9:
        rgbformat = grafix::RGBFormat::A8R8G8B8;
        break;
    default:
        throw grafix::UnsupportedColorFormatException();
    }
    // int bitdepth=Peek8(buffer+3);
    int width = Peek16(buffer + 4);
    int height = Peek16(buffer + 6);

    // Nutzdaten dekomprimieren
    ByteArray uncompressed;
    Comp.uncompress(uncompressed, buffer + 8, chunk->size() - 8);
    buffer = (char*)uncompressed.ptr();

    // Nun erstellen wir ein neues Image
    ppl7::grafix::Image surface;
    surface.create(width, height, rgbformat);
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            ppl7::grafix::Color c(Peek8(buffer + 2), Peek8(buffer + 1), Peek8(buffer), Peek8(buffer + 3));
            surface.putPixel(x, y, c);
            buffer += 4;
        }
    }
    return surface;
}

static inline void putOutlinePixel(ppl7::grafix::Image& surface, int x, int y, ppl7::grafix::Color& c)
{
}

void SpriteTexture::load(GPUContext& gpu, const String& filename)
{
    File ff;
    ff.open(filename);
    load(gpu, ff);
}

void SpriteTexture::load(GPUContext& gpu, const String& filename, SpriteBuffer buffer)
{
    bMemoryBufferd = (static_cast<int>(buffer) & static_cast<int>(SpriteBuffer::Memory)) != 0;
    bGPUBufferd = (static_cast<int>(buffer) & static_cast<int>(SpriteBuffer::GPU)) != 0;
    File ff;
    ff.open(filename);
    load(gpu, ff);
}

/*
void SpriteTexture::load(GPUContext& gpu, const String& filename, bool memory_buffered, bool gpu_buffered)
{
    bMemoryBufferd = memory_buffered;
    bGPUBufferd = gpu_buffered;
    File ff;
    ff.open(filename);
    load(gpu, ff);
}
*/

void SpriteTexture::load(GPUContext& gpu, FileObject& ff)
{
    this->gpu = &gpu;
    PFPFile File;
    clear();
    File.load(ff);
    int major, minor;
    File.getVersion(&major, &minor);
    if (File.getID() != "TEXS" || major != 1 || minor != 0) throw grafix::InvalidSpriteException();
    // Texture Chunks laden
    // printf ("opened: %s\n",(const char*)ff.filename());
    PFPChunk* chunk;
    PFPFile::Iterator it;
    File.reset(it);
    while ((chunk = File.findNextChunk(it, "SURF"))) {
        // printf ("load SURF\n");
        int id = Peek16(chunk->data());
        ppl7::grafix::Image surface = loadTexture(chunk);
        // ppl7::PrintDebugTime("SpriteTexture: loaded SURF texture %d, size %dx%d\n", id, surface.width(), surface.height());
        TextureSizeMap.insert(std::pair<int, ppl7::grafix::Size>(id, ppl7::grafix::Size(surface.width(), surface.height())));
        if (bMemoryBufferd) {
            InMemoryTextureMap.insert(std::pair<int, ppl7::grafix::Image>(id, surface));
        }
        if (bGPUBufferd) {
            SDL_GPUTexture* tex = gpu.createGPUTexture(surface);
            if (!tex) {
                ppl7::PrintDebugTime("  ERROR: GPU texture creation failed\n");
            }
            TextureMap.insert(std::pair<int, SDL_GPUTexture*>(id, tex));
        }
    }
    if (bGPUBufferd) {
        File.reset(it);
        while ((chunk = File.findNextChunk(it, "NORM"))) {
            int id = Peek16(chunk->data());
            ppl7::grafix::Image surface = loadTexture(chunk);
            SDL_GPUTexture* tex = gpu.createGPUTexture(surface);
            NormalMap.insert(std::pair<int, SDL_GPUTexture*>(id, tex));
            bHasNormals = true;
        }
        File.reset(it);
        while ((chunk = File.findNextChunk(it, "SPEC"))) {
            int id = Peek16(chunk->data());
            ppl7::grafix::Image surface = loadTexture(chunk);
            SDL_GPUTexture* tex = gpu.createGPUTexture(surface);
            SpecularMap.insert(std::pair<int, SDL_GPUTexture*>(id, tex));
            bHasSpeculars = true;
        }
    }

    // Index Chunks laden
    File.reset(it);
    // printf ("DONE SURF\n");
    while ((chunk = File.findNextChunk(it, "INDX"))) {
        // printf ("load INDX\n");
        loadIndex(chunk);
    }
}

void SpriteTexture::draw(ppl7::grafix::Drawable& target, int x, int y, int id) const
{
    if (!bMemoryBufferd) return;
    std::map<int, SpriteIndexItem>::const_iterator it;
    it = SpriteList.find(id);
    if (it == SpriteList.end()) return;
    const SpriteIndexItem& item = it->second;
    if (!item.drawable) return;
    ppl7::grafix::Rect r(item.r.x, item.r.y, item.r.w, item.r.h);
    target.bltAlpha(*item.drawable, r, x + item.Offset.x - item.Pivot.x, y + item.Offset.y - item.Pivot.y);
}

void SpriteTexture::draw(ppl7::grafix::Drawable& target, int x, int y, int id, const ppl7::grafix::Color& color_modulation) const
{
    if (!bMemoryBufferd) return;
    std::map<int, SpriteIndexItem>::const_iterator it;
    it = SpriteList.find(id);
    if (it == SpriteList.end()) return;
    const SpriteIndexItem& item = it->second;
    if (!item.drawable) return;
    ppl7::grafix::Rect r(item.r.x, item.r.y, item.r.w, item.r.h);
    target.bltAlphaMod(*item.drawable, r, color_modulation, x + item.Offset.x - item.Pivot.x, y + item.Offset.y - item.Pivot.y);
}

const ppl7::grafix::Drawable SpriteTexture::getDrawable(int id) const
{
    ppl7::grafix::Drawable draw;
    if (!bMemoryBufferd) return draw;
    std::map<int, SpriteIndexItem>::const_iterator it;
    it = SpriteList.find(id);
    if (it == SpriteList.end()) return draw;
    const SpriteIndexItem& item = it->second;
    if (!item.drawable) return draw;
    ppl7::grafix::Rect r(item.r.x, item.r.y, item.r.w, item.r.h);
    draw = (*item.drawable).getDrawable(r);
    return draw;
}

SDL_FRect SpriteTexture::getSpriteSource(int id) const
{
    SDL_FRect r;
    r.x = 0;
    r.y = 0;
    r.w = 0;
    r.h = 0;
    std::map<int, SpriteIndexItem>::const_iterator it;
    it = SpriteList.find(id);
    if (it == SpriteList.end()) return r;
    return (*it).second.uv;
}

void SpriteTexture::draw(GPUBatcher& gpu, int x, int y, int id) const
{
    if (!bGPUBufferd) return;
    gpu.addSprite(*this, id, (float)x, (float)y);
#ifdef OLD_SDL_RENDERER_API
    std::map<int, SpriteIndexItem>::const_iterator it;
    it = SpriteList.find(id);
    if (it == SpriteList.end()) return;
    const SpriteIndexItem& item = it->second;
    SDL_FRect tr;
    tr.x = x + item.Offset.x - item.Pivot.x;
    tr.y = y + item.Offset.y - item.Pivot.y;
    tr.w = item.r.w;
    tr.h = item.r.h;

    SDL_SetTextureColorMod(item.tex, 255, 255, 255);
    SDL_SetTextureAlphaMod(item.tex, 255);
    SDL_RenderTexture(renderer, item.tex, &item.r, &tr);
#endif
}

void SpriteTexture::draw(GPUBatcher& gpu, int x, int y, int id, const ppl7::grafix::Color& color_modulation) const
{
    if (!bGPUBufferd) return;
#ifdef OLD_SDL_RENDERER_API
    std::map<int, SpriteIndexItem>::const_iterator it;
    it = SpriteList.find(id);
    if (it == SpriteList.end()) return;
    const SpriteIndexItem& item = it->second;
    SDL_FRect tr;
    tr.x = x + item.Offset.x - item.Pivot.x;
    tr.y = y + item.Offset.y - item.Pivot.y;
    tr.w = item.r.w;
    tr.h = item.r.h;
    SDL_SetTextureAlphaMod(item.tex, color_modulation.alpha());
    SDL_SetTextureColorMod(item.tex, color_modulation.red(), color_modulation.green(), color_modulation.blue());
    SDL_RenderTexture(renderer, item.tex, &item.r, &tr);
#endif
}

void SpriteTexture::drawBoundingBox(GPUBatcher& gpu, int x, int y, int id) const
{
    if (!bGPUBufferd) return;
#ifdef OLD_SDL_RENDERER_API
    std::map<int, SpriteIndexItem>::const_iterator it;
    it = SpriteList.find(id);
    if (it == SpriteList.end()) return;
    const SpriteIndexItem& item = it->second;
    SDL_FRect tr;
    tr.x = x + item.Offset.x - item.Pivot.x;
    tr.y = y + item.Offset.y - item.Pivot.y;
    tr.w = item.r.w;
    tr.h = item.r.h;
    SDL_RenderRect(renderer, &tr);
#endif
}

void SpriteTexture::drawBoundingBoxWithAngle(GPUBatcher& gpu, int x, int y, int id, float scale_x, float scale_y, float angle) const
{
    if (!bGPUBufferd) return;
    std::map<int, SpriteIndexItem>::const_iterator it;
    it = SpriteList.find(id);
    if (it == SpriteList.end()) return;
    // const SpriteIndexItem& item=it->second;
    ppl7::grafix::Rect rr = spriteBoundary(id, scale_x, scale_y, angle, x, y);

    SDL_FRect tr;
    tr.x = rr.x1;
    tr.y = rr.y1;
    tr.w = rr.width();
    tr.h = rr.height();
#ifdef OLD_SDL_RENDERER_API
    SDL_SetRenderDrawColor(renderer, 255, 0, 255, 255);
    SDL_RenderRect(renderer, &tr);
#endif
}

void SpriteTexture::draw(GPUBatcher& gpu, int id, const SDL_FRect& source, const SDL_FRect& target) const
{
    if (!bGPUBufferd) return;
    // gpu.addSprite(*this, id, target.x, target.y, target.w / source.w, target.h / source.h, 0.0f, ppl7::grafix::Color(255, 255, 255, 255),
    // source);
}

void SpriteTexture::drawScaled(GPUBatcher& gpu, int x, int y, int id, float scale_factor) const
{
    if (!bGPUBufferd) return;
    gpu.addSprite(*this, id, (float)x, (float)y, scale_factor, scale_factor);
}

void SpriteTexture::drawScaled(GPUBatcher& gpu, int x, int y, int id, float scale_factor, const ppl7::grafix::Color& color_modulation) const
{
    if (!bGPUBufferd) return;
    gpu.addSprite(*this, id, (float)x, (float)y, scale_factor, scale_factor, 0.0f, color_modulation);
}

void SpriteTexture::drawScaledWithAngle(
    GPUBatcher& gpu, int x, int y, int id, float scale_x, float scale_y, float angle, const ppl7::grafix::Color& color_modulation) const
{
    if (!bGPUBufferd) return;
    gpu.addSprite(*this, id, (float)x, (float)y, scale_x, scale_y, angle, color_modulation);
}

void SpriteTexture::drawOutlines(GPUBatcher& gpu, int x, int y, int id, float scale_factor)
{
    if (!bOutlinesEnabled) return;
    std::map<int, SpriteIndexItem>::const_iterator it;
    it = SpriteList.find(id);
    if (it == SpriteList.end()) return;
    const SpriteIndexItem& item = it->second;

    if (id != current_outline_sprite_id || current_outline_texture == NULL) {
        current_outline_texture = postGenerateOutlines(id);
        if (current_outline_texture)
            current_outline_sprite_id = id;
        else
            return;
    }

    SDL_FRect tr;
    // printf ("Sprite::drawScaled %0.1f\n", scale_factor);
    if (scale_factor == 1.0) {
        tr.x = x + item.Offset.x - item.Pivot.x;
        tr.y = y + item.Offset.y - item.Pivot.y;
        tr.w = item.r.w;
        tr.h = item.r.h;
    } else {
        tr.x = x + (item.Offset.x - item.Pivot.x) * scale_factor;
        tr.y = y + (item.Offset.y - item.Pivot.y) * scale_factor;
        tr.w = (int)((float)item.r.w * scale_factor);
        tr.h = (int)((float)item.r.h * scale_factor);
    }
#ifdef OLD_SDL_RENDERER_API
    SDL_RenderTexture(renderer, current_outline_texture, NULL, &tr);
#endif
}

void SpriteTexture::drawOutlinesWithAngle(GPUBatcher& gpu, int x, int y, int id, float scale_x, float scale_y, float angle)
{
    if (!bOutlinesEnabled) return;
    std::map<int, SpriteIndexItem>::const_iterator it;
    it = SpriteList.find(id);
    if (it == SpriteList.end()) return;
    const SpriteIndexItem& item = it->second;

    if (id != current_outline_sprite_id || current_outline_texture == NULL) {
        current_outline_texture = postGenerateOutlines(id);
        if (current_outline_texture)
            current_outline_sprite_id = id;
        else
            return;
    }
    SDL_FRect tr;
    tr.x = x + (item.Offset.x - item.Pivot.x) * scale_x;
    tr.y = y + (item.Offset.y - item.Pivot.y) * scale_y;
    tr.w = (int)((float)item.r.w * scale_x);
    tr.h = (int)((float)item.r.h * scale_y);
    SDL_FPoint center;
    center.x = (item.Pivot.x - item.Offset.x) * scale_x;
    center.y = (item.Pivot.y - item.Offset.y) * scale_y;
    // TODO
#ifdef OLD_SDL_RENDERER_API
    SDL_RenderTextureRotated(renderer, current_outline_texture, NULL, &tr, angle, &center, SDL_FLIP_NONE);
#endif
}

ppl7::grafix::Size SpriteTexture::spriteSize(int id, float scale_factor) const
{
    std::map<int, SpriteIndexItem>::const_iterator it;
    it = SpriteList.find(id);
    ppl7::grafix::Size s;
    if (it == SpriteList.end()) return s;
    const SpriteIndexItem& item = it->second;
    s.width = (int)((float)item.r.w * scale_factor);
    s.height = (int)((float)item.r.h * scale_factor);
    return s;
}

ppl7::grafix::Rect SpriteTexture::spriteBoundary(int id, float scale_factor, int x, int y) const
{
    std::map<int, SpriteIndexItem>::const_iterator it;
    it = SpriteList.find(id);
    ppl7::grafix::Rect r;
    if (it == SpriteList.end()) return r;
    const SpriteIndexItem& item = it->second;
    r.x1 = x + (item.Offset.x - item.Pivot.x) * scale_factor;
    r.y1 = y + (item.Offset.y - item.Pivot.y) * scale_factor;
    r.x2 = r.x1 + (int)((float)item.r.w * scale_factor);
    r.y2 = r.y1 + (int)((float)item.r.h * scale_factor);
    return r;
}

ppl7::grafix::Point SpriteTexture::spriteOffset(int id) const
{
    std::map<int, SpriteIndexItem>::const_iterator it;
    it = SpriteList.find(id);
    ppl7::grafix::Point p;
    if (it == SpriteList.end()) return p;
    const SpriteIndexItem& item = it->second;
    p.x = (item.Offset.x - item.Pivot.x);
    p.y = (item.Offset.y - item.Pivot.y);
    return p;
}

static inline ppl7::grafix::Point rotate_point(const ppl7::grafix::Point& p, const ppl7::grafix::Point& pivot, float s, float c)
{
    ppl7::grafix::Point pr = p;
    pr.x -= pivot.x;
    pr.y -= pivot.y;
    // rotate point
    float xnew = (float)pr.x * c - (float)pr.y * s;
    float ynew = (float)pr.x * s + (float)pr.y * c;
    pr.x = xnew + pivot.x;
    pr.y = ynew + pivot.y;
    return pr;
}

static inline int min_val(int v1, int v2, int v3, int v4)
{
    int v = v1;
    if (v2 < v) v = v2;
    if (v3 < v) v = v3;
    if (v4 < v) v = v4;
    return v;
}
static inline int max_val(int v1, int v2, int v3, int v4)
{
    int v = v1;
    if (v2 > v) v = v2;
    if (v3 > v) v = v3;
    if (v4 > v) v = v4;
    return v;
}

static ppl7::grafix::Rect rotate(const ppl7::grafix::Rect& r, const ppl7::grafix::Point& pivot, float angle)
{
    ppl7::grafix::Rect r2;
    float s = sin(angle * M_PI / 180.0f);
    float c = cos(angle * M_PI / 180.0f);

    ppl7::grafix::Point p1 = rotate_point(r.topLeft(), pivot, s, c);
    ppl7::grafix::Point p2 = rotate_point(r.topRight(), pivot, s, c);
    ppl7::grafix::Point p3 = rotate_point(r.bottomLeft(), pivot, s, c);
    ppl7::grafix::Point p4 = rotate_point(r.bottomRight(), pivot, s, c);
    r2.x1 = min_val(p1.x, p2.x, p3.x, p4.x);
    r2.y1 = min_val(p1.y, p2.y, p3.y, p4.y);
    r2.x2 = max_val(p1.x, p2.x, p3.x, p4.x);
    r2.y2 = max_val(p1.y, p2.y, p3.y, p4.y);
    return r2;
}

ppl7::grafix::Rect SpriteTexture::spriteBoundary(int id, float scale_factor_x, float scale_factor_y, float rotation, int x, int y) const
{
    std::map<int, SpriteIndexItem>::const_iterator it;
    it = SpriteList.find(id);
    ppl7::grafix::Rect r;
    if (it == SpriteList.end()) return r;
    const SpriteIndexItem& item = it->second;
    // TODO: add rotation into calculation
    r.x1 = x + (item.Offset.x - item.Pivot.x) * scale_factor_x;
    r.y1 = y + (item.Offset.y - item.Pivot.y) * scale_factor_y;
    r.x2 = r.x1 + (int)((float)item.r.w * scale_factor_x);
    r.y2 = r.y1 + (int)((float)item.r.h * scale_factor_y);
    ppl7::grafix::Rect r2 = rotate(r, ppl7::grafix::Point(x, y), rotation);
    return r2;
}

int SpriteTexture::numTextures() const
{
    return (int)TextureMap.size();
}

int SpriteTexture::numSprites() const
{
    return (int)SpriteList.size();
}

void SpriteTexture::setPivot(int id, int x, int y)
{
    std::map<int, SpriteIndexItem>::iterator it;
    it = SpriteList.find(id);
    if (it == SpriteList.end()) return;
    it->second.Pivot.setPoint(x, y);
}

ppl7::grafix::Point SpriteTexture::getPivot(int id) const
{
    std::map<int, SpriteIndexItem>::const_iterator it;
    it = SpriteList.find(id);
    ppl7::grafix::Point p;
    if (it == SpriteList.end()) return p;
    return it->second.Pivot;
}

const SpriteTexture::SpriteIndexItem* SpriteTexture::getSpriteIndex(int id) const
{
    std::map<int, SpriteIndexItem>::const_iterator it;
    it = SpriteList.find(id);
    if (it == SpriteList.end()) return NULL;
    return &it->second;
}

static inline void putOutlinePixel4x4(ppl7::grafix::Drawable& target, int x, int y, ppl7::grafix::Color& color)
{
    target.putPixel(x, y, color);
    target.putPixel(x + 1, y, color);
    target.putPixel(x, y + 1, color);
    target.putPixel(x + 1, y + 1, color);
}

static inline bool isBorder(const ppl7::grafix::Drawable& src, int x, int y)
{
    ppl7::grafix::Color c = src.getPixel(x, y);
    ppl7::grafix::Color cl = src.getPixel(x - 1, y);
    ppl7::grafix::Color cr = src.getPixel(x + 1, y);
    ppl7::grafix::Color cu = src.getPixel(x, y - 1);
    ppl7::grafix::Color cd = src.getPixel(x, y + 1);
    if (c.alpha() > 128 && (cl.alpha() <= 128 || cr.alpha() <= 128 || cu.alpha() <= 128 || cd.alpha() <= 128)) {
        return true;
    }
    return false;
}

static void generateOutlinesForSprite(const ppl7::grafix::Drawable& source, ppl7::grafix::Drawable& target)
{
    if (source.width() != target.width() || source.height() != target.height()) {
        target.cls(ppl7::grafix::Color(255, 0, 0, 255));
        ppl7::PrintDebugTime("   ERROR: SpriteTexture::generateOutlinesForSprite, invalid source or target: src: %d:%d, tgt: %d,%d\n",
                             source.width(), source.height(), target.width(), target.height());
        return;
    }
    ppl7::grafix::Color white(255, 255, 255, 255);
    // Outlines at border of sprite
    int y1 = 0;
    int y2 = source.height() - 1;
    int x1 = 0;
    int x2 = source.width() - 1;
    ppl7::grafix::Color c;
    for (int x = 0; x < source.width(); x++) {
        // top line
        c = source.getPixel(x, y1);
        if (c.alpha() > 128) putOutlinePixel4x4(target, x, y1, white);
        // bottom line
        c = source.getPixel(x, y2);
        if (c.alpha() > 128) putOutlinePixel4x4(target, x, y2 - 1, white);
    }
    for (int y = 0; y < source.height(); y++) {
        // left line
        c = source.getPixel(x1, y);
        if (c.alpha() > 128) putOutlinePixel4x4(target, x1, y, white);
        // right line
        c = source.getPixel(x2, y);
        if (c.alpha() > 128) putOutlinePixel4x4(target, x2 - 1, y, white);
    }

    // Sprite interior
    for (int y = 1; y < y2; y++) {
        for (int x = 1; x < x2; x++) {
            if (isBorder(source, x, y)) {
                putOutlinePixel4x4(target, x, y, white);
            }
        }
    }
}

SDL_GPUTexture* SpriteTexture::postGenerateOutlines(int sprite_id)
{
    if (!bMemoryBufferd || !bOutlinesEnabled) return NULL;
    // ppl7::PrintDebugTime("SpriteTexture::postGenerateOutlines\n");
    // double start=ppl7::GetMicrotime();
    if (current_outline_texture) gpu->destroyGPUTexture(current_outline_texture);
    current_outline_texture = NULL;
    current_outline_sprite_id = -1;
    std::map<int, SpriteIndexItem>::iterator it;
    it = SpriteList.find(sprite_id);
    if (it == SpriteList.end()) return NULL;
    const SpriteIndexItem& item = it->second;
    ppl7::grafix::Image target(item.r.w, item.r.h, ppl7::grafix::RGBFormat::A8R8G8B8);

    ppl7::grafix::Rect r(item.r.x, item.r.y, item.r.w, item.r.h);
    ppl7::grafix::Drawable source = item.drawable->getDrawable(r);
    generateOutlinesForSprite(source, target);
    SDL_GPUTexture* tex = gpu->createGPUTexture(target);
    // ppl7::PrintDebugTime("  ===> %0.6f s\n", ppl7::GetMicrotime() - start);
    return tex;
}

const ppl7::grafix::Size& SpriteTexture::textureSize(int id) const
{
    static ppl7::grafix::Size zero_size;
    std::map<int, ppl7::grafix::Size>::const_iterator it;
    it = TextureSizeMap.find(id);
    if (it == TextureSizeMap.end()) return zero_size;
    return it->second;
}
