#include "game.h"
#include "ui/tileselection.h"
#include "ui/tiletypeselection.h"
#include "ui/statusbar.h"
#include "ui/menue.h"
#include "ui/worldwidget.h"
#include "constants.h"

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
    tiletype_selection = NULL;
    sprite_selection = NULL;
    object_selection = NULL;
    lights_selection = NULL;
    waynet_edit = NULL;
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
}

void GameEditor::closeAll()
{
    if (tiles_selection) {
        delete tiles_selection;
        tiles_selection = NULL;
    }
    if (tiletype_selection) {
        delete tiletype_selection;
        tiletype_selection = NULL;
    }
    game->viewport.x1 = 0;
    game->world_widget->setViewport(game->viewport);
    game->game_viewport.setViewport(game->viewport);
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
    game->world_widget->setViewport(game->viewport);
    game->game_viewport.setViewport(game->viewport);
    // hud->setViewport(viewport);
}

void GameEditor::showTileTypeSelection()
{
    if (tiletype_selection) {
        closeAll();
        return;
    }
    closeAll();
    tiletype_selection = new TileTypeSelection(0, 32, 300, statusbar->y() - 32, game, &game->resources.TileTypes);
    game->addChild(tiletype_selection);
    // viewport.x1 = 300;
    game->viewport.x1 = 300;
    game->world_widget->setViewport(game->viewport);
    game->game_viewport.setViewport(game->viewport);

    // hud->setViewport(viewport);
    mainmenue->setShowTileTypes(true);
    mainmenue->setCurrentLayer(ParallaxLayerId::Player);
}

void GameEditor::showSpriteSelection()
{
    closeAll();
}

void GameEditor::handleMouseDrawInWorld(const ppltk::MouseState& mouse)
{
    // ppl7::PrintDebug("GameEditor::handleMouseDrawInWorld\n");
    //   const bool* state = SDL_GetKeyboardState(NULL);
    //    if (state[SDL_SCANCODE_LSHIFT]) return;

    if (tiletype_selection) {

        ParallaxLayerId currentlayer = mainmenue->currentLayer();
        ParallaxLayer& layer = game->level.layer(currentlayer);
        ppl7::grafix::Point coords = game->WorldCoords * layer.size_factor * layer.speed_factor;
        int x = (mouse.p.x + coords.x) / TILE_WIDTH;
        int y = (mouse.p.y + coords.y) / TILE_HEIGHT;
        // ppl7::PrintDebug("TileTypeSelection Mouse Draw at %d:%d\n", x, y);
        TileType::Type type = (TileType::Type)tiletype_selection->tileType();
        if (mouse.buttonMask == ppltk::MouseState::Left) {
            layer.TileTypeMatrix.setType(x, y, type);
        } else if (mouse.buttonMask == ppltk::MouseState::Right) {
            layer.TileTypeMatrix.setType(x, y, TileType::Type::NonBlocking);
        }
    } else if (tiles_selection) {
        /*
        int currentPlane = editor.mainmenue->currentPlane();

        ppl7::grafix::Point coords = WorldCoords * planeFactor[currentPlane];
        int x = (mouse.p.x + coords.x) / TILE_WIDTH;
        int y = (mouse.p.y + coords.y) / TILE_HEIGHT;

        int selectedTile = tiles_selection->selectedTile();
        int selectedTileSet = tiles_selection->currentTileSet();
        int currentLayer = tiles_selection->currentLayer();
        int color_index = tiles_selection->colorIndex();
        Plane& plane = level.plane(currentPlane);

        if ((mouse.buttonMask == ppltk::MouseState::Right || mouse.buttonMask == ppltk::MouseState::Middle) && state[SDL_SCANCODE_LSHIFT]) {
            // Pick Tile
            ppl7::grafix::Point p = plane.getOccupationOrigin(x, y, currentLayer);
            if (p.x >= 0 && p.y >= 0) {
                tiles_selection->setCurrentTileSet(plane.getTileSet(p.x, p.y, currentLayer));
                tiles_selection->setSelectedTile(plane.getTileNo(p.x, p.y, currentLayer));
                tiles_selection->setColorIndex(plane.getColorIndex(p.x, p.y, currentLayer));
            }
        } else if (mouse.buttonMask == ppltk::MouseState::Left && selectedTile >= 0 && state[SDL_SCANCODE_LSHIFT] == 0) {
            BrickOccupation::Matrix occupation = brick_occupation.get(selectedTile);
            if (selectedTileSet == 1) occupation = brick_occupation_solid;
            if (!plane.isOccupied(x, y, currentLayer, occupation)) {
                plane.setTile(x, y, currentLayer, selectedTileSet, selectedTile, color_index, true);
                plane.setOccupation(x, y, currentLayer, occupation);
            }
        } else if (mouse.buttonMask == ppltk::MouseState::Right && state[SDL_SCANCODE_LSHIFT] == 0) {
            ppl7::grafix::Point origin = plane.getOccupationOrigin(x, y, currentLayer);
            if (origin.x >= 0 && origin.y >= 0) {
                int origin_tile = plane.getTileNo(origin.x, origin.y, currentLayer);
                int origin_tileset = plane.getTileSet(origin.x, origin.y, currentLayer);
                if (origin_tile >= 0) {
                    BrickOccupation::Matrix occupation = brick_occupation.get(origin_tile);
                    if (origin_tileset == 1) occupation = brick_occupation_solid;
                    plane.clearOccupation(origin.x, origin.y, currentLayer, occupation);
                }
                plane.clearTile(origin.x, origin.y, currentLayer);
            } else {
                plane.clearTile(x, y, currentLayer);
            }
        }
        */
    }
}
