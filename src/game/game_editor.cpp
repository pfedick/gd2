#include <math.h>
#include "game.h"
#include "ui/tileselection.h"
#include "ui/tiletypeselection.h"
#include "ui/statusbar.h"
#include "ui/menue.h"
#include "ui/worldwidget.h"
#include "ui/objectselection.h"
#include "ui/spriteselection.h"
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
        game->level.setObjectEditmode(false);
    }
    if (sprite_selection) {
        history.SpriteLayer = sprite_selection->currentSpriteLayer();
        history.SpriteColorIndex = sprite_selection->colorIndex();
        delete sprite_selection;
        sprite_selection = NULL;
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
    tiletype_selection = new TileTypeSelection(0, 32, 300, statusbar->y() - 32, game, &game->resources.TileTypesUi);
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
    if (sprite_selection) {
        closeAll();
        return;
    }
    closeAll();
    sprite_selection = new SpriteSelection(0, 32, 300, statusbar->y() - 32, game);
    sprite_selection->setSpriteSet(1, "Trees", &game->resources.SpriteSets[static_cast<int>(Resources::SpriteSets::Trees)].SpritesUi, 4);
    sprite_selection->setCurrentSpriteLayer(history.SpriteLayer);
    sprite_selection->setColorIndex(history.SpriteColorIndex);
    sprite_mode = SpriteMode::Draw;
    selected_sprite.id = -1;
    selected_sprite_system = NULL;

    game->addChild(sprite_selection);
    game->viewport.x1 = 300;
    game->world_widget->setViewport(game->viewport);
    game->game_viewport.setViewport(game->viewport);
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
    game->level.setObjectEditmode(true);
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
        if (static_cast<ParallaxLayerId>(plane) != selected_object->myParallaxLayer) {
            const ParallaxLayer& oldLayer = game->level.layer(selected_object->myParallaxLayer);
            ppl7::grafix::PointF coords = game->WorldCamera * oldLayer.speed_factor;
            ppl7::grafix::PointF pp_initial = ppl7::grafix::PointF(selected_object->initial_p) - coords;
            ppl7::grafix::PointF pp_current = ppl7::grafix::PointF(selected_object->p) - coords;
            const ParallaxLayer& newLayer = game->level.layer(plane);
            coords = game->WorldCamera * newLayer.speed_factor;
            selected_object->initial_p = pp_initial + coords;
            selected_object->p = pp_current + coords;
            selected_object->updateBoundary();
            selected_object->myParallaxLayer = plane;
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

void GameEditor::drawSelection(GameRenderer& renderer)
{
    game->editor.drawSelectedSprite(renderer, mouse.p - game->viewport.topLeft());
    game->editor.drawSelectedObject(renderer, mouse.p - game->viewport.topLeft());
    game->editor.drawSelectedTile(renderer, mouse.p - game->viewport.topLeft());
}

void GameEditor::drawSelectedSprite(GameRenderer& renderer, const ppl7::grafix::Point& mouse)
{
    if (!sprite_selection) return;
    // if (mouse.x < 0 || mouse.y < 0) return;
    if (sprite_selection->selectedSprite() >= 0 && sprite_mode != SpriteMode::Draw) {
        selected_sprite_system = NULL;
        sprite_mode = SpriteMode::Draw;
    }
    ParallaxLayerId currentParalaxLayer = mainmenue->currentLayer();
    ParallaxLayer& layer = game->level.layer(currentParalaxLayer);
    ppl7::grafix::PointF parallax_worldcoords = game->WorldCamera * layer.speed_factor * layer.size_factor;
    // ParallaxLayer::SpritePosition sprite_layer = sprite_selection->currentSpriteLayer();
    if (sprite_mode == SpriteMode::Edit && selected_sprite.id >= 0 && selected_sprite_system != NULL) {

        selected_sprite_system->drawSelectedSpriteOutline(renderer, parallax_worldcoords, selected_sprite.id);
    } else if (sprite_mode == SpriteMode::Draw) {
        int nr = sprite_selection->selectedSprite();
        if (nr < 0) return;
        int spriteset = sprite_selection->currentSpriteSet();
        int sprite_dimensions = sprite_selection->spriteSetDimensions();
        if (sprite_dimensions > 1) {
            nr = nr * sprite_dimensions;
        }

        ppl7::grafix::PointF tmouse = game->game_viewport.translate(mouse);
        float scale = sprite_selection->spriteScale() * layer.size_factor;
        float rotation = sprite_selection->spriteRotation();
        if (nr < 0 || spriteset < 0 || spriteset >= static_cast<int>(Resources::SpriteSets::MaxSpriteSet)) return;
        // game->resources.SpriteSets[static_cast<int>(spriteset)].Sprites.drawScaledWithAngle(
        //     batcher, tmouse.x, tmouse.y, nr, scale, scale, rotation, game->level.palette.getColor(sprite_selection->colorIndex()));
        game->resources.SpriteSets[static_cast<int>(spriteset)].Sprites.drawOutlinesWithAngle(renderer, tmouse.x, tmouse.y, nr, scale,
                                                                                              scale, rotation);
    }
}

void GameEditor::drawSelectedTile(GameRenderer& renderer, const ppl7::grafix::Point& mouse)
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
        renderer.addSprite(tile_resource.Sprites, nr, x, y + scaled_tile_height, layer.size_factor, layer.size_factor, 0.0f,
                           game->level.palette.getColor(color_index));
    }
}

void GameEditor::drawSelectedObject(GameRenderer& renderer, const ppl7::grafix::Point& mouse)
{
    if (!object_selection) return;
    ParallaxLayer& layer = game->level.layer(mainmenue->currentLayer());
    ppl7::grafix::PointF parallax_worldcoords = game->WorldCamera * layer.speed_factor * layer.size_factor;

    if (sprite_mode == SpriteMode::Edit && selected_object != NULL) {
        // selected_sprite_system->drawSelectedSpriteOutline(batcher, parallax_worldcoords, selected_sprite.id, layer.size_factor);
        layer.objects.drawSelectedSpriteOutline(renderer, parallax_worldcoords, selected_object->id);
    } else if (sprite_mode == SpriteMode::Draw) {
        Objects::Type object_type = object_selection->selectedObjectType();
        if (object_type == Objects::Type::Invalid) return;
        ppl7::grafix::PointF tmouse = game->game_viewport.translate(mouse);
        layer.objects.drawPlaceSelection(renderer, tmouse, object_type);
    }
}

void GameEditor::mouseDownEventOnObject(ppltk::MouseEvent* event)
{
    if ((event->buttonMask == ppltk::MouseState::Middle ||
         (event->buttonMask == ppltk::MouseState::Left && SDL_GetKeyboardState(NULL)[SDL_SCANCODE_LSHIFT]))) {
        ParallaxLayer& layer = game->level.layer(mainmenue->currentLayer());
        ppl7::grafix::Point coords = game->WorldCamera * layer.speed_factor * layer.size_factor;
        coords += event->p;
        Objects::Object* object = layer.objects.findMatchingObject(coords);
        if (object) {
            game->wm->setKeyboardFocus(game->world_widget);
            object_selection->setObjectType(object->type());
            object_selection->setObjectDifficulty(object->difficulty_matrix);
            object_selection->setLayer(object->myLayer);
            // object_selection->setPlane(static_cast<int>(object->myPlane));
            sprite_mode = SpriteMode::Edit;
            selected_object = object;
            sprite_move_start = event->p;
            object->openUi();
        }

    } else if (event->buttonMask == ppltk::MouseState::Left) {
        Objects::Type object_type = object_selection->selectedObjectType();
        if (object_type == Objects::Type::Invalid || sprite_mode == SpriteMode::Select || sprite_mode == SpriteMode::Edit) {
            sprite_mode = SpriteMode::Select;
            ParallaxLayer& layer = game->level.layer(mainmenue->currentLayer());
            ppl7::grafix::Point coords = game->WorldCamera * layer.speed_factor * layer.size_factor;
            coords += event->p;
            Objects::Object* object = layer.objects.findMatchingObject(coords);
            if (object) {
                // printf ("found Object with id %d\n", object->id);
                game->wm->setKeyboardFocus(game->world_widget);
                sprite_mode = SpriteMode::Edit;
                // if (selected_object==object) object->openUi();
                selected_object = object;
                sprite_move_start = event->p;
                object_selection->setObjectType(object->type());
                object_selection->setObjectDifficulty(object->difficulty_matrix);
                object_selection->setLayer(object->myLayer);
            }
            return;
        }
        if (sprite_mode != SpriteMode::Draw) return;
        ParallaxLayer& layer = game->level.layer(mainmenue->currentLayer());
        selected_object = layer.objects.getInstance(object_type);
        if (selected_object) {
            selected_object->difficulty_matrix = object_selection->getDifficulty();
            selected_object->myLayer = object_selection->currentLayer();
            selected_object->myParallaxLayer = layer.myParallaxLayer;
            ppl7::grafix::Point coords = game->WorldCamera * layer.speed_factor * layer.size_factor;
            selected_object->initial_p.setPoint(event->p.x + coords.x, event->p.y + coords.y);
            selected_object->p = selected_object->initial_p;
            layer.objects.addObject(selected_object);
            sprite_mode = SpriteMode::Draw;
        }
    } else if (event->buttonMask == ppltk::MouseState::Right) {
        sprite_mode = SpriteMode::Select;
        selected_object = NULL;
    }
}

void GameEditor::mouseDownEventOnSprite(ppltk::MouseEvent* event)
{
#ifdef EVENTTRACKING
    ppl7::PrintDebugTime("Game::mouseDownEventOnSprite\n");
#endif

    if (event->buttonMask == ppltk::MouseState::Left) {
        int nr = sprite_selection->selectedSprite();
        if (nr < 0) {
            selectSprite(event->p);
            return;
        }
        if (sprite_mode != SpriteMode::Draw) return;
        int spriteset = sprite_selection->currentSpriteSet();
        int sprite_dimensions = sprite_selection->spriteSetDimensions();
        if (sprite_dimensions > 1) {
            nr = nr * sprite_dimensions + ppl7::rand(0, sprite_dimensions - 1);
        }
        float scale = sprite_selection->spriteScale();
        float rotation = sprite_selection->spriteRotation();
        ParallaxLayer::SpritePosition sprite_layer = sprite_selection->currentSpriteLayer();
        int z_axis = sprite_selection->zAxis();

        ParallaxLayerId parallax_layer = mainmenue->currentLayer();
        if (spriteset >= static_cast<int>(Resources::SpriteSets::MaxSpriteSet)) return;
        ParallaxLayer& layer = game->level.layer(parallax_layer);
        ppl7::grafix::Point coords = game->WorldCamera * layer.speed_factor * layer.size_factor;

        if (sprite_layer == ParallaxLayer::SpritePosition::Background) {
            layer.background_sprites.addSprite(event->p.x + coords.x, event->p.y + coords.y, z_axis, spriteset, nr, scale, rotation,
                                               sprite_selection->colorIndex());
        }
        if (sprite_layer == ParallaxLayer::SpritePosition::Front) {
            layer.front_sprites.addSprite(event->p.x + coords.x, event->p.y + coords.y, z_axis, spriteset, nr, scale, rotation,
                                          sprite_selection->colorIndex());
        }
    } else if (event->buttonMask == ppltk::MouseState::Right) {
        sprite_selection->setSelectedSprite(-1);
        sprite_mode = SpriteMode::Draw;
        selected_sprite.id = -1;
        selected_sprite_system = NULL;
    }
}

void GameEditor::mouseWheelEventOnSprite(ppltk::MouseEvent* event)
{
    const bool* state = SDL_GetKeyboardState(NULL);
    if (state[SDL_SCANCODE_LSHIFT]) {
        if (sprite_mode == SpriteMode::Draw) {
            float angle = sprite_selection->spriteRotation();
            if (event->wheel.y < 0) angle -= 5;
            if (event->wheel.y > 0) angle += 5;
            if (angle <= 0) angle += 360;
            if (angle >= 360) angle -= 360;
            sprite_selection->setSpriteRotation(angle);
        } else if (sprite_mode == SpriteMode::Edit && selected_sprite.id >= 0 && selected_sprite_system != NULL) {
            if (event->wheel.y < 0) selected_sprite.rotation -= 5;
            if (event->wheel.y > 0) selected_sprite.rotation += 5;
            if (selected_sprite.rotation <= 0) selected_sprite.rotation += 360;
            if (selected_sprite.rotation >= 360) selected_sprite.rotation -= 360;
            sprite_selection->setSpriteRotation(selected_sprite.rotation);
            selected_sprite_system->modifySprite(selected_sprite);
        }

    } else {
        if (sprite_mode == SpriteMode::Draw) {
            float scale = sprite_selection->spriteScale();
            if (event->wheel.y < 0 && scale > 0.1)
                scale -= 0.05;
            else if (event->wheel.y > 0 && scale < 4.0)
                scale += 0.05;
            sprite_selection->setSpriteScale(scale);
        } else if (sprite_mode == SpriteMode::Edit && selected_sprite.id >= 0 && selected_sprite_system != NULL) {
            // printf ("wheel\n");
            if (event->wheel.y < 0 && selected_sprite.scale > 0.1)
                selected_sprite.scale -= 0.05;
            else if (event->wheel.y > 0 && selected_sprite.scale < 4.0)
                selected_sprite.scale += 0.05;
            selected_sprite_system->modifySprite(selected_sprite);
            sprite_selection->setSpriteScale(selected_sprite.scale);
        }
    }
}

void GameEditor::selectSprite(const ppl7::grafix::Point& mouse)
{
    ParallaxLayerId parallax_layer = ParallaxLayerId::Player;
    ParallaxLayer::SpritePosition sprite_layer = ParallaxLayer::SpritePosition::Background;
    if (game->level.findSprite(mouse, game->WorldCamera, selected_sprite, parallax_layer, sprite_layer)) {
        // ppl7::PrintDebug("found Sprite on parallax_layer %d, layer %d\n", (int)parallax_layer, (int)sprite_layer);

        mainmenue->setCurrentLayer(parallax_layer);
        sprite_selection->enableNotfies(false);
        sprite_selection->setCurrentSpriteLayer(sprite_layer);
        sprite_selection->setCurrentSpriteSet(selected_sprite.sprite_set);
        sprite_selection->setZAxis(selected_sprite.z);
        sprite_selection->setSpriteScale(selected_sprite.scale);
        sprite_selection->setSpriteRotation(selected_sprite.rotation);
        sprite_selection->setColorIndex(selected_sprite.color_index);
        sprite_selection->enableNotfies(true);

        game->wm->setKeyboardFocus(game->world_widget);
        sprite_mode = SpriteMode::Edit;
        selected_sprite_system = selected_sprite.spritesystem;
        sprite_move_start = mouse;

    } else {
        selected_sprite.id = -1;
        selected_sprite_system = NULL;
    }
}

void GameEditor::updateSpriteFromUi()
{
    if (!sprite_selection) return;
    if (!selected_sprite_system) return;
    if (selected_sprite.id < 0) return;
    selected_sprite.z = sprite_selection->zAxis();
    selected_sprite.color_index = sprite_selection->colorIndex();
    selected_sprite.rotation = sprite_selection->spriteRotation();
    selected_sprite.scale = sprite_selection->spriteScale();
    ParallaxLayer& layer = game->level.layer(mainmenue->currentLayer());
    SpriteSystem* new_ss = NULL;
    if (sprite_selection->currentSpriteLayer() == ParallaxLayer::SpritePosition::Background) {
        new_ss = &layer.background_sprites;
    } else if (sprite_selection->currentSpriteLayer() == ParallaxLayer::SpritePosition::Front) {
        new_ss = &layer.front_sprites;
    }
    if (new_ss != selected_sprite_system) {
        selected_sprite_system->deleteSprite(selected_sprite.id);
        int id = new_ss->addSprite(selected_sprite);
        selected_sprite_system = new_ss;
        selected_sprite_system->getSprite(id, selected_sprite);
    } else {
        selected_sprite_system->modifySprite(selected_sprite);
    }
}

void GameEditor::mouseDownEventOnWayNet(ppltk::MouseEvent* event)
{
}

void GameEditor::mouseDownEventOnLight(ppltk::MouseEvent* event)
{
}