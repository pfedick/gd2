#ifndef INCLUDE_UI_WORLDWIDGET_H_
#define INCLUDE_UI_WORLDWIDGET_H_

#include <ppl7.h>
#include <ppl7-grafix.h>
#include <ppltk.h>

class Player;

class StatsFrame : public ppltk::Widget
{
private:
    ppl7::String labeltext;
    ppl7::String value;
    ppl7::grafix::Font font;

public:
    StatsFrame(int x, int y, int width, int height, const ppl7::String& label);
    void setLabel(const ppl7::String& label);
    void setValue(const ppl7::String& value);
    ppl7::String widgetType() const override;
    void paint(ppl7::grafix::Drawable& draw) override;
    void setFontSize(int size);
    const ppl7::String& label() const;
};

class OxygenFrame : public ppltk::Widget
{
    ppl7::String label;
    float seconds_total, seconds_left;
    ppl7::grafix::Font font;

public:
    OxygenFrame(int x, int y, int width, int height, const ppl7::String& label);
    void setValue(float seconds_total, float seconds_left);
    void setLabel(const ppl7::String& label);
    ppl7::String widgetType() const override;
    void paint(ppl7::grafix::Drawable& draw) override;
    void setFontSize(int size);
};

class WorldWidget : public ppltk::Widget
{
private:
    StatsFrame *stats_health, *stats_lifes, *stats_points;
    OxygenFrame* stats_oxygen;
    int value_health, value_lifes, value_points;
    double oxygen_cooldown;
    bool showui;

public:
    WorldWidget();
    void retranslateUi();
    void setViewport(const ppl7::grafix::Rect& viewport);
    ppl7::String widgetType() const override;
    void paint(ppl7::grafix::Drawable& draw) override;

    void updatePlayerStats(const Player* player);
    void resetPlayerStats(const Player* player);
    void setShowUi(bool enable);
};

#endif