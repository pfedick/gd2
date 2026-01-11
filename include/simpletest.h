#ifndef SIMPLETEST_H
#define SIMPLETEST_H

#include "gpu.h"
#include "sprite.h"

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
    int screenWidth;
    int screenHeight;

    // Second sprite from SpriteTexture
    SpriteTexture spriteTexture;
    SDL_GPUBuffer* spriteVertexBuffer;
    SDL_GPUBuffer* spriteIndexBuffer;
    int currentSpriteId;

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
    void loadSprite();
    void createSpriteBuffers();

    // Helper methods for CPU-side coordinate transformation
    float pixelToNDC_X(float pixelX) const;
    float pixelToNDC_Y(float pixelY) const;
};

#endif // SIMPLETEST_H
