#ifndef INCLUDE_GAMERENDERER_H
#define INCLUDE_GAMERENDERER_H

#include <SDL3/SDL.h>

#include <ppl7.h>
#include <ppl7-grafix.h>

class SpriteTexture;
class GPUContext;

class GameRenderer
{
private:
    GPUContext* gpu;
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

    SDL_GPUTexture* render_target;
    SDL_GPUTexture* render_layer;
    SDL_GPUTexture* render_lightmap;
    SDL_GPUTexture* blur_temp;
    SDL_GPUTexture* render_normal;
    SDL_GPUTexture* depth_buffer;

    ppl7::grafix::Size render_target_size;

    void loadShaders();
    void createPipelines();
    void createSamplers();

public:
    GameRenderer();
    ~GameRenderer();

    void init(GPUContext& gpu, SDL_Window* window);

    void resizeRenderBuffer(const ppl7::grafix::Size& size);

    void accuireGPUCommandBuffer();
    void submitGPUCommandBuffer();
    SDL_GPUCommandBuffer* getCommandBuffer();
    SDL_GPUTexture* getSwapchainTexture();

    void copyTexture(SDL_GPUTexture* source, SDL_GPUTexture* target, bool alphablend = true);
    void copyTextureToSwapchain(SDL_GPUTexture* source,
                                const SDL_FRect& destRect); // Clears swapchain and copies source to destRect inside the swapchain
    void blur(SDL_GPUTexture* source, SDL_GPUTexture* target, float blur_factor); // source and target can be the same
    void setRenderTarget(SDL_GPUTexture* texture);
    void beginRenderPass();
    void endRenderPass();
    void drawSprite(const SpriteTexture& sprite,
                    int sprite_id,
                    float x,
                    float y,
                    float scale_x = 1.0f,
                    float scale_y = 1.0f,
                    float angle = 0.0f,
                    const ppl7::grafix::Color& color_modulation = ppl7::grafix::Color(255, 255, 255, 255));
    void addSpriteOutline(const SpriteTexture& sprite,
                          int sprite_id,
                          float x,
                          float y,
                          float scale_x = 1.0f,
                          float scale_y = 1.0f,
                          float angle = 0.0f,
                          const ppl7::grafix::Color& color_modulation = ppl7::grafix::Color(255, 255, 255, 255));
    void addLine(float x1, float y1, float x2, float y2, const ppl7::grafix::Color& color, float thickness = 1.0f);
    void addRect(float x, float y, float w, float h, const ppl7::grafix::Color& color, float thickness = 1.0f);
    void addFilledRect(float x, float y, float w, float h, const ppl7::grafix::Color& color);
};

#endif // INCLUDE_GAMERENDERER_H
