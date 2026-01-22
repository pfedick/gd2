#ifndef INCLUDE_COLORPALETTE_H_
#define INCLUDE_COLORPALETTE_H_

#include <ppl7.h>
#include <ppl7-grafix.h>
#include <array>

class ColorPaletteItem
{
public:
    ppl7::grafix::Color color;
    ppl7::String name;
    int ldraw_material;

    ColorPaletteItem();
    ColorPaletteItem(const ppl7::grafix::Color& color, const ppl7::String& name, int ldraw_material);

    void set(const ppl7::grafix::Color& color, const ppl7::String& name, int ldraw_material);
};

class ColorPalette
{
private:
    std::array<ColorPaletteItem, 256> palette;
    ColorPaletteItem undefined;

public:
    ColorPalette();
    void setDefaults();
    void set(uint32_t index, const ppl7::grafix::Color& color, const ppl7::String& name = ppl7::String(), int ldraw_material = 0);
    void set(uint32_t index, const ColorPaletteItem& item);
    void setColor(uint32_t index, const ppl7::grafix::Color& color);
    void setName(uint32_t index, const ppl7::String& name);

    const ColorPaletteItem& get(uint32_t index) const;
    const ppl7::grafix::Color& getColor(uint32_t index) const;
    const std::array<ColorPaletteItem, 256>& getPalette() const;
    const ppl7::String& getName(uint32_t index) const;

    void save(ppl7::FileObject& file, unsigned char id) const;
    void load(const ppl7::ByteArrayPtr& ba);
};

const ColorPalette& GetGlobalColorPalette();
void SetGlobalColorPalette(const ColorPalette& palette);

#endif // INCLUDE_COLORPALETTE_H_
