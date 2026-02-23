#ifndef INCLUDE_RENDERPIPELINES_H_
#define INCLUDE_RENDERPIPELINES_H_

#include <SDL3/SDL.h>
#include <ppl7.h>
#include <ppl7-grafix.h>

class GPUContext;
class RenderPipelines
{
private:
    GPUContext* gpu;
    SDL_Window* window;

    SDL_GPUShader* blurHorizontalShader;
    SDL_GPUShader* blurVerticalShader;
    SDL_GPUShader* copyShader;
    SDL_GPUShader* vertexShader;

    void loadShaders();
    void createPipelines();
    void createSamplers();

public:
    SDL_GPUGraphicsPipeline* blurHorizontalPipeline;
    SDL_GPUGraphicsPipeline* blurVerticalPipeline;
    SDL_GPUGraphicsPipeline* copyPipeline;
    SDL_GPUGraphicsPipeline* uiPipeline;
    SDL_GPUSampler* samplerClamp;

    RenderPipelines();
    ~RenderPipelines();
    void init(GPUContext& gpu, SDL_Window* window);
    SDL_GPUDevice* getGPUDevice();
};

#endif // INCLUDE_RENDERPIPELINES_H_