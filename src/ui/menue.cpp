#include "game.h"
#include "ui/menue.h"


MainMenue::MainMenue(int x, int y, int width, int height, Game* game)
	: ppltk::Frame(x, y, width, height)
{
	this->game = game;
	setupUi();
}

void MainMenue::resize(int x, int y, int width, int height)
{
	this->setPos(x, y);
	this->setSize(width, height);
	this->destroyChilds();
	setupUi();
	needsRedraw();
}

void MainMenue::setupUi()
{
	int x = 0;
	ppl7::grafix::Size s = this->clientSize();
	//ppl7::grafix::Grafix* gfx=ppl7::grafix::GetGrafix();
	ppltk::WindowManager* wm = ppltk::GetWindowManager();
	exit_button = new ppltk::Button(s.width - 100, 0, 100, s.height, "Exit");
	exit_button->setIcon(wm->Toolbar.getDrawable(68));
	exit_button->setEventHandler(this);
	this->addChild(exit_button);

	save_button = new ppltk::Button(0, 0, 64, s.height, "Save");
	save_button->setIcon(wm->Toolbar.getDrawable(33));
	save_button->setEventHandler(this);
	this->addChild(save_button);
	x += 65;

	save_as_button = new ppltk::Button(x, 0, 100, s.height, "Save as...");
	save_as_button->setIcon(wm->Toolbar.getDrawable(67));
	save_as_button->setEventHandler(this);
	this->addChild(save_as_button);
	x += 101;

	load_button = new ppltk::Button(x, 0, 64, s.height, "Load");
	load_button->setIcon(wm->Toolbar.getDrawable(32));
	load_button->setEventHandler(this);
	this->addChild(load_button);
	x += 65;

	new_button = new ppltk::Button(231, 0, 64, s.height, "New");
	new_button->setIcon(wm->Toolbar.getDrawable(31));
	new_button->setEventHandler(this);
	this->addChild(new_button);

	x = 320;
	edit_level_button = new ppltk::Button(320, 0, 70, s.height, "Level");
	edit_level_button->setIcon(wm->Toolbar.getDrawable(73));
	edit_level_button->setEventHandler(this);
	this->addChild(edit_level_button);

	edit_tiles_button = new ppltk::Button(391, 0, 50, s.height, "Tiles");
	edit_tiles_button->setEventHandler(this);
	this->addChild(edit_tiles_button);

	edit_tiletypes_button = new ppltk::Button(442, 0, 80, s.height, "TileTypes");
	edit_tiletypes_button->setEventHandler(this);
	this->addChild(edit_tiletypes_button);

	edit_sprites_button = new ppltk::Button(523, 0, 70, s.height, "Sprites");
	edit_sprites_button->setEventHandler(this);
	this->addChild(edit_sprites_button);

	edit_objects_button = new ppltk::Button(594, 0, 70, s.height, "Objects");
	edit_objects_button->setEventHandler(this);
	this->addChild(edit_objects_button);

	edit_lights_button = new ppltk::Button(665, 0, 70, s.height, "Lights");
	edit_lights_button->setEventHandler(this);
	this->addChild(edit_lights_button);



	edit_waynet_button = new ppltk::Button(736, 0, 70, s.height, "WayNet");
	edit_waynet_button->setEventHandler(this);
	this->addChild(edit_waynet_button);


	show_visibility_submenu_button = new ppltk::Button(827, 0, 80, s.height, "Visibility");
	show_visibility_submenu_button->setEventHandler(this);
	this->addChild(show_visibility_submenu_button);


	ppltk::Label* label = new ppltk::Label(908, 0, 100, s.height, "active Plane: ");
	this->addChild(label);

	active_plane_combobox = new ppltk::ComboBox(1009, 0, 150, s.height);
	active_plane_combobox->add("PlayerPlane", "0");
	active_plane_combobox->add("FrontPlane", "1");
	active_plane_combobox->add("FarPlane", "2");
	active_plane_combobox->add("BackPlane", "3");
	active_plane_combobox->add("MiddlePlane", "4");
	active_plane_combobox->add("HorizonPlane", "5");
	active_plane_combobox->add("NearPlane", "6");

	this->addChild(active_plane_combobox);

	show_metrics_submenu_button = new ppltk::Button(1169, 0, 70, s.height, "Metrics");
	show_metrics_submenu_button->setEventHandler(this);
	this->addChild(show_metrics_submenu_button);

	debug_button = new ppltk::Button(1249, 0, 70, s.height, "Debug");
	//debug_button->setIcon(wm->Toolbar.getDrawable(65));
	debug_button->setEventHandler(this);
	this->addChild(debug_button);


	godmode_checkbox = new ppltk::CheckBox(width() - 520, 0, 100, s.height, "god mode", false);
	godmode_checkbox->setEventHandler(this);
	this->addChild(godmode_checkbox);

	soundtrack_checkbox = new ppltk::CheckBox(width() - 420, 0, 150, s.height, "play soundtrack", true);
	this->addChild(soundtrack_checkbox);

	world_follows_player_checkbox = new ppltk::CheckBox(width() - 280, 0, 180, s.height, "World follows player", true);
	this->addChild(world_follows_player_checkbox);

	//update();
}


void MainMenue::toggledEvent(ppltk::Event* event, bool checked)
{
	if (event->widget() == godmode_checkbox) {
		//game->getPlayer()->setGodMode(checked);
	}
}

void MainMenue::mouseClickEvent(ppltk::MouseEvent* event)
{
	if (event->widget() == exit_button) {
		game->quitEvent(NULL);
	}
}


void MainMenue::closeEvent(ppltk::Event* event)
{

}
