#ifndef INCLUDE_GAMERENDERER_H
#define INCLUDE_GAMERENDERER_H

#include <SDL3/SDL.h>

#include <ppl7.h>
#include <ppl7-grafix.h>
#include "gpu.h"

class SpriteTexture;
class GPUContext;

class GameRenderer
{
private:
    SDL_Window* window;

    SDL_GPUCommandBuffer* cmdbuf;
    SDL_GPUTexture* swapchainTexture;

    SDL_GPUShader* blurHorizontalShader;
    SDL_GPUShader* blurVerticalShader;
    SDL_GPUShader* copyShader;
    SDL_GPUShader* vertexShader;

    SDL_GPUGraphicsPipeline* blurHorizontalPipeline;
    SDL_GPUGraphicsPipeline* blurVerticalPipeline;
    SDL_GPUGraphicsPipeline* copyPipeline;
    SDL_GPUGraphicsPipeline* copyWithAlphablendingPipeline;
    SDL_GPUSampler* samplerClamp;

    ppl7::grafix::Size render_target_size;

    void loadShaders();
    void createPipelines();
    void createSamplers();

    GPUBatcher batcher;

public:
    GPUContext* gpu;

    SDL_GPUTexture* render_target;
    SDL_GPUTexture* render_layer;
    SDL_GPUTexture* render_lightmap;
    SDL_GPUTexture* blur_temp;
    SDL_GPUTexture* render_normal;
    SDL_GPUTexture* depth_buffer;

    GameRenderer();
    ~GameRenderer();

    void init(GPUContext& gpu, SDL_Window* window);

    void resizeRenderBuffer(const ppl7::grafix::Size& size);

    bool accuireGPUCommandBuffer();
    void submitGPUCommandBuffer();
    SDL_GPUCommandBuffer* getCommandBuffer();
    SDL_GPUTexture* getSwapchainTexture();

    void clearTexture(SDL_GPUTexture* texture, const ppl7::grafix::Color& color);
    void copyTexture(SDL_GPUTexture* source, SDL_GPUTexture* target, bool alphablend = true);
    void copyTextureToSwapchain(SDL_GPUTexture* source,
                                const SDL_FRect& destRect); // Clears swapchain and copies source to destRect inside the swapchain
    void blur(SDL_GPUTexture* source, SDL_GPUTexture* target, float blur_factor); // source and target can be the same

    ppl7::grafix::Image getScreenshot(int width, int height);

    // Draw functions
    void setLogicalRenderSize(int screenWidth, int screenHeight);
    void setLogicalRenderSize(const ppl7::grafix::Size& size);

    void startRenderPass();
    void endRenderPass(SDL_GPUTexture* target_texture,
                       SDL_GPULoadOp loadOp = SDL_GPU_LOADOP_CLEAR,
                       const ppl7::grafix::Color& clearColor = ppl7::grafix::Color(0, 0, 0, 0));

    void addSprite(const SpriteTexture& sprite,
                   int id,
                   float x,
                   float y,
                   float scale_x = 1.0f,
                   float scale_y = 1.0f,
                   float angle = 0.0f,
                   const ppl7::grafix::Color& color_modulation = ppl7::grafix::Color(255, 255, 255, 255));
    void addSpriteOutline(const SpriteTexture& sprite,
                          int id,
                          float x,
                          float y,
                          float scale_x = 1.0f,
                          float scale_y = 1.0f,
                          float angle = 0.0f,
                          const ppl7::grafix::Color& color_modulation = ppl7::grafix::Color(255, 255, 255, 255));

    void addLine(float x1, float y1, float x2, float y2, const ppl7::grafix::Color& color, int thickness = 1);
    void addRect(float x, float y, float w, float h, const ppl7::grafix::Color& color, int thickness = 1);
    void addFilledRect(float x, float y, float w, float h, const ppl7::grafix::Color& color);
};

#endif // INCLUDE_GAMERENDERER_H
