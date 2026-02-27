#include "game.h"
#include "ui/menue.h"
#include "background.h"
#include "player.h"
#include "widgets.h"

const ppl7::String& Game::getLevelFilename() const
{
    return LevelFile;
}

void Game::save(const ppl7::String& filename)
{
    try {
        level.backup(filename);
        level.save(filename);
        LevelFile = filename;
    }
    catch (...) {
        ppl7::String levelname = ppl7::File::getFilename(filename);
        LevelFile = config.CustomLevelPath + "/" + levelname;
#ifdef WIN32
        LevelFile.replace("/", "\\");
        openSaveAsDialog();
        return;
#endif
    }
    if (editor.mainmenue) editor.mainmenue->update();
}

void Game::unloadLevel()
{
    // TODO: soundtrack.fadeout(2.0f);
    editor.closeAll();
    editor.selected_object = NULL;
    editor.history.clear();
    LevelFile.clear();
    level.clear();
    background.clear();
}

void Game::load()
{
    editor.closeAll();
    editor.selected_object = NULL;
    editor.history.clear();
    level.load(LevelFile);
    enableControls(true);
    if (editor.mainmenue) editor.mainmenue->update();
}

void Game::createNewLevel(const LevelParameter& params)
{
    editor.closeAll();
    editor.selected_object = NULL;
    editor.history.clear();
    enableControls(false);
    if (editor.mainmenue) editor.mainmenue->setWorldFollowsPlayer(false);
    WorldCamera.setPoint(0, 0);
    level.create(params.width, params.height);
    level.params = params;
    if (player) {
        player->move(500, 500);
        player->setVisible(false);
    }
    LevelFile.clear();
    enableControls(false);
    updateFromLevelParameters();
    if (editor.mainmenue) editor.mainmenue->update();
}

void Game::updateFromLevelParameters()
{
    if (player) {
        if (level.params.drainBattery)
            player->setBatteryDrainRate(level.params.batteryDrainRate);
        else
            player->setBatteryDrainRate(0.0f);
    }
    level.runtimeParams = level.params;
    level.runtimeParams.CurrentSong = level.params.InitialSong;
}

void Game::openSaveAsDialog()
{
    if (filedialog) delete filedialog;
    filedialog = NULL;
    int w = 800;
    int h = 600;
    if (w >= width() - 100) w = width() - 100;
    if (h >= height() - 100) h = height() - 100;

    filedialog = new FileDialog(this, w, h);
    filedialog->setFilename(LevelFile);
    if (!ppl7::File::exists("Makefile")) {
        // we check if the save file is inside the installation path. We won't be able to write here
        if (LevelFile.startsWith(ppl7::Dir::currentPath()) || LevelFile.startsWith("level/")) {
            filedialog->setFilename(config.CustomLevelPath + "/" + ppl7::File::getFilename(LevelFile));
        }
    }
    filedialog->setWindowTitle("save level");
    filedialog->custom_id = 1;
    this->addChild(filedialog);
}

void Game::openLoadDialog()
{
    if (filedialog) delete filedialog;
    filedialog = NULL;

    int w = 800;
    int h = 600;
    if (w >= width() - 100) w = width() - 100;
    if (h >= height() - 100) h = height() - 100;
    filedialog = new FileDialog(this, w, h, FileDialog::FileMode::ExistingFile);
    filedialog->setFilter("*.lvl");
    if (LevelFile.notEmpty())
        filedialog->setFilename(LevelFile);
    else {
        ppl7::String pwd = ppl7::Dir::currentPath() + "/level";
        if (ppl7::Dir::exists(pwd))
            filedialog->setDirectory(pwd);
        else
            filedialog->setDirectory(config.CustomLevelPath);
    }

    filedialog->setWindowTitle("load existing level");
    filedialog->custom_id = 2;
    this->addChild(filedialog);
}

void Game::checkFileDialog()
{
    if (!filedialog) return;
    if (filedialog->state() == FileDialog::DialogState::Open) return;
    if (filedialog->state() == FileDialog::DialogState::Aborted) {
        delete filedialog;
        filedialog = NULL;
        return;
    }
    if (filedialog->custom_id == 1) { // save level
        ppl7::String filename = filedialog->filename();
        save(filename);
        config.LastEditorLevel = filename;
        LevelFile = filename;
        // Are we in dev environment?
        if (!ppl7::File::exists("Makefile")) {
            config.CustomLevelPath = ppl7::File::getPath(filename);
        }
        config.save();

    } else if (filedialog->custom_id == 2) { // load level
        ppl7::String filename = filedialog->filename();
        startLevel(filename);
        config.LastEditorLevel = filename;
        config.save();
    }

    delete filedialog;
    filedialog = NULL;
}

void Game::openNewLevelDialog()
{
    showUi(true);
    editor.mainmenue->openLevelDialog(true);
    player->setVisible(false);
}

bool Game::nextLevel(const ppl7::String& filename)
/*!Jump to next level
 * @param filename Filename of level to load
 * @return Returns true, if level exists and will be loaded, returns false, when level is not loadable
 */
{
    nextLevelFile.clear();
    if (filename == "MENU") {
        nextLevelFile = "MENU";
        // TODO: gameState = GameState::LevelEndTriggerd;
        // TODO: fade_to_black = 0;
        enableControls(false);
        return true;
    }
    if (ppl7::File::exists(filename)) {
        nextLevelFile = filename;
    } else if (ppl7::File::exists("level/" + filename)) {
        nextLevelFile = "level/" + filename;
    }
    if (nextLevelFile.isEmpty()) return false;
    enableControls(false);
    // TODO: gameState = GameState::LevelEndTriggerd;
    // TODO: fade_to_black = 0;
    // printf("wir sollten hierhin gehen: %s\n", (const char*)nextLevelFile);

    return true;
}

void Game::startLevel(const ppl7::String& filename)
{
    if (!ppl7::File::exists(filename)) {
        return;
    }
    editor.closeAll();
    editor.history.clear();
    level.load(filename);
    background.clear();
    LevelFile = filename;
    editor.mainmenue->update();
    ppl7::grafix::Point startpoint = level.layer(ParallaxLayerId::Player).objects.findPlayerStart();
    editor.mainmenue->setWorldFollowsPlayer(true);
    player->setParallaxLayer(ParallaxLayerId::Player, 1.0f);

    if (startpoint.x > 0) {
        player->move(startpoint.x, startpoint.y);
        player->setSavePoint(startpoint);
        player->setVisible(true);
        ppl7::grafix::Size halfsize = WorldCamera.getLogicalRenderSize() / 2;

        WorldCamera.setPoint(startpoint - ppl7::grafix::Point(halfsize.width, halfsize.height));
        enableControls(true);

    } else {
        player->setVisible(false);
        enableControls(false);
    }
    player->resetLevelObjects();
    player->setPetrified(false);
    player->enableControl();
    if (level.params.drainBattery)
        player->setBatteryDrainRate(level.params.batteryDrainRate);
    else
        player->setBatteryDrainRate(0.0f);

    // TODO: hud->resetPlayerStats(player);

    for (auto it = level.params.InitialItems.begin(); it != level.params.InitialItems.end(); ++it) {
        // if ((*it) == Objects::Type::PowerCell)
        //     player->addPowerCell();
        // else
        player->addSpecialObject((*it));
    }
    if (level.params.flashlightOnOnLevelStart) player->enableFlashlight(true);

    player->stand();
    // soundtrack.playInitialSong();
    background.setBackgroundType(level.params.backgroundType);
    background.setColor(level.params.BackgroundColor);
    background.setImage(level.params.BackgroundImage);
    background.setLevelDimension(level.getOccupiedAreaFromTileTypePlane(ParallaxLayerId::Player));
    // gameState = GameState::Running;
    if (filename == "level/start.lvl") {
        player->setAutoWalk(true);
        enableControls(false);

    } else {
        player->setAutoWalk(false);
    }
}
