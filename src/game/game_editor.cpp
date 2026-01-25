#include "game.h"
#include "ui/tileselection.h"
#include "ui/statusbar.h"
#include "ui/menue.h"
#include "ui/worldwidget.h"

GameEditor::History::History()
{
    clear();
}

void GameEditor::History::clear()
{
    lastTileset = 2;
    lastTile = 0;
    lastTileColor = 2;
    lastTileLayer = 0;
}

GameEditor::GameEditor()
{
    tiles_selection = NULL;
    mainmenue = NULL;
    statusbar = NULL;
    game = NULL;
}

GameEditor::~GameEditor()
{
    closeAll();
}

void GameEditor::init(Game& game)
{
    this->game = &game;
    mainmenue = new MainMenue(0, 0, 1920, 32, &game);
    game.addChild(mainmenue);

    statusbar = new StatusBar(0, 1080 - 30, 1920, 30);
    game.addChild(statusbar);
}

void GameEditor::closeAll()
{
    if (tiles_selection) {
        delete tiles_selection;
        tiles_selection = NULL;
    }
    game->viewport.x1 = 0;
    game->world_widget->setViewport(game->viewport);
}

void GameEditor::showTilesSelection()
{
    if (tiles_selection) {
        closeAll();
        return;
    }
    closeAll();
    tiles_selection = new TilesSelection(0, 32, 300, statusbar->y() - 32, game);
    tiles_selection->setTileSet(1, "Concrete", &game->resources.TilesUi);
    tiles_selection->setCurrentTileSet(history.lastTileset);
    tiles_selection->setSelectedTile(history.lastTile);
    tiles_selection->setColorIndex(history.lastTileColor);
    tiles_selection->setLayer(history.lastTileLayer);
    game->addChild(tiles_selection);

    game->viewport.x1 = 300;
    // game_viewport.setMenuOffset(300);
    game->world_widget->setViewport(game->viewport);
    // hud->setViewport(viewport);
}

void GameEditor::showTileTypeSelection()
{
    closeAll();
}

void GameEditor::showSpriteSelection()
{
    closeAll();
}
