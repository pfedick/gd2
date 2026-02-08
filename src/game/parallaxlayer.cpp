#include "game.h"
#include "level.h"
#include "gpu.h"
#include "constants.h"
#include "player.h"

ParallaxLayer::ParallaxLayer()
{
    blur_factor = 0.0f;
    speed_factor = 1.0f;
    size_factor = 1.0f;
    isVisible = true;
    bShowGrid = false;
    bShowTileTypes = false;
    player = NULL;
}

ParallaxLayer::~ParallaxLayer()
{
}

void ParallaxLayer::init(ParallaxLayerId parallaxLayer, float blur, float speed, float size)
{
    this->myParallaxLayer = parallaxLayer;
    blur_factor = blur;
    speed_factor = speed;
    size_factor = size;
    objects.init(parallaxLayer);
}

void ParallaxLayer::clear()
{
    front_sprites.clear();
    background_sprites.clear();
    tiles.clear();
}

void ParallaxLayer::updateSprites(const GameClock& clock,
                                  const ppl7::grafix::PointF& worldcoords,
                                  const ppl7::grafix::Size& render_target_size)
{
    background_sprites.updateVisibleSpriteList(worldcoords, render_target_size);
    front_sprites.updateVisibleSpriteList(worldcoords, render_target_size);
}

void ParallaxLayer::updateObjects(const GameClock& clock,
                                  const ppl7::grafix::PointF& worldcoords,
                                  const ppl7::grafix::Size& render_target_size)
{
    objects.updateVisibleObjectList(worldcoords, render_target_size);
    if (player) objects.update(clock, TileTypeMatrix, *player);
}
void ParallaxLayer::updateParticles(const GameClock& clock,
                                    const ppl7::grafix::PointF& worldcoords,
                                    const ppl7::grafix::Size& render_target_size)
{
    // particles.update(clock, worldcoords, render_target_size);
}
void ParallaxLayer::updateLights(const GameClock& clock,
                                 const ppl7::grafix::PointF& worldcoords,
                                 const ppl7::grafix::Size& render_target_size)
{
}

bool ParallaxLayer::hasVisibleGrafix() const
{
    return true;
}

void ParallaxLayer::setVisible(bool visible)
{
    isVisible = visible;
}

void ParallaxLayer::setPlayer(Player* p)
{
    player = p;
}

void ParallaxLayer::draw(RenderState& renderstate,
                         SDL_GPUTexture* render_target,
                         const ppl7::grafix::PointF& worldcoords,
                         const GameViewport& viewport)
{
    if (!isVisible) return;
    ppl7::grafix::PointF parallax_worldcoords = worldcoords * speed_factor * size_factor;
    if (hasVisibleGrafix()) {
    }
    // renderstate.batcher->startRenderPass();
    renderstate.batcher->startRenderPass();
    tiles.draw(*renderstate.batcher, viewport, parallax_worldcoords, size_factor);

    if (myParallaxLayer == ParallaxLayerId::Player && player != NULL) {
        player->draw(*renderstate.batcher, viewport, parallax_worldcoords, size_factor);
    }

    if (bShowGrid) {
        drawTileGrid(renderstate, parallax_worldcoords, viewport);
    }
    if (bShowTileTypes) {
        TileTypeMatrix.draw(*renderstate.batcher, viewport, parallax_worldcoords, size_factor);
    }
    //  background_sprites.draw(batcher, cmdbuf, swapchainTexture, worldcoords, viewport
    renderstate.batcher->prepareInstanceData(renderstate.cmdbuf);

    SDL_GPUColorTargetInfo colorTargetInfo = {0};
    colorTargetInfo.texture = renderstate.render_layer;
    colorTargetInfo.clear_color = (SDL_FColor){0.0f, 0.0f, 0.0f, 0.0f}; // Black background
    colorTargetInfo.load_op = SDL_GPU_LOADOP_CLEAR;
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

    // Post-Processing: Blur
    if (blur_factor > 0.0f) {
        blur(renderstate, renderstate.render_layer);
    }
    copyLayerToTarget(renderstate, renderstate.render_layer, render_target);
}

void ParallaxLayer::drawTileGrid(RenderState& renderstate, const ppl7::grafix::PointF& worldcoords, const GameViewport& viewport)
{
    ppl7::grafix::Color grid_color(255, 255, 255, 128);
    ppl7::grafix::Color grid_shadow(0, 0, 0, 128);
    float tile_width = viewport.tileWidth() * size_factor;
    float tile_height = viewport.tileHeight() * size_factor;
    // ppl7::PrintDebug("Drawing tile grid...\n");

    int start_x = static_cast<int>(worldcoords.x / tile_width);
    int start_y = static_cast<int>(worldcoords.y / tile_height);

    float offset_x = worldcoords.x - (start_x * tile_width);
    float offset_y = worldcoords.y - (start_y * tile_height);

    start_x = -offset_x;
    start_y = -offset_y;
    // renderstate.batcher->addLine(100, 100, 200, 200, ppl7::grafix::Color(255, 0, 0, 255), 10.0f);
    // renderstate.batcher->addRect(300, 100, 50, 50, ppl7::grafix::Color(0, 255, 0, 255), 10.0f);
    // renderstate.batcher->addFilledRect(400, 100, 50, 50, ppl7::grafix::Color(0, 0, 255, 255));

    for (float x = start_x; x < viewport.width(); x += tile_width) {
        renderstate.batcher->addLine(x + 2, 0, x + 2, viewport.height(), grid_shadow, 2.0f);
        renderstate.batcher->addLine(x, 0, x, viewport.height(), grid_color, 2.0f);
    }

    for (float y = start_y; y < viewport.height(); y += tile_height) {
        renderstate.batcher->addLine(0, y + 2, viewport.width(), y + 2, grid_shadow, 2.0f);
        renderstate.batcher->addLine(0, y, viewport.width(), y, grid_color, 2.0f);
    }
}

struct BlurParams
{
    float blurStrength;
    float padding; // WICHTIG: 4 Bytes Füllmaterial für std140 Alignment
    float texelSizeX;
    float texelSizeY;
};

void ParallaxLayer::blur(RenderState& renderstate, SDL_GPUTexture* target_texture)
{
    SDL_GPUColorTargetInfo targetInfo = {};
    SDL_GPUTextureSamplerBinding binding = {};

    BlurParams params;
    float scale = (float)renderstate.render_target_size.width / 3840.0f;
    params.blurStrength = blur_factor * scale;
    params.padding = 0.0f;                                                   // Egal was hier steht
    params.texelSizeX = 1.0f / (float)renderstate.render_target_size.width;  // Breite der Textur
    params.texelSizeY = 1.0f / (float)renderstate.render_target_size.height; // Höhe der Textur

    // Slot Index 0 (passend zu binding = 0 im Shader, Set 3 ist implizit für Fragment Uniforms)
    SDL_PushGPUFragmentUniformData(renderstate.cmdbuf, 0, &params, sizeof(BlurParams));

    targetInfo.texture = renderstate.blur_temp; // Ziel: Temp Textur
    targetInfo.load_op = SDL_GPU_LOADOP_DONT_CARE;
    targetInfo.store_op = SDL_GPU_STOREOP_STORE;

    SDL_GPURenderPass* renderPass = SDL_BeginGPURenderPass(renderstate.cmdbuf, &targetInfo, 1, NULL);
    SDL_SetGPUViewport(renderPass, NULL);
    SDL_SetGPUScissor(renderPass, NULL);

    SDL_BindGPUGraphicsPipeline(renderPass, renderstate.renderpipelines->blurHorizontalPipeline);
    binding.texture = target_texture;
    binding.sampler = renderstate.renderpipelines->samplerClamp; // Einen Clamp-Sampler benutzen!
    SDL_BindGPUFragmentSamplers(renderPass, 0, &binding, 1);

    // Fullscreen Triangle zeichnen (3 Vertices, Shader generiert Coords)
    SDL_DrawGPUPrimitives(renderPass, 3, 1, 0, 0);

    SDL_EndGPURenderPass(renderPass);

    targetInfo.texture = target_texture;
    targetInfo.load_op = SDL_GPU_LOADOP_DONT_CARE; // Wir überschreiben alles
    targetInfo.store_op = SDL_GPU_STOREOP_STORE;

    renderPass = SDL_BeginGPURenderPass(renderstate.cmdbuf, &targetInfo, 1, NULL);
    SDL_SetGPUViewport(renderPass, NULL);
    SDL_SetGPUScissor(renderPass, NULL);

    SDL_BindGPUGraphicsPipeline(renderPass, renderstate.renderpipelines->blurVerticalPipeline);

    // Eingabe-Textur binden (das Bild aus Pass 2)
    binding.texture = renderstate.blur_temp;
    binding.sampler = renderstate.renderpipelines->samplerClamp;
    SDL_BindGPUFragmentSamplers(renderPass, 0, &binding, 1);

    SDL_DrawGPUPrimitives(renderPass, 3, 1, 0, 0);
    SDL_EndGPURenderPass(renderPass);
}

void ParallaxLayer::copyLayerToTarget(RenderState& renderstate, SDL_GPUTexture* source, SDL_GPUTexture* target)
{
    SDL_GPUColorTargetInfo targetInfo = {};
    SDL_GPUTextureSamplerBinding binding = {};
    targetInfo.texture = target;
    targetInfo.load_op = SDL_GPU_LOADOP_LOAD;
    targetInfo.store_op = SDL_GPU_STOREOP_STORE;

    SDL_GPURenderPass* renderPass = SDL_BeginGPURenderPass(renderstate.cmdbuf, &targetInfo, 1, NULL);
    SDL_SetGPUViewport(renderPass, NULL);
    SDL_SetGPUScissor(renderPass, NULL);

    SDL_BindGPUGraphicsPipeline(renderPass, renderstate.renderpipelines->uiPipeline);
    binding.texture = source;
    binding.sampler = renderstate.renderpipelines->samplerClamp; // Einen Clamp-Sampler benutzen!
    SDL_BindGPUFragmentSamplers(renderPass, 0, &binding, 1);

    // Fullscreen Triangle zeichnen (3 Vertices, Shader generiert Coords)
    SDL_DrawGPUPrimitives(renderPass, 3, 1, 0, 0);

    SDL_EndGPURenderPass(renderPass);
}