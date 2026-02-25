#ifndef INCLUDE_GAMERENDERER_H
#define INCLUDE_GAMERENDERER_H

#include <SDL3/SDL.h>

#include <ppl7.h>
#include <ppl7-grafix.h>
#include "gpu.h"

class SpriteTexture;
class GPUContext;

class GameRenderer;
class GPUBatcher
{
    friend class GameRenderer;

private:
    struct UniformData
    {
        float projection[16]; // 4x4 matrix
        float view[16];       // 4x4 matrix
    };

    float z;
    GPUContext* gpu;
    int screenWidth;
    int screenHeight;

    SDL_GPUShader* fragShader;
    SDL_GPUShader* vertShader;
    SDL_GPUGraphicsPipeline* spritePipeline;
    SDL_GPUGraphicsPipeline* spriteOutlinePipeline;
    SDL_GPUShader* outlineFragShader;

    SDL_GPUShader* primitiveVertShader;
    SDL_GPUShader* primitiveFragShader;
    SDL_GPUGraphicsPipeline* primitivePipeline;     // LINELIST
    SDL_GPUGraphicsPipeline* primitiveFillPipeline; // TRIANGLELIST

    SDL_GPUSampler* sampler;
    SDL_GPUBuffer* vertexBuffer;
    SDL_GPUBuffer* indexBuffer;
    SDL_GPUBuffer* storageBuffer; // Storage buffer for sprite instances
    Uint32 storageBufferCapacity; // Current capacity in bytes

    SDL_GPUBuffer* primitiveVertexBuffer;
    Uint32 primitiveVertexCapacity;

    SDL_GPUBuffer* uniformBuffer; // Not used - push constants instead
    UniformData currentUniforms;  // Current projection/view matrices

    struct Vertex
    {
        float x, y;       // Position
        float u, v;       // Texcoords
        float r, g, b, a; // Color
    };

    struct PrimitiveVertex
    {
        float x, y, z;
        float r, g, b, a;
    };

    struct SpriteInstance
    {
        float pos_x, pos_y;                       // Sprite position (NDC) center/pivot
        float m00, m01;                           // Transform Matrix Col 1
        float m10, m11;                           // Transform Matrix Col 2
        float pos_z;                              // Z-index from addSprite
        float pad;                                // PADDING to 16-byte boundary
        float uv_x, uv_y, uv_w, uv_h;             // UV rect (normalized 0-1)
        float u_min, v_min, u_max, v_max;         // UV boundaries (clipping for outlines)
        float pivot_x, pivot_y;                   // Pivot point (0..1)
        float offset_x, offset_y;                 // Unused
        float color_r, color_g, color_b, color_a; // Color Modulation
    };

    enum class BatchType
    {
        None,
        Sprites,
        PrimitiveLines,
        PrimitiveTriangles
    };

    struct RenderBatch
    {
        BatchType type;
        uint32_t offset;
        uint32_t count;
        SDL_GPUTexture* texture;
        bool outline;
    };

    std::vector<SpriteInstance> spriteInstances;
    std::vector<PrimitiveVertex> primitiveVertices;
    std::vector<RenderBatch> batches;

    void loadShaders();
    void createPipeline();
    void createBuffers();
    void uploadStaticQuadData();
    void cleanup();
    void bindTexture(SDL_GPURenderPass* render_pass, SDL_GPUTexture* texture);
    void finishCurrentBatch();
    void addSpriteInternal(const SpriteTexture& sprite,
                           int id,
                           float x,
                           float y,
                           float scale_x,
                           float scale_y,
                           float angle,
                           const ppl7::grafix::Color& color,
                           bool outline);

public:
    size_t contextSwitchCount; // For debugging: Count how many times we switch GPU context (render pass)
    size_t totalSpriteCount;
    size_t totalPrimitivesCount;

    GPUBatcher();
    ~GPUBatcher();
    void init(GPUContext* gpu, SDL_GPUSampler* sampler);

    void setLogicalRenderSize(int screenWidth, int screenHeight);
    void setLogicalRenderSize(const ppl7::grafix::Size& size);

    void startRenderPass();
    void prepareInstanceData(SDL_GPUCommandBuffer* cmd); // Upload instance data before render pass
    void endRenderPass(SDL_GPUCommandBuffer* cmd, SDL_GPURenderPass* render_pass);

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

SDL_FColor toSDLFColor(const ppl7::grafix::Color& color);
SDL_FColor toSDLFPMAColor(const ppl7::grafix::Color& color);

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
    class Metrics
    {
    public:
        size_t totalSpritesDrawn = 0;
        size_t totalPrimitivesDrawn = 0;
        size_t contextSwitches = 0;
    };

    GPUContext* gpu;

    SDL_GPUTexture* render_target;
    SDL_GPUTexture* render_layer;
    SDL_GPUTexture* render_lightmap;
    SDL_GPUTexture* blur_temp;
    SDL_GPUTexture* render_normal;

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
    void resetMetrics();
    Metrics getMetrics() const;

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
