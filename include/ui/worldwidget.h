#ifndef INCLUDE_UI_WORLDWIDGET_H_
#define INCLUDE_UI_WORLDWIDGET_H_

#include <ppl7.h>
#include <ppl7-grafix.h>
#include <ppltk.h>

class WorldWidget : public ppltk::Widget
{
private:
    bool show_ui;

public:
    WorldWidget();
    void setViewport(const ppl7::grafix::Rect& viewport);
    ppl7::String widgetType() const override;
    void paint(ppl7::grafix::Drawable& draw) override;
    void setShowUi(bool enable);
};

#endif