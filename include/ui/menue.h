#ifndef INCLUDE_UI_MENUE_H_
#define INCLUDE_UI_MENUE_H_

#include <ppl7.h>
#include <ppl7-grafix.h>
#include <ppltk.h>
#include "ui/leveldialog.h"
#include "level.h"

class Game;
class MainMenue;
enum class ParallaxLayerId;

class VisibilitySubMenu : public ppltk::Frame
{
private:
    MainMenue* menue;
    ppltk::CheckBox* lighting_checkbox;
    ppltk::CheckBox* blur_checkbox;
    ppltk::CheckBox* show_grid_checkbox;
    ppltk::CheckBox* show_tiletypes_checkbox;
    ppltk::CheckBox* show_collision_checkbox;
    ppltk::CheckBox* show_sprites_checkbox;
    ppltk::CheckBox* show_objects_checkbox;
    ppltk::CheckBox* show_particles_checkbox;
    ppltk::CheckBox* show_hud_checkbox;
    ppltk::CheckBox* visible_layer_checkbox[static_cast<int>(ParallaxLayerId::MaxLayerId)];

    void addVisibilityCheckbox(int& y1, ParallaxLayerId layerId, const ppl7::String& label, bool initialState);

public:
    VisibilitySubMenu(int x, int y, MainMenue* menue);
    void setShowTileTypes(bool show);
    void setShowHud(bool show);
    void toggledEvent(ppltk::Event* event, bool checked) override;
    void lostFocusEvent(ppltk::FocusEvent* event) override;
};

class DebugSubMenu : public ppltk::Frame
{
private:
    MainMenue* menue;
    ppltk::CheckBox* godmode_checkbox;
    ppltk::Button* pause_button;
    ppltk::Button* step_button;
    ppltk::Button* battery_button;
    ppltk::Button* add_hammer_button;
    ppltk::Button* add_flashlight_button;
    ppltk::Button* add_cheese_button;
    ppltk::Button* add_extralife_button;
    ppltk::Button* add_medikit_button;
    ppltk::Button* add_oxygen_button;

public:
    DebugSubMenu(int x, int y, MainMenue* menue);
    void mouseClickEvent(ppltk::MouseEvent* event) override;
    void lostFocusEvent(ppltk::FocusEvent* event) override;
    void toggledEvent(ppltk::Event* event, bool checked) override;
};

class MetricsSubMenu : public ppltk::Frame
{
private:
    MainMenue* menue;
    Metrics metrics;
    ppl7::grafix::Font font;
    void drawMillisecondMetric(ppl7::grafix::Drawable& draw, int c1, int c2, int y, const ppl7::String& text, double value);
    void drawDoubleMetric(ppl7::grafix::Drawable& draw, int c1, int c2, int y, const ppl7::String& text, double value);
    void drawIntMetric(ppl7::grafix::Drawable& draw, int c1, int c2, int y, const ppl7::String& text, uint64_t value);
    void drawCountMetric(
        ppl7::grafix::Drawable& draw, int c1, int c2, int c3, int y, const ppl7::String& text, uint64_t value1, uint64_t value2);

public:
    MetricsSubMenu(int x, int y, MainMenue* menue);
    void update(const Metrics& metrics);

    virtual void paint(ppl7::grafix::Drawable& draw);
};

class MainMenue : public ppltk::Frame
{
private:
    Game* game;
    ppltk::Button* exit_button;
    ppltk::Button* edit_tiles_button;

    ppltk::Button* save_button;
    ppltk::Button* save_as_button;
    ppltk::Button* load_button;
    ppltk::Button* new_button;

    ppltk::Button* edit_tiletypes_button;
    ppltk::Button* edit_sprites_button;
    ppltk::Button* edit_objects_button;
    ppltk::Button* edit_waynet_button;
    ppltk::Button* edit_lights_button;
    ppltk::Button* edit_level_button;
    ppltk::Button* show_visibility_submenu_button;
    ppltk::Button* show_metrics_submenu_button;

    // ppltk::Button* pause_button;
    // ppltk::Button* step_button;
    ppltk::Button* debug_button;

    ppltk::ComboBox* active_plane_combobox;

    ppltk::CheckBox* world_follows_player_checkbox;
    ppltk::CheckBox* soundtrack_checkbox;
    ppltk::CheckBox* godmode_checkbox;

    VisibilitySubMenu* visibility;
    DebugSubMenu* debug_submenu;
    MetricsSubMenu* metrics;

    LevelDialog* level_dialog;
    bool controlsEnabled;

    void setupUi();

public:
    bool layer_visibility[static_cast<int>(ParallaxLayerId::MaxLayerId)];
    bool visibility_sprites;
    bool visibility_objects;
    bool visibility_particles;
    bool visibility_grid;
    bool visibility_tiletypes;
    bool visibility_collision;
    bool visibility_lighting;
    bool visibility_blur;
    bool visibility_hud;

    MainMenue(int x, int y, int width, int height, Game* game);
    void update();
    ppl7::String widgetType() const;

    void resize(int x, int y, int width, int height);

    void mouseDownEvent(ppltk::MouseEvent* event) override;
    void mouseClickEvent(ppltk::MouseEvent* event) override;
    // void textChangedEvent(ppltk::Event* event, const ppl7::String& text) override;
    void closeEvent(ppltk::Event* event) override;
    void toggledEvent(ppltk::Event* event, bool checked) override;

    void setWorldFollowsPlayer(bool enable);
    void setCurrentLayer(ParallaxLayerId index);
    ParallaxLayerId currentLayer() const;
    bool worldFollowsPlayer() const;
    void setGodMode(bool enable);
    void setShowTileTypes(bool show);
    void openLevelDialog(bool new_flag);

    void updateMetrics(const Metrics& last_metrics);
    void fitMetrics(const ppl7::grafix::Rect& viewport);
    void showMetrics();
};

#endif /* INCLUDE_UI_MENUE_H_ */
