#ifndef INCLUDE_UI_STATUSBAR_H_
#define INCLUDE_UI_STATUSBAR_H_

#include <ppl7.h>
#include <ppl7-grafix.h>
#include <ppltk.h>

class StatusBar : public ppltk::Frame
{
private:
    ppltk::Label* fps_label;
    ppltk::Label* frametime_label;
    ppltk::Label* load_label;
    ppltk::Label* mouse_coords;
    ppltk::Label* world_coords;
    ppltk::Label* player_coords;
    ppltk::Label* mouse_buttons;
    ppltk::Label* time_label;
    ppltk::Label* version_label;
    ppltk::Label* player_state;
    ppltk::Label* object_id;

    int timer_id;

    void setupUi();

public:
    StatusBar(int x, int y, int width, int height);
    ~StatusBar();
    virtual ppl7::String widgetType() const;

    void resize(int x, int y, int width, int height);

    void setFps(int fps);
    void setLoad(float load);
    void setFrameTime(float time);
    void setMouse(const ppltk::MouseState& mouse);
    void setWorldCoords(const ppl7::grafix::Point& p);
    void setPlayerCoords(const ppl7::grafix::Point& p);
    void setPlayerState(const ppl7::String& state);
    void setSelectedObject(int id);

    // Events
    void timerEvent(ppltk::Event* event);
};

#endif /* INCLUDE_UI_STATUSBAR_H_ */