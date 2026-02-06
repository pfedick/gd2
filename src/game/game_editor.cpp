#include "game.h"
#include "ui/tileselection.h"
#include "ui/tiletypeselection.h"
#include "ui/statusbar.h"
#include "ui/menue.h"
#include "ui/worldwidget.h"
#include "ui/objectselection.h"
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
    lastTileLayer = 2;
    lastTileType = 1;
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
        history.lastTileset = tiles_selection->currentTileSet();
        history.lastTile = tiles_selection->selectedTile();
        history.lastTileColor = tiles_selection->colorIndex();
        history.lastTileLayer = tiles_selection->currentTileLayer();
        delete tiles_selection;
        tiles_selection = NULL;
    }
    if (tiletype_selection) {
        history.lastTileType = tiletype_selection->tileType();
        delete tiletype_selection;
        tiletype_selection = NULL;
        mainmenue->setShowTileTypes(false);
    }
    if (object_selection) {
        history.ObjectType = object_selection->selectedObjectType();
        history.ObjectDifficulty = object_selection->getDifficulty();
        history.ObjectLayer = object_selection->currentLayer();
        delete object_selection;
        object_selection = NULL;
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
    tiles_selection->setTileSet(static_cast<int>(Resources::TileSets::Granit), "Concrete",
                                &game->resources.Tiles[static_cast<int>(Resources::TileSets::Granit)].SpritesUi);
    tiles_selection->setCurrentTileSet(history.lastTileset);
    tiles_selection->setSelectedTile(history.lastTile);
    tiles_selection->setColorIndex(history.lastTileColor);
    tiles_selection->setTileLayer(history.lastTileLayer);
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
    tiletype_selection->setTileType(history.lastTileType);
    game->addChild(tiletype_selection);
    // viewport.x1 = 300;
    game->viewport.x1 = 300;
    game->world_widget->setViewport(game->viewport);
    game->game_viewport.setViewport(game->viewport);

    // hud->setViewport(viewport);
    mainmenue->setShowTileTypes(true);
    // mainmenue->setCurrentLayer(ParallaxLayerId::Player);
}

void GameEditor::showSpriteSelection()
{
    closeAll();
}

void GameEditor::showObjectSelection()
{
    if (object_selection) {
        closeAll();
        return;
    }
    closeAll();
    object_selection = new ObjectSelection(0, 32, 300, statusbar->y() - 32, game);
    object_selection->setObjectType(history.ObjectType);
    object_selection->setObjectDifficulty(history.ObjectDifficulty);
    object_selection->setLayer(history.ObjectLayer);
    object_selection->setSpriteSet(&game->resources.ObjectsUi);
    game->addChild(object_selection);
    game->viewport.x1 = 300;
    game->world_widget->setViewport(game->viewport);
    game->game_viewport.setViewport(game->viewport);
}

void GameEditor::updateDifficultyForSelectedObject(uint8_t dificulty)
{
}

void GameEditor::updateObjectLayerForSelectedObject(int layer)
{
}

void GameEditor::setSpriteModeToDraw()
{
}

void GameEditor::handleMouseDrawInWorld(const ppltk::MouseState& mouse)
{
    // ppl7::PrintDebug("GameEditor::handleMouseDrawInWorld\n");
    const bool* state = SDL_GetKeyboardState(NULL);
    //    if (state[SDL_SCANCODE_LSHIFT]) return;

    if (tiletype_selection) {

        ParallaxLayerId currentLayer = mainmenue->currentLayer();
        ParallaxLayer& layer = game->level.layer(currentLayer);
        ppl7::grafix::Point coords = game->WorldCoords * layer.speed_factor * layer.size_factor;

        int x = (mouse.p.x + coords.x) / (TILE_WIDTH * layer.size_factor);
        int y = (mouse.p.y + coords.y) / (TILE_HEIGHT * layer.size_factor);
        // ppl7::PrintDebug("TileTypeSelection Mouse Draw at %d:%d\n", x, y);
        TileType::Type type = (TileType::Type)tiletype_selection->tileType();
        if (mouse.buttonMask == ppltk::MouseState::Left) {
            layer.TileTypeMatrix.setType(x, y, type);
        } else if (mouse.buttonMask == ppltk::MouseState::Right) {
            layer.TileTypeMatrix.setType(x, y, TileType::Type::NonBlocking);
        }
    } else if (tiles_selection) {
        ParallaxLayerId currentLayer = mainmenue->currentLayer();
        ParallaxLayer& layer = game->level.layer(currentLayer);
        ppl7::grafix::Point coords = game->WorldCoords * layer.speed_factor * layer.size_factor;
        int x = (mouse.p.x + coords.x) / (TILE_WIDTH * layer.size_factor);
        int y = (mouse.p.y + coords.y) / (TILE_HEIGHT * layer.size_factor);

        int selectedTile = tiles_selection->selectedTile();
        int selectedTileSet = tiles_selection->currentTileSet();
        int currentTileLayer = tiles_selection->currentTileLayer();
        int color_index = tiles_selection->colorIndex();

        if ((mouse.buttonMask == ppltk::MouseState::Right || mouse.buttonMask == ppltk::MouseState::Middle) && state[SDL_SCANCODE_LSHIFT]) {
            // Pick Tile
            ppl7::grafix::Point p = layer.tiles.getOccupationOrigin(x, y, currentTileLayer);
            if (p.x >= 0 && p.y >= 0) {
                tiles_selection->setCurrentTileSet(layer.tiles.getTileSet(p.x, p.y, currentTileLayer));
                tiles_selection->setSelectedTile(layer.tiles.getTileNo(p.x, p.y, currentTileLayer));
                tiles_selection->setColorIndex(layer.tiles.getColorIndex(p.x, p.y, currentTileLayer));
            }
        } else if (mouse.buttonMask == ppltk::MouseState::Left && selectedTile >= 0 && state[SDL_SCANCODE_LSHIFT] == 0) {
            const TileOccupation::Matrix& occupation = game->resources.Tiles[selectedTileSet].Occupation.get(selectedTile);
            if (!layer.tiles.isOccupied(x, y, currentTileLayer, occupation)) {
                layer.tiles.setTile(x, y, currentTileLayer, selectedTileSet, selectedTile, color_index, true);
                layer.tiles.setOccupation(x, y, currentTileLayer, occupation);
            }
        } else if (mouse.buttonMask == ppltk::MouseState::Right && state[SDL_SCANCODE_LSHIFT] == 0) {
            ppl7::grafix::Point origin = layer.tiles.getOccupationOrigin(x, y, currentTileLayer);
            if (origin.x >= 0 && origin.y >= 0) {
                int origin_tile = layer.tiles.getTileNo(origin.x, origin.y, currentTileLayer);
                int origin_tileset = layer.tiles.getTileSet(origin.x, origin.y, currentTileLayer);
                if (origin_tile >= 0) {
                    const TileOccupation::Matrix& occupation = game->resources.Tiles[origin_tileset].Occupation.get(origin_tile);
                    layer.tiles.clearOccupation(origin.x, origin.y, currentTileLayer, occupation);
                }
                layer.tiles.clearTile(origin.x, origin.y, currentTileLayer);
            } else {
                layer.tiles.clearTile(x, y, currentTileLayer);
            }
        }
    }
}
