#ifndef INCLUDE_UI_MENUE_H_
#define INCLUDE_UI_MENUE_H_

#include <ppl7.h>
#include <ppl7-grafix.h>
#include <ppltk.h>

class Game;

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

	//ppltk::Button* pause_button;
	//ppltk::Button* step_button;
	ppltk::Button* debug_button;


	ppltk::ComboBox* active_plane_combobox;

	ppltk::CheckBox* world_follows_player_checkbox;
	ppltk::CheckBox* soundtrack_checkbox;
	ppltk::CheckBox* godmode_checkbox;

	void setupUi();




public:
	MainMenue(int x, int y, int width, int height, Game* game);

	void resize(int x, int y, int width, int height);

	void mouseClickEvent(ppltk::MouseEvent* event) override;
	//void textChangedEvent(ppltk::Event* event, const ppl7::String& text) override;
	void closeEvent(ppltk::Event* event) override;
	void toggledEvent(ppltk::Event* event, bool checked) override;
};

#endif /* INCLUDE_UI_MENUE_H_ */
