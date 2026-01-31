#ifndef INCLUDE_UI_LEVELDIALOG_H_
#define INCLUDE_UI_LEVELDIALOG_H_

#include <ppl7.h>
#include <ppl7-grafix.h>
#include <ppltk.h>
#include "widgets.h"

class LevelDialog : public Dialog
{
public:
    enum class DialogState
    {
        Open = 0,
        OK,
        Aborted
    };

private:
    ppltk::TabWidget* tabwidget;

    // ppltk::LineInput* level_name;
    ppltk::SpinBox* level_width;
    ppltk::SpinBox* level_height;
    ppltk::Label* level_pixel_size;
    ppltk::TabWidget* tstrings;
    std::map<ppl7::String, ppltk::LineInput*> LevelName;
    std::map<ppl7::String, ppltk::TextEdit*> Description;
    ppltk::SpinBox* LevelSort;
    ppltk::CheckBox* part_of_story;
    ppltk::CheckBox* level_is_listed;
    ppltk::Label* thumbnail;
    ppltk::Button* thumb_take_screenshot;
    ppltk::Button* thumb_to_clipboard;
    ppltk::Button* thumb_from_clipboard;
    ppltk::Button* thumb_load;
    ppltk::Button* thumb_save;
    ppltk::TextEdit* author;
    ppl7::ByteArray compressed_screenshot;
    int screenshot_timer_id;

    ppltk::RadioButton* radio_image;
    ppltk::RadioButton* radio_color;
    ppltk::ComboBox* background_image;

    // ppltk::Button* image_fileselect;

    ppltk::SpinBox* color_red;
    ppltk::SpinBox* color_green;
    ppltk::SpinBox* color_blue;
    ppltk::HorizontalSlider* slider_red;
    ppltk::HorizontalSlider* slider_green;
    ppltk::HorizontalSlider* slider_blue;
    ppltk::Frame* color_preview;
    // ppltk::Button* color_picker;

    ppltk::ComboBox* base_soundtrack;

    ppltk::ComboBox* additional_soundtrack;
    ppltk::Button* add_soundtrack_button;
    ppltk::Button* delete_soundtrack_button;
    ppltk::ListWidget* soundtrack_list;
    ppltk::CheckBox* soundtrack_random;

    // Items & Options
    ppltk::CheckBox* option_drain_battery;
    ppltk::CheckBox* option_flashlite_on_on_level_start;
    ppltk::DoubleHorizontalSlider* battery_drain_rate;
    ppltk::ComboBox* available_items;
    ppltk::Button* add_item_button;
    ppltk::Button* delete_item_button;
    ppltk::ListWidget* initial_items_list;
    ppltk::Label* battery_empty_time;

    ppltk::Button* ok_button;
    ppltk::Button* cancel_button;

    bool newlevel;
    DialogState my_state;

    std::map<ppl7::String, ppl7::String> song_map_identifier;
    Game* game;
    ppl7::grafix::Color previous_background;

    void updateColorPreview();
    void setupUi();
    void setupLevelTab();
    void setupBackgroundTab();
    void setupSoundtrackTab();
    void setupItemsAndOptionsTab();

public:
    int custom_id;
    LevelDialog(int width, int height);
    ~LevelDialog();

    void setGame(Game* game);
    void setNewLevelFlag(bool newlevel);
    bool isNewLevel() const;
    DialogState state() const;

    void loadValues(const LevelParameter& params);
    void saveValues(LevelParameter& params) const;

    ppl7::String widgetType() const override;
    void mouseClickEvent(ppltk::MouseEvent* event) override;
    void mouseDownEvent(ppltk::MouseEvent* event) override;
    void valueChangedEvent(ppltk::Event* event, int64_t value) override;
    void valueChangedEvent(ppltk::Event* event, double value) override;
    void keyDownEvent(ppltk::KeyEvent* event) override;
    void textChangedEvent(ppltk::Event* event, const ppl7::String& text) override;
    void closeEvent(ppltk::Event* event);
};

#endif /* INCLUDE_UI_LEVELDIALOG_H_ */