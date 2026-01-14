#ifndef INCLUDE_RENDERPIPELINES_H_
#define INCLUDE_RENDERPIPELINES_H_

#include <SDL3/SDL.h>
#include <ppl7.h>
#include <ppl7-grafix.h>


class RenderPipelines
{
private:
    SDL_GPUDevice* gpu_device;
    SDL_Window* window;

    SDL_GPUShader* blurHorizontalShader;
    SDL_GPUShader* blurVerticalShader;
    SDL_GPUShader* copyShader;
    SDL_GPUShader* vertexShader;

    SDL_GPUShader* loadShader(const ppl7::String& filename, SDL_GPUShaderStage stage, int num_samplers, int num_storage_textures, int num_storage_buffers, int num_uniform_buffers);
    void releaseShader(SDL_GPUShader* shader);

    void loadShaders();
    void createPipelines();
    void createSamplers();

public:
    struct BlurUniforms {
        float blurStrength;
        float texelSizeX;
        float texelSizeY;
        float padding;  // Align auf 16 Bytes
    };

    SDL_GPUGraphicsPipeline* blurHorizontalPipeline;
    SDL_GPUGraphicsPipeline* blurVerticalPipeline;
    SDL_GPUGraphicsPipeline* copyPipeline;
    SDL_GPUGraphicsPipeline* uiPipeline;
    SDL_GPUSampler* samplerClamp;

    RenderPipelines();
    ~RenderPipelines();
    void init(SDL_GPUDevice* gpu, SDL_Window* window);
    SDL_GPUDevice* getGPUDevice();




};

#endif // INCLUDE_RENDERPIPELINES_H_