#ifndef INCLUDE_SPRITESELECTION_H_
#define INCLUDE_SPRITESELECTION_H_

#include <ppl7.h>
#include <ppl7-grafix.h>
#include <ppltk.h>
#include "tileselection.h"

class Game;
class ColorSelectionFrame;

class SpriteSelection : public ppltk::Frame
{
private:
    Game* game;
    TilesFrame* tilesframe;
    ColorSelectionFrame* colorframe;
    ppltk::HorizontalSlider* z_axis;
    ppltk::ComboBox* tileset_combobox;
    ppltk::ComboBox* sprite_layer_combobox;
    ppltk::DoubleHorizontalSlider* scale_slider;
    ppltk::DoubleHorizontalSlider* rotation_slider;

    class SpriteSet
    {
    public:
        ppl7::String name;
        SpriteTexture* sprites = NULL;
        int dimensions = 1;
    };

    std::map<int, SpriteSet> spritesets;

    int tileset;

    bool notifies_enabled;

public:
    SpriteSelection(int x, int y, int width, int height, Game* game);

    // virtual void paint(Drawable &draw);
    void enableNotfies(bool enable);

    void setSelectedSprite(int nr);
    int selectedSprite() const;
    void setCurrentSpriteSet(int id);
    int currentSpriteSet() const;
    int spriteSetDimensions() const;
    void setSpriteScale(float factor);
    float spriteScale() const;
    void setSpriteRotation(float rotation);
    float spriteRotation() const;
    void setSpriteSet(int id, const ppl7::String& name, SpriteTexture* sprites, int dimensions = 1);
    int currentLayer() const;
    void setCurrentLayer(int layer);
    int colorIndex() const;
    void setColorIndex(int index);
    void setZAxis(int z);
    int zAxis() const;
    void valueChangedEvent(ppltk::Event* event, int value) override;
    void valueChangedEvent(ppltk::Event* event, int64_t value) override;
    void valueChangedEvent(ppltk::Event* event, double value) override;
    void toggledEvent(ppltk::Event* event, bool checked) override;
};

#endif