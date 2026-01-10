#ifndef SIMPLETEST_H
#define SIMPLETEST_H

#include "gpu.h"

class SimpleQuadTest
{
private:
    GPUContext* gpu;
    SDL_GPUGraphicsPipeline* pipeline;
    SDL_GPUShader* vertShader;
    SDL_GPUShader* fragShader;
    SDL_GPUBuffer* vertexBuffer;
    SDL_GPUBuffer* indexBuffer;
    SDL_GPUSampler* sampler;
    SDL_GPUTexture* testTexture;
    float projection[16];

public:
    SimpleQuadTest();
    ~SimpleQuadTest();
    void init(GPUContext* gpuCtx, int screenWidth, int screenHeight);
    void draw(SDL_GPUCommandBuffer* cmd, SDL_GPURenderPass* renderPass);
    void cleanup();

private:
    void loadShaders();
    void createPipeline();
    void createBuffers();
    void loadTestTexture();
};

#endif // SIMPLETEST_H
