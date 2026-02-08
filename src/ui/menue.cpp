#include "game.h"
#include "ui/menue.h"
#include "player.h"

MainMenue::MainMenue(int x, int y, int width, int height, Game* game)
    : ppltk::Frame(x, y, width, height)
{
    this->game = game;
    setupUi();
    visibility = NULL;
    debug_submenu = NULL;
    level_dialog = NULL;
    for (int i = 0; i < static_cast<int>(ParallaxLayerId::MaxLayerId); i++) {
        layer_visibility[i] = true;
    }
    visibility_sprites = true;
    visibility_objects = true;
    visibility_particles = true;
    visibility_grid = false;
    visibility_tiletypes = false;
    visibility_collision = false;
    visibility_lighting = true;
    visibility_hud = true;
    visibility_blur = true;
    controlsEnabled = true;
}

void MainMenue::update()
{
    if (game->getLevelFilename().isEmpty())
        save_button->setEnabled(false);
    else
        save_button->setEnabled(true);
}

ppl7::String MainMenue::widgetType() const
{
    return "MainMenue";
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
    // ppl7::grafix::Grafix* gfx=ppl7::grafix::GetGrafix();
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

    ppltk::Label* label = new ppltk::Label(908, 0, 100, s.height, "edit Layer:");
    this->addChild(label);

    active_plane_combobox = new ppltk::ComboBox(1009, 0, 150, s.height);
    active_plane_combobox->add("Near", ppl7::ToString("%d", static_cast<int>(ParallaxLayerId::Near)));
    active_plane_combobox->add("Close", ppl7::ToString("%d", static_cast<int>(ParallaxLayerId::Close)));
    active_plane_combobox->add("Front", ppl7::ToString("%d", static_cast<int>(ParallaxLayerId::Front)));
    active_plane_combobox->add("Player", ppl7::ToString("%d", static_cast<int>(ParallaxLayerId::Player)));
    active_plane_combobox->add("Back", ppl7::ToString("%d", static_cast<int>(ParallaxLayerId::Back)));
    active_plane_combobox->add("Behind", ppl7::ToString("%d", static_cast<int>(ParallaxLayerId::Behind)));
    active_plane_combobox->add("Middle", ppl7::ToString("%d", static_cast<int>(ParallaxLayerId::Middle)));
    active_plane_combobox->add("Far", ppl7::ToString("%d", static_cast<int>(ParallaxLayerId::Far)));
    active_plane_combobox->add("Horizon", ppl7::ToString("%d", static_cast<int>(ParallaxLayerId::Horizon)));
    active_plane_combobox->add("Sky", ppl7::ToString("%d", static_cast<int>(ParallaxLayerId::Sky)));
    active_plane_combobox->setCurrentIdentifier(ppl7::ToString("%d", static_cast<int>(ParallaxLayerId::Player)));
    this->addChild(active_plane_combobox);

    show_metrics_submenu_button = new ppltk::Button(1169, 0, 70, s.height, "Metrics");
    show_metrics_submenu_button->setEventHandler(this);
    this->addChild(show_metrics_submenu_button);

    debug_button = new ppltk::Button(1249, 0, 70, s.height, "Debug");
    // debug_button->setIcon(wm->Toolbar.getDrawable(65));
    debug_button->setEventHandler(this);
    this->addChild(debug_button);

    godmode_checkbox = new ppltk::CheckBox(width() - 520, 0, 100, s.height, "god mode", false);
    godmode_checkbox->setEventHandler(this);
    this->addChild(godmode_checkbox);

    soundtrack_checkbox = new ppltk::CheckBox(width() - 420, 0, 150, s.height, "play soundtrack", true);
    this->addChild(soundtrack_checkbox);

    world_follows_player_checkbox = new ppltk::CheckBox(width() - 280, 0, 180, s.height, "World follows player", true);
    this->addChild(world_follows_player_checkbox);

    update();
}

void MainMenue::setWorldFollowsPlayer(bool enable)
{
    if (world_follows_player_checkbox) world_follows_player_checkbox->setChecked(enable);
}

void MainMenue::setCurrentLayer(ParallaxLayerId index)
{
    if (active_plane_combobox) active_plane_combobox->setCurrentIdentifier(ppl7::ToString("%d", static_cast<int>(index)));
}

void MainMenue::setShowTileTypes(bool show)
{
    visibility_tiletypes = show;
    if (visibility) visibility->setShowTileTypes(show);
}

ParallaxLayerId MainMenue::currentLayer() const
{
    if (active_plane_combobox) return static_cast<ParallaxLayerId>(active_plane_combobox->currentIdentifier().toInt());
    return ParallaxLayerId::Player;
}

bool MainMenue::worldFollowsPlayer() const
{
    if (world_follows_player_checkbox) return world_follows_player_checkbox->checked();
    return true;
}

void MainMenue::setGodMode(bool enabled)
{
    godmode_checkbox->setChecked(enabled);
    game->getPlayer()->setGodMode(enabled);
}

void MainMenue::openLevelDialog(bool new_flag)
{
    controlsEnabled = game->getControlsEnabled();
    if (level_dialog) delete level_dialog;
    game->enableControls(false);
    int w = 800, h = 640;
    level_dialog = new LevelDialog(w, h);
    level_dialog->setNewLevelFlag(new_flag);
    level_dialog->setGame(game);
    level_dialog->setEventHandler(this);
    if (!new_flag) level_dialog->loadValues(game->level.params);
    GetGameWindow()->addChild(level_dialog);
}

void MainMenue::toggledEvent(ppltk::Event* event, bool checked)
{
    if (event->widget() == godmode_checkbox) {
        game->getPlayer()->setGodMode(checked);
    }
}

void MainMenue::mouseClickEvent(ppltk::MouseEvent* event)
{
    if (event->widget() == exit_button) {
        game->quitEvent(NULL);
    }
}

void MainMenue::mouseDownEvent(ppltk::MouseEvent* event)
{
    if (event->widget() == edit_tiles_button) {
        game->editor.showTilesSelection();
    } else if (event->widget() == edit_tiletypes_button) {
        game->editor.showTileTypeSelection();
    } else if (event->widget() == edit_sprites_button) {
        game->editor.showSpriteSelection();
    } else if (event->widget() == edit_objects_button) {
        game->editor.showObjectSelection();
    } else if (event->widget() == edit_level_button) {
        openLevelDialog(false);
    } else if (event->widget() == save_button) {
        game->save(game->getLevelFilename());
    } else if (event->widget() == save_as_button) {
        game->openSaveAsDialog();
    } else if (event->widget() == new_button) {
        openLevelDialog(true);
    } else if (event->widget() == load_button) {
        game->openLoadDialog();
    } else if (event->widget() == show_visibility_submenu_button) {
        if (visibility) {
            delete visibility;
            visibility = NULL;
        } else {
            ppltk::Widget* top = show_visibility_submenu_button->getTopmostParent();
            ppl7::grafix::Point p = show_visibility_submenu_button->absolutePosition();
            visibility = new VisibilitySubMenu(p.x, height(), this);

            visibility->setTopmost(true);
            visibility->setEventHandler(this);

            top->addChild(visibility);
            ppltk::GetWindowManager()->setMouseFocus(visibility);
        }
    } else if (event->widget() == debug_button) {
        if (debug_submenu) {
            delete debug_submenu;
            debug_submenu = NULL;
        } else {
            ppltk::Widget* top = debug_button->getTopmostParent();
            ppl7::grafix::Point p = debug_button->absolutePosition();
            debug_submenu = new DebugSubMenu(p.x, height(), this);

            debug_submenu->setTopmost(true);
            debug_submenu->setEventHandler(this);
            top->addChild(debug_submenu);
            ppltk::GetWindowManager()->setMouseFocus(debug_submenu);
        }
    }
}

void MainMenue::closeEvent(ppltk::Event* event)
{
    if (event->widget() == level_dialog) {
        if (level_dialog->state() == LevelDialog::DialogState::OK) {
            if (level_dialog->isNewLevel()) {
                LevelParameter new_params;
                level_dialog->saveValues(new_params);
                delete (level_dialog);
                level_dialog = NULL;
                game->createNewLevel(new_params);
                return;
            } else {
                level_dialog->saveValues(game->level.params);
                delete (level_dialog);
                level_dialog = NULL;
                game->enableControls(controlsEnabled);
                game->updateFromLevelParameters();
            }
        } else if (level_dialog->state() == LevelDialog::DialogState::Aborted) {
            delete (level_dialog);
            level_dialog = NULL;
            game->enableControls(controlsEnabled);
        }
    } else if (event->widget() == visibility) {
        visibility = NULL;
    } else if (event->widget() == debug_submenu) {
        debug_submenu = NULL;
    }
}

VisibilitySubMenu::VisibilitySubMenu(int x, int y, MainMenue* menue)
    : ppltk::Frame(x, y, 140, 410)
{
    this->menue = menue;
    int y1 = 0;
    this->addChild(new ppltk::Label(0, y1, 100, 20, "Misc:"));
    y1 += 20;

    lighting_checkbox = new ppltk::CheckBox(20, y1, 100, 20, "Lighting", menue->visibility_lighting);
    lighting_checkbox->setEventHandler(this);
    this->addChild(lighting_checkbox);
    y1 += 20;

    blur_checkbox = new ppltk::CheckBox(20, y1, 100, 20, "Blur", menue->visibility_blur);
    blur_checkbox->setEventHandler(this);
    this->addChild(blur_checkbox);
    y1 += 20;

    show_grid_checkbox = new ppltk::CheckBox(20, y1, 100, 20, "Grid", menue->visibility_grid);
    show_grid_checkbox->setEventHandler(this);
    this->addChild(show_grid_checkbox);
    y1 += 20;

    show_tiletypes_checkbox = new ppltk::CheckBox(20, y1, 100, 20, "Tiletypes", menue->visibility_tiletypes);
    show_tiletypes_checkbox->setEventHandler(this);
    this->addChild(show_tiletypes_checkbox);
    y1 += 20;

    show_collision_checkbox = new ppltk::CheckBox(20, y1, 100, 20, "Collision", menue->visibility_collision);
    show_collision_checkbox->setEventHandler(this);
    this->addChild(show_collision_checkbox);
    y1 += 20;

    show_sprites_checkbox = new ppltk::CheckBox(20, y1, 100, 20, "Sprites", menue->visibility_sprites);
    show_sprites_checkbox->setEventHandler(this);
    this->addChild(show_sprites_checkbox);
    y1 += 20;

    show_objects_checkbox = new ppltk::CheckBox(20, y1, 100, 20, "Objects", menue->visibility_objects);
    show_objects_checkbox->setEventHandler(this);
    this->addChild(show_objects_checkbox);
    y1 += 20;

    show_particles_checkbox = new ppltk::CheckBox(20, y1, 100, 20, "Particles", menue->visibility_particles);
    show_particles_checkbox->setEventHandler(this);
    this->addChild(show_particles_checkbox);
    y1 += 20;

    show_hud_checkbox = new ppltk::CheckBox(20, y1, 100, 20, "HUD", menue->visibility_hud);
    show_hud_checkbox->setEventHandler(this);
    this->addChild(show_hud_checkbox);
    y1 += 20;

    y1 += 20;
    this->addChild(new ppltk::Label(0, y1, 100, 20, "visible Planes:"));
    y1 += 20;

    addVisibilityCheckbox(y1, ParallaxLayerId::Near, "Near", menue->layer_visibility[static_cast<int>(ParallaxLayerId::Near)]);
    addVisibilityCheckbox(y1, ParallaxLayerId::Close, "Close", menue->layer_visibility[static_cast<int>(ParallaxLayerId::Close)]);
    addVisibilityCheckbox(y1, ParallaxLayerId::Front, "Front", menue->layer_visibility[static_cast<int>(ParallaxLayerId::Front)]);
    addVisibilityCheckbox(y1, ParallaxLayerId::Player, "Player", menue->layer_visibility[static_cast<int>(ParallaxLayerId::Player)]);
    addVisibilityCheckbox(y1, ParallaxLayerId::Back, "Back", menue->layer_visibility[static_cast<int>(ParallaxLayerId::Back)]);
    addVisibilityCheckbox(y1, ParallaxLayerId::Behind, "Behind", menue->layer_visibility[static_cast<int>(ParallaxLayerId::Behind)]);
    addVisibilityCheckbox(y1, ParallaxLayerId::Middle, "Middle", menue->layer_visibility[static_cast<int>(ParallaxLayerId::Middle)]);
    addVisibilityCheckbox(y1, ParallaxLayerId::Far, "Far", menue->layer_visibility[static_cast<int>(ParallaxLayerId::Far)]);
    addVisibilityCheckbox(y1, ParallaxLayerId::Horizon, "Horizon", menue->layer_visibility[static_cast<int>(ParallaxLayerId::Horizon)]);
    addVisibilityCheckbox(y1, ParallaxLayerId::Sky, "Sky", menue->layer_visibility[static_cast<int>(ParallaxLayerId::Sky)]);
}

void VisibilitySubMenu::addVisibilityCheckbox(int& y1, ParallaxLayerId layerId, const ppl7::String& label, bool initialState)
{
    visible_layer_checkbox[static_cast<int>(layerId)] = new ppltk::CheckBox(20, y1, 100, 20, label, initialState);
    visible_layer_checkbox[static_cast<int>(layerId)]->setEventHandler(this);
    this->addChild(visible_layer_checkbox[static_cast<int>(layerId)]);
    y1 += 20;
}

void VisibilitySubMenu::setShowTileTypes(bool show)
{
    show_tiletypes_checkbox->setChecked(show);
}

void VisibilitySubMenu::toggledEvent(ppltk::Event* event, bool checked)
{

    ppltk::Widget* widget = event->widget();
    if (widget == lighting_checkbox) {
        menue->visibility_lighting = checked;
    } else if (widget == blur_checkbox) {
        menue->visibility_blur = checked;
    } else if (widget == show_hud_checkbox) {
        menue->visibility_hud = checked;
    } else if (widget == show_grid_checkbox) {
        menue->visibility_grid = checked;
    } else if (widget == show_tiletypes_checkbox) {
        menue->visibility_tiletypes = checked;
    } else if (widget == show_collision_checkbox) {
        menue->visibility_collision = checked;
    } else if (widget == show_sprites_checkbox) {
        menue->visibility_sprites = checked;
    } else if (widget == show_objects_checkbox) {
        menue->visibility_objects = checked;
    } else if (widget == show_particles_checkbox) {
        menue->visibility_particles = checked;
    } else {
        for (int i = 0; i < static_cast<int>(ParallaxLayerId::MaxLayerId); i++) {
            if (widget == visible_layer_checkbox[i]) {
                menue->layer_visibility[i] = checked;
                break;
            }
        }
    }
}

void VisibilitySubMenu::lostFocusEvent(ppltk::FocusEvent* event)
{
    if (event->newWidget() != this && !event->newWidget()->isChildOf(this)) {
        // ppl7::PrintDebug("VisibilitySubMenu::lostFocusEvent\n");
        this->deleteLater();
        ppltk::Event e(ppltk::Event::Type::Close);
        e.setWidget(this);
        menue->closeEvent(&e);
    }
}

void VisibilitySubMenu::setShowHud(bool show)
{
    show_hud_checkbox->setChecked(show);
}

DebugSubMenu::DebugSubMenu(int x, int y, MainMenue* menue)
    : ppltk::Frame(x, y, 140, 300)
{
    this->menue = menue;
    ppltk::WindowManager* wm = ppltk::GetWindowManager();
    ppl7::grafix::Rect client = clientRect();
    int y1 = 0;
    godmode_checkbox = new ppltk::CheckBox(0, y1, client.width(), 29, "god mode", GetGame().getPlayer()->godModeEnabled());
    godmode_checkbox->setEventHandler(this);
    this->addChild(godmode_checkbox);
    y1 += 30;

    battery_button = new ppltk::Button(0, y1, client.width(), 29, "add battery");
    // battery_button->setIcon(wm->Toolbar.getDrawable(65));
    battery_button->setEventHandler(this);
    this->addChild(battery_button);
    y1 += 30;

    add_flashlight_button = new ppltk::Button(0, y1, client.width(), 29, "add flashlight");
    // battery_button->setIcon(wm->Toolbar.getDrawable(65));
    add_flashlight_button->setEventHandler(this);
    this->addChild(add_flashlight_button);
    y1 += 30;

    add_hammer_button = new ppltk::Button(0, y1, client.width(), 29, "add hammer");
    // battery_button->setIcon(wm->Toolbar.getDrawable(65));
    add_hammer_button->setEventHandler(this);
    this->addChild(add_hammer_button);
    y1 += 30;

    add_cheese_button = new ppltk::Button(0, y1, client.width(), 29, "add cheese");
    // battery_button->setIcon(wm->Toolbar.getDrawable(65));
    add_cheese_button->setEventHandler(this);
    this->addChild(add_cheese_button);
    y1 += 30;

    add_extralife_button = new ppltk::Button(0, y1, client.width(), 29, "add life");
    // battery_button->setIcon(wm->Toolbar.getDrawable(65));
    add_extralife_button->setEventHandler(this);
    this->addChild(add_extralife_button);
    y1 += 30;

    add_medikit_button = new ppltk::Button(0, y1, client.width(), 29, "add medikit");
    // battery_button->setIcon(wm->Toolbar.getDrawable(65));
    add_medikit_button->setEventHandler(this);
    this->addChild(add_medikit_button);
    y1 += 30;

    add_oxygen_button = new ppltk::Button(0, y1, client.width(), 29, "add oxygen");
    // battery_button->setIcon(wm->Toolbar.getDrawable(65));
    add_oxygen_button->setEventHandler(this);
    this->addChild(add_oxygen_button);
    y1 += 30;

    y1 += 10;
    pause_button = new ppltk::Button(0, y1, client.width(), 29, "Pause");
    pause_button->setIcon(wm->Toolbar.getDrawable(64));
    pause_button->setEventHandler(this);
    pause_button->setCheckable(true);
    this->addChild(pause_button);
    y1 += 30;

    step_button = new ppltk::Button(0, y1, client.width(), 29, "Step");
    step_button->setIcon(wm->Toolbar.getDrawable(65));
    step_button->setEventHandler(this);
    this->addChild(step_button);
    y1 += 30;
}

void DebugSubMenu::mouseClickEvent(ppltk::MouseEvent* event)
{
    if (event->widget() == battery_button) {
        // GetGame().getPlayer()->addPowerCell();
    } else if (event->widget() == pause_button) {
        // GetGame().pauseGame(pause_button->isChecked());
    } else if (event->widget() == step_button) {
        pause_button->setChecked(true);
        // GetGame().stepFrame();
    } else if (event->widget() == add_hammer_button) {
        // GetGame().getPlayer()->addSpecialObject(Decker::Objects::Type::Hammer);
    } else if (event->widget() == add_flashlight_button) {
        // GetGame().getPlayer()->addSpecialObject(Decker::Objects::Type::Flashlight);
    } else if (event->widget() == add_cheese_button) {
        // GetGame().getPlayer()->addSpecialObject(Decker::Objects::Type::Cheese);
    } else if (event->widget() == add_extralife_button) {
        // GetGame().getPlayer()->addLife(1);
    } else if (event->widget() == add_medikit_button) {
        // GetGame().getPlayer()->addHealth(100);
    } else if (event->widget() == add_oxygen_button) {
        // GetGame().getPlayer()->addAir(30.0f);
    }
}

void DebugSubMenu::toggledEvent(ppltk::Event* event, bool checked)
{
    if (event->widget() == godmode_checkbox) {
        menue->setGodMode(checked);
    }
}

void DebugSubMenu::lostFocusEvent(ppltk::FocusEvent* event)
{
    if (event->newWidget() != this && !event->newWidget()->isChildOf(this)) {
        // ppl7::PrintDebug("VisibilitySubMenu::lostFocusEvent\n");
        this->deleteLater();
        ppltk::Event e(ppltk::Event::Type::Close);
        e.setWidget(this);
        menue->closeEvent(&e);
    }
}
