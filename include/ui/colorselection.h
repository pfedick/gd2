#ifndef INCLUDE_COLORSELECTIONFRAME_H_
#define INCLUDE_COLORSELECTIONFRAME_H_

#include <ppl7.h>
#include <ppl7-grafix.h>
#include <ppltk.h>
#include "colorpalette.h"

class ColorPaletteFrame : public ppltk::Frame
{
  private:
    ColorPalette &palette;
    int color_index;
    ppltk::Scrollbar *scrollbar;
    int tsize;
    int items_per_row;
    int rows;
    ppl7::grafix::Color color_clipboard;

  public:
    ColorPaletteFrame(int x, int y, int width, int height, ColorPalette &palette);
    ~ColorPaletteFrame();
    int colorIndex() const;
    void setColorIndex(int index);
    ppl7::grafix::Color color() const;

    void mouseDownEvent(ppltk::MouseEvent *event) override;
    void mouseWheelEvent(ppltk::MouseEvent *event) override;
    void valueChangedEvent(ppltk::Event *event, int value) override;
    void paint(ppl7::grafix::Drawable &draw) override;
};

class ColorSelectionFrame : public ppltk::Widget
{
  private:
    ColorPalette &palette;
    ColorPaletteFrame *color_palette;
    ppltk::LineInput *color_name;
    ppltk::HorizontalSlider *slider_red;
    ppltk::HorizontalSlider *slider_green;
    ppltk::HorizontalSlider *slider_blue;

    ppltk::SpinBox *color_red;
    ppltk::SpinBox *color_green;
    ppltk::SpinBox *color_blue;

    ppl7::grafix::Color color_clipboard;

    void sendEventValueChanged();

  public:
    ColorSelectionFrame(int x, int y, int width, int height, ColorPalette &palette);
    int colorIndex() const;
    void setColorIndex(int index);
    ppl7::grafix::Color color() const;

    void paint(ppl7::grafix::Drawable &draw) override;
    void textChangedEvent(ppltk::Event *event, const ppl7::String &text) override;
    void valueChangedEvent(ppltk::Event *event, int value) override;
    void valueChangedEvent(ppltk::Event *event, int64_t value) override;
    void keyDownEvent(ppltk::KeyEvent *event) override;
};

#endif