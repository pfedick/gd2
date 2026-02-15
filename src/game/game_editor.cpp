#include <math.h>
#include "game.h"
#include "ui/tileselection.h"
#include "ui/tiletypeselection.h"
#include "ui/statusbar.h"
#include "ui/menue.h"
#include "ui/worldwidget.h"
#include "ui/objectselection.h"
#include "constants.h"
#include "resources.h"

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
    selected_object = NULL;
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
        selected_object = NULL;
    }
    game->viewport.x1 = 0;
    if (game->world_widget) game->world_widget->setViewport(game->viewport);
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
    if (selected_object) selected_object->difficulty_matrix = dificulty;
}

void GameEditor::updateObjectLayerForSelectedObject(int layer)
{
    if (selected_object) {
        selected_object->myLayer = static_cast<Objects::Object::Layer>(layer);
    }
}
void GameEditor::updateParallaxLayerForSelectedObject(ParallaxLayerId plane)
{
    if (selected_object) {
        if (static_cast<ParallaxLayerId>(plane) != selected_object->myPlane) {
            const ParallaxLayer& oldLayer = game->level.layer(selected_object->myPlane);
            ppl7::grafix::PointF coords = game->WorldCamera * oldLayer.speed_factor;
            ppl7::grafix::PointF pp_initial = ppl7::grafix::PointF(selected_object->initial_p) - coords;
            ppl7::grafix::PointF pp_current = ppl7::grafix::PointF(selected_object->p) - coords;
            const ParallaxLayer& newLayer = game->level.layer(plane);
            coords = game->WorldCamera * newLayer.speed_factor;
            selected_object->initial_p = pp_initial + coords;
            selected_object->p = pp_current + coords;
            selected_object->updateBoundary();
            selected_object->myPlane = plane;
            // if (selected_object->myPlane != PlaneId::Player && static_cast<int>(selected_object->myLayer) > 1)
            //     selected_object->myLayer = Decker::Objects::Object::Layer::BeforeBricks;
        }
        // ppl7::PrintDebugTime("Update Layer to: %d\n", layer);
    }
}

void GameEditor::setSpriteModeToDraw()
{
    sprite_mode = SpriteMode::Draw;
    selected_object = NULL;
}

void GameEditor::handleMouseDrawInWorld(const ppltk::MouseState& mouse)
{
    // ppl7::PrintDebug("GameEditor::handleMouseDrawInWorld\n");
    const bool* state = SDL_GetKeyboardState(NULL);
    //    if (state[SDL_SCANCODE_LSHIFT]) return;

    if (tiletype_selection) {

        ParallaxLayerId currentLayer = mainmenue->currentLayer();
        ParallaxLayer& layer = game->level.layer(currentLayer);
        ppl7::grafix::Point coords = game->WorldCamera * layer.speed_factor * layer.size_factor;

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
        ppl7::grafix::Point coords = game->WorldCamera * layer.speed_factor * layer.size_factor;
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

void GameEditor::drawSelection(GPUBatcher& batcher)
{
    game->editor.drawSelectedSprite(batcher, mouse.p - game->viewport.topLeft());
    game->editor.drawSelectedObject(batcher, mouse.p - game->viewport.topLeft());
    game->editor.drawSelectedTile(batcher, mouse.p - game->viewport.topLeft());
}

void GameEditor::drawSelectedSprite(GPUBatcher& batcher, const ppl7::grafix::Point& mouse)
{
    if (!sprite_selection) return;
    // mouse.p
    /*
    if (sprite_selection->selectedSprite() >= 0 && sprite_mode != spriteModeDraw) {
        selected_sprite_system = NULL;
        sprite_mode = spriteModeDraw;
    }
    if (sprite_mode == SpriteModeEdit && selected_sprite.id >= 0 && selected_sprite_system != NULL) {
        int currentPlane = mainmenue->currentPlane();
        selected_sprite_system->drawSelectedSpriteOutline(renderer, game_viewport, WorldCoords * planeFactor[currentPlane],
                                                          selected_sprite.id);
    } else if (sprite_mode == spriteModeDraw) {
        if (!mouse.inside(game_viewport)) return;
        int nr = sprite_selection->selectedSprite();
        if (nr < 0) return;
        int spriteset = sprite_selection->currentSpriteSet();
        int sprite_dimensions = sprite_selection->spriteSetDimensions();
        if (spriteset == 7) {
            if (nr == 0)
                nr = ppl7::rand(0, 47);
            else
                nr = (nr - 1) * 6;
        } else if (sprite_dimensions > 1) {
            // nr=nr * sprite_dimensions + ppl7::rand(0, sprite_dimensions - 1);
            nr = nr * sprite_dimensions;
        }
        ppl7::grafix::Point tmouse = game_viewport.translate(mouse);
        float scale = sprite_selection->spriteScale();
        float rotation = sprite_selection->spriteRotation();
        if (!level.spriteset[spriteset]) return;
        level.spriteset[spriteset]->drawScaledWithAngle(renderer, tmouse.x, tmouse.y, nr, scale, scale, rotation,
                                                        level.palette.getColor(sprite_selection->colorIndex()));
        level.spriteset[spriteset]->drawOutlinesWithAngle(renderer, tmouse.x, tmouse.y, nr, scale, scale, rotation);
    }
        */
}

void GameEditor::drawSelectedTile(GPUBatcher& batcher, const ppl7::grafix::Point& mouse)
{
    if (!tiles_selection) return;
    if (mouse.x < 0 || mouse.y < 0) return;

    ParallaxLayerId currentParalaxLayer = mainmenue->currentLayer();
    ParallaxLayer& layer = game->level.layer(currentParalaxLayer);
    int currentLayer = tiles_selection->currentTileLayer();
    int nr = tiles_selection->selectedTile();
    int tileset = tiles_selection->currentTileSet();
    int color_index = tiles_selection->colorIndex();
    if (nr < 0 || tileset < 0 || tileset >= static_cast<int>(Resources::TileSets::MaxTileSet)) return;
    ppl7::grafix::PointF tmouse = game->game_viewport.translate(mouse);

    // Effektive Kamera-Position für diesen Layer
    ppl7::grafix::PointF parallax_worldcoords = game->WorldCamera * layer.speed_factor * layer.size_factor;
    ppl7::grafix::PointF wp = tmouse + parallax_worldcoords;

    float scaled_tile_width = TILE_WIDTH * layer.size_factor;
    float scaled_tile_height = TILE_HEIGHT * layer.size_factor;

    int tx = static_cast<int>(floorf(wp.x / scaled_tile_width));
    int ty = static_cast<int>(floorf(wp.y / scaled_tile_height));

    TileResource& tile_resource = game->resources.Tiles[tileset];
    TileOccupation::Matrix occupation = tile_resource.Occupation.get(nr);
    if (!layer.tiles.isOccupied(tx, ty, currentLayer, occupation)) {
        float x = (float)tx * scaled_tile_width - parallax_worldcoords.x;
        float y = (float)ty * scaled_tile_height - parallax_worldcoords.y;
        batcher.addSprite(tile_resource.Sprites, nr, x, y + scaled_tile_height, layer.size_factor, layer.size_factor, 0.0f,
                          game->level.palette.getColor(color_index));
    }
}

void GameEditor::drawSelectedObject(GPUBatcher& batcher, const ppl7::grafix::Point& mouse)
{
    if (!object_selection) return;
    ParallaxLayer& layer = game->level.layer(mainmenue->currentLayer());
    /*
    if (sprite_mode == SpriteMode::Edit && selected_object != NULL) {
        layer.objects.drawSelectedSpriteOutline(batcher, game_viewport,
                                                WorldCoords * planeFactor[static_cast<int>(selected_object->myPlane)], selected_object->id);
    } else if (sprite_mode == spriteModeDraw) {
        if (!mouse.inside(game_viewport)) return;
        int object_type = object_selection->selectedObjectType();
        if (object_type < 0) return;
        ppl7::grafix::Point tmouse = game_viewport.translate(mouse);
        level.objects->drawPlaceSelection(renderer, tmouse, object_type);
    }
        */
}