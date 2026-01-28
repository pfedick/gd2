#include "game.h"
#include "level.h"
#include "gpu.h"

ParallaxLayer::ParallaxLayer()
{
    blur_factor = 0.0f;
    speed_factor = 1.0f;
    size_factor = 1.0f;
    isVisible = true;
    bShowGrid = false;
}

ParallaxLayer::~ParallaxLayer()
{
}

void ParallaxLayer::init(ParallaxLayerId layerType, float blur, float speed, float size)
{
    this->layerType = layerType;
    blur_factor = blur;
    speed_factor = speed;
    size_factor = size;
}

void ParallaxLayer::clear()
{
    front_sprites.clear();
    background_sprites.clear();
    tiles.clear();
}

void ParallaxLayer::updateVisibleObjects(const ppl7::grafix::PointF& worldcoords, const ppl7::grafix::Rect& viewport)
{
    background_sprites.updateVisibleSpriteList(worldcoords, viewport);
    front_sprites.updateVisibleSpriteList(worldcoords, viewport);
}

bool ParallaxLayer::hasVisibleGrafix() const
{
    return true;
}

void ParallaxLayer::showGrid(bool enable)
{
    bShowGrid = enable;
}

void ParallaxLayer::draw(RenderState& renderstate,
                         SDL_GPUTexture* swapchainTexture,
                         const ppl7::grafix::PointF& worldcoords,
                         const ppl7::grafix::Rect& viewport)
{
    if (!isVisible) return;
    ppl7::grafix::PointF parallax_worldcoords = worldcoords * speed_factor * size_factor;
    if (hasVisibleGrafix()) {
    }
    // renderstate.batcher->startRenderPass();

    if (bShowGrid) {
        drawTileGrid(renderstate, swapchainTexture, parallax_worldcoords, viewport);
    }

    // background_sprites.draw(batcher, cmdbuf, swapchainTexture, worldcoords, viewport
}

void ParallaxLayer::drawTileGrid(RenderState& renderstate,
                                 SDL_GPUTexture* target_texture,
                                 const ppl7::grafix::PointF& worldcoords,
                                 const ppl7::grafix::Rect& viewport)
{
    ppl7::grafix::Color grid_color(255, 255, 255, 64);
    float tile_width = 32 * size_factor;
    float tile_height = 32 * size_factor;
    // ppl7::PrintDebug("Drawing tile grid...\n");

    float start_x = static_cast<int>(worldcoords.x / tile_width) * tile_width - worldcoords.x;
    float start_y = static_cast<int>(worldcoords.y / tile_height) * tile_height - worldcoords.y;

    renderstate.batcher->startRenderPass();

    // renderstate.batcher->addLine(100, 100, 200, 200, ppl7::grafix::Color(255, 0, 0, 255), 10.0f);
    // renderstate.batcher->addRect(300, 100, 50, 50, ppl7::grafix::Color(0, 255, 0, 255), 10.0f);
    // renderstate.batcher->addFilledRect(400, 100, 50, 50, ppl7::grafix::Color(0, 0, 255, 255));

    for (float x = start_x; x < viewport.width(); x += tile_width) {
        renderstate.batcher->addLine(x, 0, x, viewport.height(), grid_color, 1.0f);
    }

    for (float y = start_y; y < viewport.height(); y += tile_height) {
        renderstate.batcher->addLine(0, y, viewport.width(), y, grid_color, 1.0f);
    }

    renderstate.batcher->prepareInstanceData(renderstate.cmdbuf);

    SDL_GPUColorTargetInfo colorTargetInfo = {0};
    colorTargetInfo.texture = target_texture;
    colorTargetInfo.clear_color = (SDL_FColor){0.0f, 0.0f, 0.4f, 1.0f}; // Black background
    colorTargetInfo.load_op = SDL_GPU_LOADOP_LOAD;
    colorTargetInfo.store_op = SDL_GPU_STOREOP_STORE;
    colorTargetInfo.cycle = false; // CRITICAL: SDL examples use false!

    SDL_GPUDepthStencilTargetInfo depthTargetInfo = {0};
    depthTargetInfo.texture = renderstate.tex_depth_buffer;
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