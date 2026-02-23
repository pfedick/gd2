#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL3/SDL.h>
#include <ppl7-grafix.h>
#include "level.h"
#include "gpu.h"
#include "player.h"
#include "game.h"

RenderState::RenderState()
{
    cmdbuf = NULL;
    render_target = NULL;
    render_lightmap = NULL;
    render_layer = NULL;
    render_normal = NULL;
    depth_buffer = NULL;
    blur_temp = NULL;
    gpu = NULL;
    renderpipelines = NULL;
    batcher = NULL;
}

Level::Level(Game* game)
{
    // objects = new Decker::Objects::ObjectSystem(&waynet);
    //  particles = new ParticleSystem();
    objectEditMode = false;
    bShowGrid = false;
    bShowTileTypes = false;
    showSprites = true;
    showObjects = true;
    showParticles = true;
    lightsEnabled = true;
    blurEnabled = true;
    player = NULL;
    editlayer = ParallaxLayerId::Player;
    SetGlobalColorPalette(palette);
    parallax_layers[static_cast<int>(ParallaxLayerId::Near)].init(ParallaxLayerId::Near, 2.0f, 1.4f, 1.6f);
    parallax_layers[static_cast<int>(ParallaxLayerId::Close)].init(ParallaxLayerId::Close, 0.8f, 1.2f, 1.3f);
    parallax_layers[static_cast<int>(ParallaxLayerId::Front)].init(ParallaxLayerId::Front, 0.0f, 1.0f, 1.0f);
    parallax_layers[static_cast<int>(ParallaxLayerId::Player)].init(ParallaxLayerId::Player, 0.0f, 1.0f, 1.0f);
    parallax_layers[static_cast<int>(ParallaxLayerId::Back)].init(ParallaxLayerId::Back, 0.0f, 1.0f, 1.0f);
    parallax_layers[static_cast<int>(ParallaxLayerId::Behind)].init(ParallaxLayerId::Behind, 0.1f, 0.9f, 0.9f);
    parallax_layers[static_cast<int>(ParallaxLayerId::Middle)].init(ParallaxLayerId::Middle, 0.3f, 0.8f, 0.8f);
    parallax_layers[static_cast<int>(ParallaxLayerId::Far)].init(ParallaxLayerId::Far, 0.8f, 0.6f, 0.6f);
    parallax_layers[static_cast<int>(ParallaxLayerId::Horizon)].init(ParallaxLayerId::Horizon, 1.2f, 0.4f, 0.4f);
    parallax_layers[static_cast<int>(ParallaxLayerId::Sky)].init(ParallaxLayerId::Sky, 2.0f, 0.3f, 0.3f);
    for (auto& layer : parallax_layers) {
        layer.background_sprites.setColorPalette(palette);
        layer.front_sprites.setColorPalette(palette);
        layer.tiles.setColorPalette(palette);
        layer.game = game;
        layer.level = this;
    }
}

Level::~Level()
{
    clear();
    if (renderstate.gpu) {
        if (renderstate.render_target) renderstate.gpu->destroyGPUTexture(renderstate.render_target);
        if (renderstate.render_layer) renderstate.gpu->destroyGPUTexture(renderstate.render_layer);
        if (renderstate.render_lightmap) renderstate.gpu->destroyGPUTexture(renderstate.render_lightmap);
        if (renderstate.blur_temp) renderstate.gpu->destroyGPUTexture(renderstate.blur_temp);
        if (renderstate.render_normal) renderstate.gpu->destroyGPUTexture(renderstate.render_normal);
        if (renderstate.depth_buffer) renderstate.gpu->destroyGPUTexture(renderstate.depth_buffer);
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

void Level::setObjectEditmode(bool enabled)
{
    objectEditMode = enabled;
    for (auto& pl : parallax_layers) {
        pl.objectEditMode = enabled;
    }
}

void Level::updateVisibility()
{
    for (auto& pl : parallax_layers) {
        pl.bShowGrid = false;
        pl.bShowTileTypes = false;
        pl.isEditLayer = false;
    }
    parallax_layers[static_cast<int>(editlayer)].bShowGrid = bShowGrid;
    parallax_layers[static_cast<int>(editlayer)].bShowTileTypes = bShowTileTypes;
    parallax_layers[static_cast<int>(editlayer)].isEditLayer = true;
}

void Level::setEditLayer(ParallaxLayerId layer)
{
    editlayer = layer;
    updateVisibility();
}

void Level::setShowTileGrid(bool enable)
{
    bShowGrid = enable;
    updateVisibility();
}

void Level::setShowTileTypes(bool enable)
{
    bShowTileTypes = enable;
    updateVisibility();
}

void Level::setShowCollisions(bool enable)
{
    bShowCollisions = enable;
}

void Level::setShowTiles(bool enable)
{
    showTiles = enable;
    for (auto& layer : parallax_layers) {
        layer.bShowTiles = enable;
    }
}

void Level::setShowSprites(bool enable)
{
    showSprites = enable;
    for (auto& layer : parallax_layers) {
        layer.bShowSprites = enable;
    }
}

void Level::setShowObjects(bool enable)
{
    showObjects = enable;
    for (auto& layer : parallax_layers) {
        layer.bShowObjects = enable;
    }
}

void Level::setShowParticles(bool enable)
{
    showParticles = enable;
    for (auto& layer : parallax_layers) {
        layer.bShowParticles = enable;
    }
}

void Level::setLightsEnabled(bool enabled)
{
    lightsEnabled = enabled;
    for (auto& layer : parallax_layers) {
        layer.bLightningEnabled = enabled;
    }
}

void Level::setTileTypeSpriteset(SpriteTexture* tileset)
{
    for (auto& layer : parallax_layers) {
        layer.TileTypeMatrix.setTileTypesSprites(tileset);
    }
}

void Level::setTileset(int no, SpriteTexture* tileset)
{
    for (auto& layer : parallax_layers) {
        layer.tiles.setTileset(no, tileset);
    }
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

void Level::setSpritesetResources(ObjectSpritesets* spriteset)
{
    for (auto& layer : parallax_layers) {
        layer.objects.setSpritesetResources(spriteset);
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

ParallaxLayer& Level::layer(ParallaxLayerId id)
{
    return parallax_layers[static_cast<int>(id)];
}

ParallaxLayer& Level::editLayer()
{
    return parallax_layers[static_cast<int>(editlayer)];
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
        // ppl7::PrintDebug("Resizing Level Render Targets to %dx%d\n", size.width, size.height);
        if (renderstate.render_target) renderstate.gpu->destroyGPUTexture(renderstate.render_target);
        renderstate.render_target = renderstate.gpu->createRenderTarget(size.width, size.height);
        if (renderstate.render_layer) renderstate.gpu->destroyGPUTexture(renderstate.render_layer);
        renderstate.render_layer = renderstate.gpu->createRenderTarget(size.width, size.height);
        if (renderstate.render_lightmap) renderstate.gpu->destroyGPUTexture(renderstate.render_lightmap);
        renderstate.render_lightmap = renderstate.gpu->createRenderTarget(size.width, size.height);
        if (renderstate.blur_temp) renderstate.gpu->destroyGPUTexture(renderstate.blur_temp);
        renderstate.blur_temp = renderstate.gpu->createRenderTarget(size.width, size.height);
        if (renderstate.depth_buffer) renderstate.gpu->destroyGPUTexture(renderstate.depth_buffer);
        renderstate.depth_buffer = renderstate.gpu->createDepthBuffer(size.width, size.height);
        render_target_size = size;
        renderstate.render_target_size = size;
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
                int layer = ppl7::Peek8(ba.adr());
                if (layer < static_cast<int>(ParallaxLayerId::MaxLayerId)) parallax_layers[layer].objects.load(ba);
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
    for (auto& layer : parallax_layers) {
        int layer_width = static_cast<int>(params.width * layer.size_factor);
        int layer_height = static_cast<int>(params.height * layer.size_factor);
        if (layer.tiles.getSize().width != layer_width || layer.tiles.getSize().height != layer_height)
            layer.tiles.create(layer_width, layer_height);
        if (layer.TileTypeMatrix.size().width != layer_width || layer.TileTypeMatrix.size().height != layer_height)
            layer.TileTypeMatrix.create(layer_width, layer_height);
    }
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
        parallax_layers[layer].objects.save(ff, static_cast<int>(ChunkId::Objects), layer);
        // parallax_layers[layer].waynet.save(ff, static_cast<int>(ChunkId::Waynet), layer);
        // parallax_layers[layer].lights.save(ff, static_cast<int>(ChunkId::Lights), layer);
    }
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

void Level::updateParticles(double time)
{
    for (auto& layer : parallax_layers) {
        // layer.particles.update(time);
    }
}

void Level::clearRenderTarget(SDL_GPUCommandBuffer* cmdbuf)
{
    // 1. Das interne 4K-Target (render_target) einmalig leeren
    SDL_GPUColorTargetInfo rtInfo = {0};
    rtInfo.texture = renderstate.render_target;
    rtInfo.clear_color = toSDLFColor(runtimeParams.BackgroundColor);
    rtInfo.load_op = SDL_GPU_LOADOP_CLEAR;
    rtInfo.store_op = SDL_GPU_STOREOP_STORE;
    rtInfo.cycle = false;
    SDL_GPURenderPass* rtClearPass = SDL_BeginGPURenderPass(cmdbuf, &rtInfo, 1, NULL);
    SDL_EndGPURenderPass(rtClearPass);
}

void Level::copyRenderTargetToSwapchain(SDL_GPUCommandBuffer* cmdbuf, SDL_GPUTexture* swapchainTexture, const SDL_FRect& destRect)
{
    SDL_GPUColorTargetInfo colorTargetInfo = {0};
    colorTargetInfo.texture = swapchainTexture;
    colorTargetInfo.clear_color = (SDL_FColor){0.0f, 0.0f, 0.0f, 1.0f}; // Black background
    colorTargetInfo.load_op = SDL_GPU_LOADOP_CLEAR;
    colorTargetInfo.store_op = SDL_GPU_STOREOP_STORE;
    colorTargetInfo.cycle = false;

    SDL_GPURenderPass* renderPass = SDL_BeginGPURenderPass(cmdbuf, &colorTargetInfo, 1, NULL);

    // Hier setzen wir den Viewport auf die Zielgröße im Fenster
    SDL_GPUViewport gpuViewport = {
        .x = destRect.x, .y = destRect.y, .w = destRect.w, .h = destRect.h, .min_depth = 0.0f, .max_depth = 1.0f};

    SDL_SetGPUViewport(renderPass, &gpuViewport);
    SDL_SetGPUScissor(renderPass, NULL);

    SDL_BindGPUGraphicsPipeline(renderPass, renderstate.renderpipelines->copyPipeline);
    SDL_GPUTextureSamplerBinding binding = {};

    binding.texture = renderstate.render_target;
    binding.sampler = renderstate.renderpipelines->samplerClamp; // Einen Clamp-Sampler benutzen!
    SDL_BindGPUFragmentSamplers(renderPass, 0, &binding, 1);

    // Fullscreen Triangle zeichnen (3 Vertices, Shader generiert Coords)
    SDL_DrawGPUPrimitives(renderPass, 3, 1, 0, 0);

    SDL_EndGPURenderPass(renderPass);
}

void Level::setPlayer(Player* player)
{
    this->player = player;
    for (auto& layer : parallax_layers) {
        layer.setPlayer(player);
    }
}

void Level::draw(SDL_GPUCommandBuffer* cmdbuf,
                 SDL_GPUTexture* swapchainTexture,
                 const Camera& worldcoords,
                 const GameViewport& viewport,
                 Metrics& metrics)
{
    renderstate.cmdbuf = cmdbuf;
    // Step 1: Clear internal render target
    clearRenderTarget(cmdbuf);

    // Step 2: Draw all parallax layers in correct order to internal render target
    parallax_layers[static_cast<int>(ParallaxLayerId::Sky)].draw(renderstate, renderstate.render_target, worldcoords, viewport, metrics);
    parallax_layers[static_cast<int>(ParallaxLayerId::Horizon)].draw(renderstate, renderstate.render_target, worldcoords, viewport,
                                                                     metrics);
    parallax_layers[static_cast<int>(ParallaxLayerId::Far)].draw(renderstate, renderstate.render_target, worldcoords, viewport, metrics);
    parallax_layers[static_cast<int>(ParallaxLayerId::Middle)].draw(renderstate, renderstate.render_target, worldcoords, viewport, metrics);
    parallax_layers[static_cast<int>(ParallaxLayerId::Behind)].draw(renderstate, renderstate.render_target, worldcoords, viewport, metrics);
    parallax_layers[static_cast<int>(ParallaxLayerId::Back)].draw(renderstate, renderstate.render_target, worldcoords, viewport, metrics);
    parallax_layers[static_cast<int>(ParallaxLayerId::Player)].draw(renderstate, renderstate.render_target, worldcoords, viewport, metrics);
    parallax_layers[static_cast<int>(ParallaxLayerId::Front)].draw(renderstate, renderstate.render_target, worldcoords, viewport, metrics);
    parallax_layers[static_cast<int>(ParallaxLayerId::Close)].draw(renderstate, renderstate.render_target, worldcoords, viewport, metrics);
    parallax_layers[static_cast<int>(ParallaxLayerId::Near)].draw(renderstate, renderstate.render_target, worldcoords, viewport, metrics);

    drawDebug(worldcoords, viewport, player);
    // Copy final render target, which is 4k, into viewport on swapchain, which may be smaller or bigger
    SDL_FRect destRect = viewport.getRenderRect();
    const ppl7::grafix::Rect& v = viewport.getViewport();
    if (v.y1 > 0) {
        if (destRect.y < v.y1) destRect.y += v.y1;
    }
    if (v.x1 > 0) {
        if (destRect.x < v.x1) destRect.x += v.x1;
    }

    copyRenderTargetToSwapchain(cmdbuf, swapchainTexture, destRect);
}

void Level::drawDebug(const Camera& camera, const GameViewport& viewport, const Player* player)
{
    renderstate.batcher->startRenderPass();
    if (player && bShowCollisions) player->drawCollision(*renderstate.batcher, viewport, camera);
    camera.draw(*renderstate.batcher, viewport);

    renderstate.batcher->prepareInstanceData(renderstate.cmdbuf);
    SDL_GPUColorTargetInfo colorTargetInfo = {0};
    colorTargetInfo.texture = renderstate.render_target;
    colorTargetInfo.clear_color = (SDL_FColor){0.0f, 0.0f, 0.0f, 0.0f}; // Black background
    colorTargetInfo.load_op = SDL_GPU_LOADOP_LOAD;
    colorTargetInfo.store_op = SDL_GPU_STOREOP_STORE;
    colorTargetInfo.cycle = false; // CRITICAL: SDL examples use false!

    SDL_GPUDepthStencilTargetInfo depthTargetInfo = {0};
    depthTargetInfo.texture = renderstate.depth_buffer;
    depthTargetInfo.clear_depth = 1.0f;
    depthTargetInfo.load_op = SDL_GPU_LOADOP_CLEAR;
    depthTargetInfo.store_op = SDL_GPU_STOREOP_DONT_CARE;
    depthTargetInfo.stencil_load_op = SDL_GPU_LOADOP_DONT_CARE;
    depthTargetInfo.stencil_store_op = SDL_GPU_STOREOP_DONT_CARE;
    depthTargetInfo.cycle = false;

    SDL_GPURenderPass* renderPass = SDL_BeginGPURenderPass(renderstate.cmdbuf, &colorTargetInfo, 1, &depthTargetInfo);
    // SDL_GPURenderPass* renderPass = SDL_BeginGPURenderPass(renderstate.cmdbuf, &colorTargetInfo, 1, NULL);
    SDL_SetGPUViewport(renderPass, NULL);
    SDL_SetGPUScissor(renderPass, NULL);

    renderstate.batcher->endRenderPass(renderstate.cmdbuf, renderPass);
    SDL_EndGPURenderPass(renderPass);
}

ppl7::grafix::Image Level::getScreenshot(int width, int height)
{
    if (!renderstate.gpu || !renderstate.render_target) return ppl7::grafix::Image();
    ppl7::grafix::Image img;

    // 1. Kleine temporäre Target-Textur erstellen
    SDL_GPUTexture* thumbTex = renderstate.gpu->createRenderTarget(width, height);

    SDL_GPUCommandBuffer* cmdbuf = SDL_AcquireGPUCommandBuffer(renderstate.gpu->gpu);

    // 2. In die kleine Textur rendern (Skalierung)
    SDL_GPUColorTargetInfo targetInfo = {0};
    targetInfo.texture = thumbTex;
    targetInfo.load_op = SDL_GPU_LOADOP_CLEAR;
    targetInfo.store_op = SDL_GPU_STOREOP_STORE;

    SDL_GPURenderPass* pass = SDL_BeginGPURenderPass(cmdbuf, &targetInfo, 1, NULL);

    // Viewport auf die kleine Größe setzen
    SDL_GPUViewport viewport = {0, 0, (float)width, (float)height, 0, 1};
    SDL_SetGPUViewport(pass, &viewport);

    SDL_BindGPUGraphicsPipeline(pass, renderstate.renderpipelines->copyPipeline);
    SDL_GPUTextureSamplerBinding binding = {.texture = renderstate.render_target, .sampler = renderstate.renderpipelines->samplerClamp};
    SDL_BindGPUFragmentSamplers(pass, 0, &binding, 1);

    SDL_DrawGPUPrimitives(pass, 3, 1, 0, 0);
    SDL_EndGPURenderPass(pass);
    SDL_SubmitGPUCommandBuffer(cmdbuf);

    renderstate.gpu->downloadTexture(thumbTex, width, height, img);
    renderstate.gpu->destroyGPUTexture(thumbTex);
    return img;
}

void Level::update(const GameClock& clock,
                   Metrics& metrics,
                   const ppl7::grafix::PointF& worldcoords,
                   const ppl7::grafix::Size& render_target_size)
{
    metrics.time_update_total.start();
    // Objects
    metrics.time_update_objects.start();
    for (auto& layer : parallax_layers) {
        layer.updateObjects(clock, worldcoords * layer.size_factor * layer.speed_factor, render_target_size);
    }
    metrics.time_update_objects.stop();

    // Sprites
    metrics.time_update_sprites.start();
    for (auto& layer : parallax_layers) {
        layer.updateSprites(clock, worldcoords * layer.size_factor * layer.speed_factor, render_target_size);
    }
    metrics.time_update_sprites.stop();

    // Particles
    metrics.time_update_particles.start();
    for (auto& layer : parallax_layers) {
        // TODO:layer.updateParticles(clock, worldcoords / layer.size_factor, render_target_size / layer.size_factor);
    }
    metrics.time_update_particles.stop();

    // Lights
    metrics.time_update_lights.start();
    for (auto& layer : parallax_layers) {
        // TODO:layer.updateLights(clock, worldcoords / layer.size_factor, render_target_size / layer.size_factor);
    }
    metrics.time_update_lights.stop();
    metrics.time_update_total.stop();
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

size_t Level::countLights() const
{
    size_t total = 0;
    for (auto& layer : parallax_layers) {
        // total += layer.lights.count();
    }
    return total;
}

size_t Level::countVisibleLights() const
{
    size_t total = 0;
    for (auto& layer : parallax_layers) {
        if (!layer.tiles.isVisible()) continue;
        // total += layer.lights.count();
    }
    return total;
}

size_t Level::countObjects() const
{
    size_t total = 0;
    for (auto& layer : parallax_layers) {
        total += layer.objects.count();
    }
    return total;
}

size_t Level::countVisibleObjects() const
{
    size_t total = 0;
    for (auto& layer : parallax_layers) {
        if (!layer.tiles.isVisible()) continue;
        total += layer.objects.countVisible();
    }
    return total;
}

size_t Level::countParticles() const
{
    size_t total = 0;
    for (auto& layer : parallax_layers) {
        total += layer.particles.count();
    }
    return total;
}

size_t Level::countVisibleParticles() const
{
    size_t total = 0;
    for (auto& layer : parallax_layers) {
        if (!layer.tiles.isVisible()) continue;
        total += layer.particles.countVisible();
    }
    return total;
}

bool Level::findSprite(const ppl7::grafix::Point& p,
                       const ppl7::grafix::Point& worldcoords,
                       SpriteSystem::Item& item,
                       ParallaxLayerId& parallax_layer,
                       ParallaxLayer::SpritePosition& layer_position) const
{
    for (auto& layer : parallax_layers) {
        if (!layer.isVisible) continue;
        ppl7::grafix::Point coords = p + worldcoords * layer.speed_factor * layer.size_factor;
        if (layer.front_sprites.findMatchingSprite(coords, item)) {
            parallax_layer = layer.myParallaxLayer;
            layer_position = ParallaxLayer::SpritePosition::Front;
            return true;
        }
        if (layer.background_sprites.findMatchingSprite(coords, item)) {
            parallax_layer = layer.myParallaxLayer;
            layer_position = ParallaxLayer::SpritePosition::Background;
            return true;
        }
    }
    return false;
}

size_t Level::tileCount() const
{
    size_t count = 0;
    for (auto& layer : parallax_layers) {
        count += layer.tiles.tileCount();
    }
    return count;
}

Objects::Object* Level::getObject(uint32_t object_id) const
{
    ParallaxLayerId layer = static_cast<ParallaxLayerId>((object_id >> 24) & 0x7f);
    return parallax_layers[static_cast<int>(layer)].objects.getObject(object_id);
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

Objects::Object* Level::findMatchingObject(const ppl7::grafix::Point& worldcoords, const ppl7::grafix::Point& p) const
{
    for (auto& layer : parallax_layers) {
        if (!layer.isVisible) continue;
        ppl7::grafix::Point coords = p + worldcoords * layer.speed_factor * layer.size_factor;
        Objects::Object* obj = layer.objects.findMatchingObject(coords);
        if (obj) return obj;
    }
    return nullptr;
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