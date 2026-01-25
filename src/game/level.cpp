#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL3/SDL.h>
#include <ppl7-grafix.h>
#include "level.h"

RenderState::RenderState()
{
    cmdbuf = NULL;
    tex_render_lightmap = NULL;
    tex_render_layer = NULL;
    tex_render_normal = NULL;
    tex_depth_buffer = NULL;
    tex_blur_temp = NULL;
    gpu = NULL;
    renderpipelines = NULL;
    batcher = NULL;
}

Level::Level()
{
    // objects = new Decker::Objects::ObjectSystem(&waynet);
    //  particles = new ParticleSystem();
    editMode = false;
    bShowGrid = false;
    showSprites = true;
    showObjects = true;
    showParticles = true;
    lightsEnabled = true;
    editlayer = ParallaxLayerId::Player;
    SetGlobalColorPalette(palette);
    parallax_layers[static_cast<int>(ParallaxLayerId::Near)].init(ParallaxLayerId::Near, 1.5f, 1.5f, 2.0f);
    parallax_layers[static_cast<int>(ParallaxLayerId::Close)].init(ParallaxLayerId::Close, 0.8f, 1.3f, 1.5f);
    parallax_layers[static_cast<int>(ParallaxLayerId::Front)].init(ParallaxLayerId::Front, 0.0f, 1.0f, 1.0f);
    parallax_layers[static_cast<int>(ParallaxLayerId::Player)].init(ParallaxLayerId::Player, 0.0f, 1.0f, 1.0f);
    parallax_layers[static_cast<int>(ParallaxLayerId::Back)].init(ParallaxLayerId::Back, 0.0f, 1.0f, 1.0f);
    parallax_layers[static_cast<int>(ParallaxLayerId::Behind)].init(ParallaxLayerId::Behind, 0.1f, 0.9f, 0.9f);
    parallax_layers[static_cast<int>(ParallaxLayerId::Middle)].init(ParallaxLayerId::Middle, 0.3f, 0.8f, 0.8f);
    parallax_layers[static_cast<int>(ParallaxLayerId::Far)].init(ParallaxLayerId::Far, 0.8f, 0.6f, 0.6f);
    parallax_layers[static_cast<int>(ParallaxLayerId::Horizon)].init(ParallaxLayerId::Horizon, 1.2f, 0.4f, 0.4f);
    parallax_layers[static_cast<int>(ParallaxLayerId::Sky)].init(ParallaxLayerId::Sky, 0.0f, 0.3f, 0.3f);
    for (auto layer : parallax_layers) {
        layer.background_sprites.setColorPalette(palette);
        layer.front_sprites.setColorPalette(palette);
    }
}

Level::~Level()
{
    clear();
    if (renderstate.gpu) {
        if (renderstate.tex_render_layer) renderstate.gpu->destroyGPUTexture(renderstate.tex_render_layer);
        if (renderstate.tex_render_lightmap) renderstate.gpu->destroyGPUTexture(renderstate.tex_render_lightmap);
        if (renderstate.tex_blur_temp) renderstate.gpu->destroyGPUTexture(renderstate.tex_blur_temp);
        if (renderstate.tex_render_normal) renderstate.gpu->destroyGPUTexture(renderstate.tex_render_normal);
        if (renderstate.tex_depth_buffer) renderstate.gpu->destroyGPUTexture(renderstate.tex_depth_buffer);
    }
}

void Level::clear()
{
    for (auto& layer : parallax_layers) {
        layer.clear();
    }
    // waynet.clear();
    params.clear();
    runtimeParams.clear();
    // TODO: GetGame().texture_cache.clear();
}

void Level::setEditmode(bool enabled)
{
    editMode = enabled;
}

void Level::setShowSprites(bool enabled)
{
    showSprites = enabled;
}

void Level::setShowObjects(bool enabled)
{
    showObjects = enabled;
}

void Level::setShowParticles(bool enabled)
{
    showParticles = enabled;
}

void Level::setEnableLights(bool enabled)
{
    lightsEnabled = enabled;
}

void Level::setEditLayer(ParallaxLayerId layer)
{
    editlayer = layer;
    if (bShowGrid) {
        for (auto& pl : parallax_layers) {
            pl.showGrid(false);
        }
        parallax_layers[static_cast<int>(editlayer)].showGrid(true);
    }
}

void Level::setShowTileGrid(bool enable)
{
    bShowGrid = enable;
    for (auto& layer : parallax_layers) {
        layer.showGrid(false);
    }
    parallax_layers[static_cast<int>(editlayer)].showGrid(enable);
}

void Level::setTileset(int no, SpriteTexture* tileset)
{
    if (no >= (int)this->tileset.size()) {
        this->tileset.resize(no + 1, nullptr);
    }
    this->tileset[no] = tileset;
}

void Level::setSpriteset(int no, SpriteTexture* spriteset)
{
    if (no < 0) return;
    if (no >= (int)this->spriteset.size()) {
        this->spriteset.resize(no + 1, nullptr);
    }
    this->spriteset[no] = spriteset;
    for (auto& layer : parallax_layers) {
        layer.front_sprites.setSpriteset(no, spriteset);
        layer.background_sprites.setSpriteset(no, spriteset);
    }
}

void Level::create(int width, int height)
{
    clear();
    palette.setDefaults();
    for (auto& layer : parallax_layers) {
        int layer_width = static_cast<int>(width * layer.size_factor);
        int layer_height = static_cast<int>(height * layer.size_factor);
        layer.tiles.create(layer_width, layer_height);
        layer.TileTypeMatrix.create(layer_width, layer_height);
    }
}

Plane& Level::plane(ParallaxLayerId id)
{
    return parallax_layers[static_cast<int>(id)].tiles;
}

ParallaxLayer& Level::layer(ParallaxLayerId id)
{
    return parallax_layers[static_cast<int>(id)];
}

SpriteSystem& Level::spritesystem(ParallaxLayerId id, ParallaxLayer::SpritePosition layer)
{
    if (layer == ParallaxLayer::SpritePosition::Front) {
        return parallax_layers[static_cast<int>(id)].front_sprites;
    } else {
        return parallax_layers[static_cast<int>(id)].background_sprites;
    }
}

void Level::initialize(GPUContext& gpu, RenderPipelines& renderpipelines, GPUBatcher& batcher)
{
    renderstate.gpu = &gpu;
    renderstate.renderpipelines = &renderpipelines;
    renderstate.batcher = &batcher;
}

void Level::resizeRenderBuffer(const ppl7::grafix::Size& size)
{
    if (!renderstate.gpu) return;
    if (size != render_target_size) {
        ppl7::PrintDebug("Resizing Level Render Targets to %dx%d\n", size.width, size.height);
        if (renderstate.tex_render_layer) renderstate.gpu->destroyGPUTexture(renderstate.tex_render_layer);
        renderstate.tex_render_layer = renderstate.gpu->createRenderTarget(size.width, size.height);
        if (renderstate.tex_render_lightmap) renderstate.gpu->destroyGPUTexture(renderstate.tex_render_lightmap);
        renderstate.tex_render_lightmap = renderstate.gpu->createRenderTarget(size.width, size.height);
        if (renderstate.tex_blur_temp) renderstate.gpu->destroyGPUTexture(renderstate.tex_blur_temp);
        renderstate.tex_blur_temp = renderstate.gpu->createRenderTarget(size.width, size.height);
        if (renderstate.tex_depth_buffer) renderstate.gpu->destroyGPUTexture(renderstate.tex_depth_buffer);
        renderstate.tex_depth_buffer = renderstate.gpu->createDepthBuffer(size.width, size.height);
        render_target_size = size;
    }
}

void Level::load(const ppl7::String& Filename)
{
    clear();
    ppl7::File ff;
    ff.open(Filename, ppl7::File::READ);
    ppl7::ByteArray ba;
    ff.read(ba, 7);
    const char* buffer = ba.toCharPtr();
    if (memcmp(buffer, "PFMagic", 7) != 0) {
        printf("Invalid Fileformat\n");
        return;
    }
    while (!ff.eof()) {
        try {
            size_t bytes_read = ff.read(ba, 5);
            if (bytes_read != 5) break;
            buffer = ba.toCharPtr();
            size_t size = ppl7::Peek32(buffer);
            ChunkId id = static_cast<ChunkId>(ppl7::Peek8(buffer + 4));
            // printf ("load id=%d, size=%zd\n",id,size);
            if (size <= 5) continue;
            bytes_read = ff.read(ba, size - 5);
            if (bytes_read != size - 5) break;
            if (id == ChunkId::LevelParameter) {
                params.load(ba);
                runtimeParams = params;
                runtimeParams.CurrentSong = params.InitialSong;
            } else if (id == ChunkId::ColorPalette) {
                palette.load(ba);
            } else if (id == ChunkId::Tiles) {
                int layer = ppl7::Peek8(ba.adr());
                if (layer < static_cast<int>(ParallaxLayerId::MaxLayerId)) parallax_layers[layer].tiles.load(ba);
            } else if (id == ChunkId::Sprites) {
                int layer = ppl7::Peek8(ba.adr());
                int position = ppl7::Peek8((char*)ba.adr() + 1);

                if (layer < static_cast<int>(ParallaxLayerId::MaxLayerId)) {
                    if (position == 0) {
                        parallax_layers[layer].front_sprites.load(ba);
                    } else if (position == 1) {
                        parallax_layers[layer].background_sprites.load(ba);
                    }
                }
            } else if (id == ChunkId::TileTypes) {
                int layer = ppl7::Peek8(ba.adr());
                if (layer < static_cast<int>(ParallaxLayerId::MaxLayerId)) parallax_layers[layer].TileTypeMatrix.load(ba);
            } else if (id == ChunkId::Objects) {
                // objects->load(ba);
            } else if (id == ChunkId::WayNet) {
                // waynet.load(ba);
            } else if (id == ChunkId::Lights) {
                // lights.load(ba);
            }
        }
        catch (const ppl7::EndOfFileException&) {
            break;
        }
    }
    ff.close();
}

void Level::save(const ppl7::String& Filename)
{
    ppl7::File ff;
    ff.open(Filename, ppl7::File::WRITE);
    char* buffer[20];
    memcpy(buffer, "PFMagic", 7);
    ff.write(buffer, 7);
    params.save(ff, static_cast<int>(ChunkId::LevelParameter));
    palette.save(ff, static_cast<int>(ChunkId::ColorPalette));
    for (unsigned char layer = 0; layer < static_cast<unsigned char>(ParallaxLayerId::MaxLayerId); layer++) {
        parallax_layers[layer].TileTypeMatrix.save(ff, static_cast<int>(ChunkId::TileTypes), layer);
        parallax_layers[layer].tiles.save(ff, static_cast<int>(ChunkId::Tiles), layer);
        parallax_layers[layer].front_sprites.save(ff, static_cast<int>(ChunkId::Sprites), layer, 0);
        parallax_layers[layer].background_sprites.save(ff, static_cast<int>(ChunkId::Sprites), layer, 1);
    }

    // objects->save(ff, LevelChunkId::chunkObjects);
    // waynet.save(ff, LevelChunkId::chunkWayNet);
    // lights.save(ff, LevelChunkId::chunkLights);

    ff.close();
}

void Level::backup(const ppl7::String& Filename)
{
    ppl7::String path = ppl7::File::getPath(Filename);
    try {
        if (ppl7::File::exists(Filename)) {
            if (path.notEmpty())
                path += "/backup";
            else
                path = "backup";
            if (!ppl7::Dir::exists(path)) ppl7::Dir::mkDir(path, true);
            path += "/" + ppl7::File::getFilename(Filename);
            ppl7::String suffix = ppl7::File::getSuffix(path);
            path.chopRight(suffix.len() + 1);
            ppl7::DateTime date = ppl7::DateTime::currentTime();
            path += "_" + date.toString("%Y%m%d%H%M%S") + "." + suffix;
            // ppl7::PrintDebugTime("Backup to: >>%s<<\n", (const char*)path);
            ppl7::File::copy(Filename, path);
        }
    }
    catch (const ppl7::Exception& exp) {
        ppl7::PrintDebugTime("could not make backup of level file: %s => %s [%s]\n", (const char*)Filename, (const char*)path,
                             (const char*)exp.toString());
    }
}

void Level::updateObjects(double time)
{
    for (auto& layer : parallax_layers) {
        // layer.objects.update(time);
    }
}

void Level::updateParticles(double time)
{
    for (auto& layer : parallax_layers) {
        // layer.particles.update(time);
    }
}

#ifdef OLDCODE

void Level::drawPlane(SDL_Renderer* renderer, const Plane& plane, const ppl7::grafix::Point& worldcoords) const
{
    // printf("viewport: x=%d, y=%d\n",viewport.x1, viewport.y1);
    int tiles_width = viewport.width() / TILE_WIDTH + 9;
    int tiles_height = viewport.height() / TILE_HEIGHT + 4;
    int offset_x = worldcoords.x % TILE_WIDTH;
    int offset_y = worldcoords.y % TILE_HEIGHT;
    int start_x = worldcoords.x / TILE_WIDTH - 7;
    int start_y = worldcoords.y / TILE_HEIGHT;
    int x1 = viewport.x1 - offset_x - TILE_WIDTH * 7;
    int y1 = viewport.y1 - offset_y + TILE_HEIGHT;

    std::map<int, ColorPaletteItem>::const_iterator cit;

    for (int z = 0; z < MAX_TILE_LAYER; z++) {
        for (int y = tiles_height; y >= 0; y--) {
            for (int x = 0; x < tiles_width; x++) {
                const Tile* tile = plane.get(x + start_x, y + start_y);
                if (tile) {
                    // if (tile->layer[z].tileset>8) printf ("draw %d, %d\n",tile->layer[z].tileset, tile->layer[z].tileno);
                    if (tileset[tile->layer[z].tileset]) {
                        // printf ("%d = %zd\n,",tile->tileset[z], tileset[tile->tileset[z]]);
                        tileset[tile->layer[z].tileset]->draw(renderer, x1 + x * TILE_WIDTH, y1 + y * TILE_HEIGHT, tile->layer[z].tileno,
                                                              palette.getColor(tile->layer[z].color_index));
                    }
                }
            }
        }
    }
}

void Level::drawNonePlayerPlane(SDL_Renderer* renderer,
                                PlaneId planeid,
                                const Plane& plane,
                                const SpriteSystem& sprites1,
                                const SpriteSystem& sprites2,
                                const ppl7::grafix::Point& worldcoords,
                                Metrics& metrics,
                                Particle::Layer particle_back,
                                Particle::Layer particle_front)
{
    if (!plane.isVisible()) return;

    if (showObjects) { // Objects behind Bricks
        metrics.time_objects.start();
        if (!editMode) objects->draw(renderer, viewport, worldcoords, planeid, Decker::Objects::Object::Layer::BehindBricks);
        metrics.time_objects.stop();
    }

    if (showSprites) {
        metrics.time_sprites.start();
        sprites1.draw(renderer, viewport, worldcoords);
        metrics.time_sprites.stop();
    }
    drawParticles(renderer, particle_back, worldcoords, metrics);

    metrics.time_plane.start();
    drawPlane(renderer, plane, worldcoords);
    metrics.time_plane.stop();
    if (showSprites) {
        metrics.time_sprites.start();
        sprites2.draw(renderer, viewport, worldcoords);
        metrics.time_sprites.stop();
    }

    if (showObjects) { // Objects before Bricks
        metrics.time_objects.start();
        if (!editMode) objects->draw(renderer, viewport, worldcoords, planeid, Decker::Objects::Object::Layer::BeforeBricks);
        metrics.time_objects.stop();
    }
    drawParticles(renderer, particle_front, worldcoords, metrics);
}

void Level::drawParticles(SDL_Renderer* renderer, Particle::Layer layer, const ppl7::grafix::Point& worldcoords, Metrics& metrics)
{
    if (!showParticles) return;
    metrics.time_draw_particles.start();
    particles->draw(renderer, viewport, worldcoords, layer);
    metrics.time_draw_particles.stop();
}

void Level::prepareLayer(SDL_Renderer* renderer)
{
    if (lightsEnabled) {
        SDL_SetRenderTarget(renderer, tex_render_lightmap);
        SDL_SetRenderDrawColor(renderer, runtimeParams.GlobalLighting.red(), runtimeParams.GlobalLighting.green(),
                               runtimeParams.GlobalLighting.blue(), 255);
        SDL_RenderClear(renderer);
        SDL_SetRenderTarget(renderer, tex_render_layer);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
        SDL_RenderClear(renderer);
    } else {
        SDL_SetRenderTarget(renderer, tex_render_target);
    }
}

void Level::addLightmap(
    SDL_Renderer* renderer, LightPlaneId plane, LightPlayerPlaneMatrix pplane, const ppl7::grafix::Point& worldcoords, Metrics& metrics)
{
    if (screenshot) screenshot->save(plane, pplane, Screenshot::Type::Color);
    if (!lightsEnabled) return;
    metrics.time_lights.start();
    SDL_SetRenderTarget(renderer, tex_render_lightmap);
    lights.draw(renderer, viewport, worldcoords, plane, pplane);
    if (screenshot) screenshot->save(plane, pplane, Screenshot::Type::Lightmap);

    SDL_SetRenderTarget(renderer, tex_render_layer);
    SDL_RenderTexture(renderer, tex_render_lightmap, NULL, NULL);
    lights.drawLensFlares(renderer, viewport, worldcoords, plane, pplane);
    if (screenshot) screenshot->save(plane, pplane, Screenshot::Type::Final);
    // SDL_SetRenderTarget(renderer, tex_render_target);
    // SDL_RenderTexture(renderer, tex_render_layer, NULL, NULL);
    // lights.drawLensFlares(renderer, viewport, worldcoords, plane, pplane);
    metrics.time_lights.stop();
}

void Level::blurLayer(SDL_Renderer* renderer, float factor)
{
    // Textur liegt zu beginn in tex_render_layer und soll am Ende
    // nach tex_render_target kopiert werden
    if (factor <= 0.0f) {
        SDL_SetRenderTarget(renderer, tex_render_target);
        SDL_RenderTexture(renderer, tex_render_layer, NULL, NULL);
        return;
    }

    // Funktioniert noch nicht richtig
    // SDL_SetRenderTarget(renderer, tex_render_target);
    // SDL_RenderTexture(renderer, tex_render_layer, NULL, NULL);

    // return;

    struct BlurUniforms
    {
        float blurStrength;
        float padding1; // std140: vec2 alignment
        float texelSizeX;
        float texelSizeY;
    };

    // ppl7::PrintDebug("Level::blurLayer factor=%f\n", factor);

    float texWidth, texHeight;
    SDL_GetTextureSize(tex_render_layer, &texWidth, &texHeight);

    // ppl7::PrintDebug("Level::blurLayer texWidth=%f, texHeight=%f\n", texWidth, texHeight);

    BlurUniforms uniforms;
    uniforms.blurStrength = factor;
    uniforms.padding1 = 0.0f;
    uniforms.texelSizeX = 1.0f / texWidth;
    uniforms.texelSizeY = 1.0f / texHeight;

    // Pass 1: Horizontal Blur
    SDL_SetGPURenderStateFragmentUniforms(renderstate->blurHorizontalState,
                                          0, // slot_index
                                          &uniforms, sizeof(BlurUniforms));
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    SDL_SetRenderTarget(renderer, tex_blur_temp);
    SDL_RenderClear(renderer);
    SDL_SetGPURenderState(renderer, renderstate->blurHorizontalState);
    SDL_RenderTexture(renderer, tex_render_layer, NULL, NULL);

    SDL_SetGPURenderState(renderer, NULL);

    // Pass 2: Vertical Blur (analog mit anderem Shader)
    SDL_SetGPURenderStateFragmentUniforms(renderstate->blurVerticalState,
                                          0, // slot_index
                                          &uniforms, sizeof(BlurUniforms));
    SDL_SetRenderTarget(renderer, tex_render_target);
    SDL_SetGPURenderState(renderer, renderstate->blurVerticalState);
    SDL_RenderTexture(renderer, tex_blur_temp, NULL, NULL);

    // Shader deaktivieren
    SDL_SetGPURenderState(renderer, NULL);
}

void Level::draw(SDL_Renderer* renderer, const ppl7::grafix::Point& worldcoords, Player* player, Metrics& metrics, Glimmer* glimmer)
{
    prepareLayer(renderer);
    drawNonePlayerPlane(renderer, PlaneId::Horizon, HorizonPlane, HorizonSprites[0], HorizonSprites[1], worldcoords * planeFactor[5],
                        metrics, Particle::Layer::HorizonPlaneBack, Particle::Layer::HorizonPlaneFront);
    addLightmap(renderer, LightPlaneId::Horizon, LightPlayerPlaneMatrix::None,
                worldcoords * planeFactor[static_cast<int>(PlaneId::Horizon)], metrics);
    blurLayer(renderer, 1.2f);

    prepareLayer(renderer);

    drawParticles(renderer, Particle::Layer::FarPlaneBack, worldcoords * planeFactor[2], metrics);
    drawNonePlayerPlane(renderer, PlaneId::Far, FarPlane, FarSprites[0], FarSprites[1], worldcoords * planeFactor[2], metrics,
                        Particle::Layer::FarPlaneBack, Particle::Layer::FarPlaneFront);
    addLightmap(renderer, LightPlaneId::Far, LightPlayerPlaneMatrix::None, worldcoords * planeFactor[static_cast<int>(PlaneId::Far)],
                metrics);
    blurLayer(renderer, 0.8f);

    prepareLayer(renderer);

    drawNonePlayerPlane(renderer, PlaneId::Middle, MiddlePlane, MiddleSprites[0], MiddleSprites[1], worldcoords * planeFactor[4], metrics,
                        Particle::Layer::MiddlePlaneBack, Particle::Layer::MiddlePlaneFront);
    // addLightmap(renderer, MiddleLights, worldcoords * planeFactor[4], metrics);
    addLightmap(renderer, LightPlaneId::Middle, LightPlayerPlaneMatrix::None, worldcoords * planeFactor[static_cast<int>(PlaneId::Middle)],
                metrics);
    blurLayer(renderer, 0.5f);

    prepareLayer(renderer);

    drawNonePlayerPlane(renderer, PlaneId::Back, BackPlane, BackSprites[0], BackSprites[1], worldcoords * planeFactor[3], metrics,
                        Particle::Layer::BackplaneBack, Particle::Layer::BackplaneFront);
    addLightmap(renderer, LightPlaneId::Player, LightPlayerPlaneMatrix::Back, worldcoords * planeFactor[static_cast<int>(PlaneId::Back)],
                metrics);
    blurLayer(renderer, 0.0f);

    prepareLayer(renderer);
    if (PlayerPlane.isVisible()) {
        if (showSprites) {
            metrics.time_sprites.start();
            PlayerSprites[0].draw(renderer, viewport, worldcoords * planeFactor[0]);
            metrics.time_sprites.stop();
        }
        if (showObjects) { // Objects behind Bricks
            metrics.time_objects.start();
            if (!editMode)
                objects->draw(renderer, viewport, worldcoords * planeFactor[0], PlaneId::Player,
                              Decker::Objects::Object::Layer::BehindBricks);
            metrics.time_objects.stop();
        }
        drawParticles(renderer, Particle::Layer::BehindBricks, worldcoords * planeFactor[0], metrics);
        metrics.time_plane.start();
        drawPlane(renderer, PlayerPlane, worldcoords * planeFactor[0]);
        metrics.time_plane.stop();
        if (showSprites) {
            metrics.time_sprites.start();
            PlayerSprites[1].draw(renderer, viewport, worldcoords * planeFactor[0]);
            metrics.time_sprites.stop();
        }

        // Behind Player
        drawParticles(renderer, Particle::Layer::BehindPlayer, worldcoords * planeFactor[0], metrics);
        metrics.time_objects.start();
        if (showObjects) { // Objects behind Player
            // metrics.time_objects.start();
            if (!editMode)
                objects->draw(renderer, viewport, worldcoords * planeFactor[0], PlaneId::Player,
                              Decker::Objects::Object::Layer::BehindPlayer);
            // metrics.time_objects.stop();
        }
        // Player
        player->draw(renderer, viewport, worldcoords * planeFactor[0]);
        glimmer->draw(renderer, viewport, worldcoords * planeFactor[0]);
        if (showObjects) { // Objects before Player
            // metrics.time_objects.start();
            if (!editMode)
                objects->draw(renderer, viewport, worldcoords * planeFactor[0], PlaneId::Player,
                              Decker::Objects::Object::Layer::BeforePlayer);
            // metrics.time_objects.stop();
        }
        metrics.time_objects.stop();
        drawParticles(renderer, Particle::Layer::BeforePlayer, worldcoords * planeFactor[0], metrics);
        if (showSprites) {
            metrics.time_sprites.start();
            PlayerSprites[2].draw(renderer, viewport, worldcoords * planeFactor[0]);
            metrics.time_sprites.stop();
        }

        addLightmap(renderer, LightPlaneId::Player, LightPlayerPlaneMatrix::Player,
                    worldcoords * planeFactor[static_cast<int>(PlaneId::Player)], metrics);
        blurLayer(renderer, 0.0f);

        if (showObjects && editMode) {
            metrics.time_objects.start();

            objects->drawEditMode(renderer, viewport, worldcoords * planeFactor[5], PlaneId::Horizon,
                                  Decker::Objects::Object::Layer::BehindBricks);
            objects->drawEditMode(renderer, viewport, worldcoords * planeFactor[5], PlaneId::Horizon,
                                  Decker::Objects::Object::Layer::BeforeBricks);
            objects->drawEditMode(renderer, viewport, worldcoords * planeFactor[2], PlaneId::Far,
                                  Decker::Objects::Object::Layer::BehindBricks);
            objects->drawEditMode(renderer, viewport, worldcoords * planeFactor[2], PlaneId::Far,
                                  Decker::Objects::Object::Layer::BeforeBricks);
            objects->drawEditMode(renderer, viewport, worldcoords * planeFactor[4], PlaneId::Middle,
                                  Decker::Objects::Object::Layer::BehindBricks);
            objects->drawEditMode(renderer, viewport, worldcoords * planeFactor[4], PlaneId::Middle,
                                  Decker::Objects::Object::Layer::BeforeBricks);

            objects->drawEditMode(renderer, viewport, worldcoords * planeFactor[0], PlaneId::Player,
                                  Decker::Objects::Object::Layer::BehindBricks);
            objects->drawEditMode(renderer, viewport, worldcoords * planeFactor[0], PlaneId::Player,
                                  Decker::Objects::Object::Layer::BehindPlayer);
            objects->drawEditMode(renderer, viewport, worldcoords * planeFactor[0], PlaneId::Player,
                                  Decker::Objects::Object::Layer::BeforePlayer);

            metrics.time_objects.stop();
        }
    }
    // addLightmap(renderer, PlayerLights, worldcoords * planeFactor[0], metrics);
    prepareLayer(renderer);

    drawNonePlayerPlane(renderer, PlaneId::Front, FrontPlane, FrontSprites[0], FrontSprites[1], worldcoords * planeFactor[1], metrics,
                        Particle::Layer::FrontplaneBack, Particle::Layer::FrontplaneFront);
    // addLightmap(renderer, FrontLights, worldcoords * planeFactor[1], metrics);
    addLightmap(renderer, LightPlaneId::Player, LightPlayerPlaneMatrix::Front, worldcoords * planeFactor[static_cast<int>(PlaneId::Front)],
                metrics);
    blurLayer(renderer, 0.0f);

    prepareLayer(renderer);

    drawNonePlayerPlane(renderer, PlaneId::Near, NearPlane, NearSprites[0], NearSprites[1], worldcoords * planeFactor[6], metrics,
                        Particle::Layer::NearPlaneBack, Particle::Layer::NearPlaneFront);
    // addLightmap(renderer, NearLights, worldcoords * planeFactor[6], metrics);
    addLightmap(renderer, LightPlaneId::Near, LightPlayerPlaneMatrix::None, worldcoords * planeFactor[static_cast<int>(PlaneId::Near)],
                metrics);
    blurLayer(renderer, 1.5f);

    if (showObjects && editMode) {
        metrics.time_objects.start();
        objects->drawEditMode(renderer, viewport, worldcoords * planeFactor[6], PlaneId::Near,
                              Decker::Objects::Object::Layer::BehindBricks);
        objects->drawEditMode(renderer, viewport, worldcoords * planeFactor[6], PlaneId::Near,
                              Decker::Objects::Object::Layer::BeforeBricks);
        metrics.time_objects.stop();
    }

    if (screenshot) screenshot->save(Screenshot::Layer::Complete, Screenshot::Type::Final);
    screenshot = NULL;
}

#endif

void Level::draw(SDL_GPUCommandBuffer* cmdbuf,
                 SDL_GPUTexture* swapchainTexture,
                 const ppl7::grafix::PointF& worldcoords,
                 const ppl7::grafix::Rect& viewport)
{
    renderstate.cmdbuf = cmdbuf;
    parallax_layers[static_cast<int>(ParallaxLayerId::Sky)].draw(renderstate, swapchainTexture, worldcoords, viewport);
    parallax_layers[static_cast<int>(ParallaxLayerId::Horizon)].draw(renderstate, swapchainTexture, worldcoords, viewport);
    parallax_layers[static_cast<int>(ParallaxLayerId::Far)].draw(renderstate, swapchainTexture, worldcoords, viewport);
    parallax_layers[static_cast<int>(ParallaxLayerId::Middle)].draw(renderstate, swapchainTexture, worldcoords, viewport);
    parallax_layers[static_cast<int>(ParallaxLayerId::Behind)].draw(renderstate, swapchainTexture, worldcoords, viewport);
    parallax_layers[static_cast<int>(ParallaxLayerId::Back)].draw(renderstate, swapchainTexture, worldcoords, viewport);
    parallax_layers[static_cast<int>(ParallaxLayerId::Player)].draw(renderstate, swapchainTexture, worldcoords, viewport);
    parallax_layers[static_cast<int>(ParallaxLayerId::Front)].draw(renderstate, swapchainTexture, worldcoords, viewport);
    parallax_layers[static_cast<int>(ParallaxLayerId::Close)].draw(renderstate, swapchainTexture, worldcoords, viewport);
    parallax_layers[static_cast<int>(ParallaxLayerId::Near)].draw(renderstate, swapchainTexture, worldcoords, viewport);
}

void Level::updateVisibleObjects(const ppl7::grafix::PointF& worldcoords, const ppl7::grafix::Rect& viewport)
{
    for (auto& layer : parallax_layers) {
        layer.updateVisibleObjects(worldcoords, viewport);
    }
}

/*
void Level::updateVisibleLightsLists(const ppl7::grafix::Point& worldcoords, const ppl7::grafix::Rect& viewport)
{
    lights.updateVisibleLightList(worldcoords, viewport);
}

void Level::updateDynamicLightsLists(const ppl7::grafix::Point& worldcoords, const ppl7::grafix::Rect& viewport)
{
    lights.updateDynamicLightList(worldcoords, viewport);
}
    */

size_t Level::countSprites() const
{
    size_t total = 0;
    for (auto& layer : parallax_layers) {
        total += layer.background_sprites.count();
        total += layer.front_sprites.count();
    }
    return total;
}

size_t Level::countVisibleSprites() const
{
    size_t total = 0;
    for (auto& layer : parallax_layers) {
        if (!layer.tiles.isVisible()) continue;
        total += layer.background_sprites.countVisible();
        total += layer.front_sprites.countVisible();
    }
    return total;
}

/*
size_t Level::countLights() const
{
    return lights.count();
}

size_t Level::countVisibleLights() const
{
    return lights.countVisible();
}
    */

/*
bool Level::findSprite(
const ppl7::grafix::Point& p, const ppl7::grafix::Point& worldcoords, SpriteSystem::Item& item, int& plane, int& layer) const
{
if (NearPlane.isVisible()) {
    ppl7::grafix::Point coords = p + worldcoords * planeFactor[6];
    if (NearSprites[1].findMatchingSprite(coords, item)) {
        plane = 6;
        layer = 1;
        return true;
    }
    if (NearSprites[0].findMatchingSprite(coords, item)) {
        plane = 6;
        layer = 0;
        return true;
    }
}
if (FrontPlane.isVisible()) {
    ppl7::grafix::Point coords = p + worldcoords * planeFactor[1];
    if (FrontSprites[1].findMatchingSprite(coords, item)) {
        plane = 1;
        layer = 1;
        return true;
    }
    if (FrontSprites[0].findMatchingSprite(coords, item)) {
        plane = 1;
        layer = 0;
        return true;
    }
}
if (PlayerPlane.isVisible()) {
    ppl7::grafix::Point coords = p + worldcoords * planeFactor[0];
    if (PlayerSprites[2].findMatchingSprite(coords, item)) {
        plane = 0;
        layer = 2;
        return true;
    }

    if (PlayerSprites[1].findMatchingSprite(coords, item)) {
        plane = 0;
        layer = 1;
        return true;
    }
    if (PlayerSprites[0].findMatchingSprite(coords, item)) {
        plane = 0;
        layer = 0;
        return true;
    }
}
if (BackPlane.isVisible()) {
    ppl7::grafix::Point coords = p + worldcoords * planeFactor[3];
    if (BackSprites[1].findMatchingSprite(coords, item)) {
        plane = 3;
        layer = 1;
        return true;
    }
    if (BackSprites[0].findMatchingSprite(coords, item)) {
        plane = 3;
        layer = 0;
        return true;
    }
}
if (MiddlePlane.isVisible()) {
    ppl7::grafix::Point coords = p + worldcoords * planeFactor[4];
    if (MiddleSprites[1].findMatchingSprite(coords, item)) {
        plane = 4;
        layer = 1;
        return true;
    }
    if (MiddleSprites[0].findMatchingSprite(coords, item)) {
        plane = 4;
        layer = 0;
        return true;
    }
}
if (FarPlane.isVisible()) {
    ppl7::grafix::Point coords = p + worldcoords * planeFactor[2];
    if (FarSprites[1].findMatchingSprite(coords, item)) {
        plane = 2;
        layer = 1;
        return true;
    }
    if (FarSprites[0].findMatchingSprite(coords, item)) {
        plane = 2;
        layer = 0;
        return true;
    }
}
if (HorizonPlane.isVisible()) {
    ppl7::grafix::Point coords = p + worldcoords * planeFactor[5];
    if (HorizonSprites[1].findMatchingSprite(coords, item)) {
        plane = 5;
        layer = 1;
        return true;
    }
    if (HorizonSprites[0].findMatchingSprite(coords, item)) {
        plane = 5;
        layer = 0;
        return true;
    }
}
return false;
}
*/

size_t Level::tileCount() const
{
    size_t count = 0;
    for (auto& layer : parallax_layers) {
        count += layer.tiles.tileCount();
    }
    return count;
}

/*
ppl7::grafix::Rect Level::getOccupiedArea() const
{
    ppl7::grafix::Rect r;

    return r;
}
*/

ppl7::grafix::Rect Level::getOccupiedAreaFromTileTypePlane(ParallaxLayerId layer) const
{
    return parallax_layers[static_cast<int>(layer)].TileTypeMatrix.getOccupiedArea();
}

#ifdef TODO
LevelStats::LevelStats()
{
    player = NULL;
}

void LevelStats::clear()
{
    object_counter.clear();
}

void LevelStats::setPlayer(Player* player)
{
    this->player = player;
}

size_t LevelStats::getObjectCount(int type) const
{
    std::map<int, size_t>::const_iterator it = object_counter.find(type);
    if (it != object_counter.end()) return (*it).second;
    return 0;
}

void LevelStats::print() const
{
    std::map<int, size_t>::const_iterator it;
    printf("\n\n");
    for (it = object_counter.begin(); it != object_counter.end(); ++it) {
        if (player) {
            ppl7::PrintDebugTime("%-30s: Total: %5zd, Player: %zd\n",
                                 (const char*)Decker::Objects::Type::name((Decker::Objects::Type::ObjectType)(*it).first), (*it).second,
                                 player->getObjectCount((*it).first));

        } else {
            ppl7::PrintDebugTime("%s: %zd\n", (const char*)Decker::Objects::Type::name((Decker::Objects::Type::ObjectType)(*it).first),
                                 (*it).second);
        }
    }
}

void Level::getLevelStats(LevelStats& stats) const
{
    stats.clear();
    if (objects) objects->getObjectCounter(stats.object_counter);
}

#endif