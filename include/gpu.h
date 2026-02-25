#ifndef INCLUDE_GPU_H
#define INCLUDE_GPU_H

#include <SDL3/SDL.h>

#include <ppl7.h>
#include <ppl7-grafix.h>

class SpriteTexture;

class GPUException : public ppl7::Exception
{
public:
    using ppl7::Exception::Exception;

    GPUException(const char* msg, ...) noexcept
    {
        va_list args;
        va_start(args, msg);
        copyText(msg, args);
        va_end(args);
    }

    const char* what() const noexcept override
    {
        return "GPUException";
    }
};

class GPUStreamingTexture
{
private:
    SDL_GPUDevice* gpu;
    SDL_GPUTexture* texture;
    SDL_GPUTransferBuffer* transfer_buffer;
    ppl7::grafix::Size size;
    bool needs_update;

public:
    GPUStreamingTexture(SDL_GPUDevice* gpu, int width, int height);
    ~GPUStreamingTexture();
    ppl7::grafix::Drawable lock();
    void unlock();
    ppl7::grafix::Size getSize() const;
    void resize(int width, int height);
    void updateTexture(SDL_GPUCommandBuffer* cmdbuf);
    SDL_GPUTexture* getTexture() const;
};

class GPUContext
{
public:
    SDL_GPUDevice* gpu;
    SDL_Window* window;

    GPUContext();
    ~GPUContext();

    void initializeGPUDevice();
    void initializeWindow(SDL_Window* window);
    void shutdown();

    SDL_GPUTexture* createGPUTexture(const ppl7::grafix::Drawable& surface);
    void destroyGPUTexture(SDL_GPUTexture* texture);
    void updateGPUTexture(SDL_GPUTexture* texture, const ppl7::grafix::Drawable& surface);

    SDL_GPUShader* loadShader(const ppl7::String& filename,
                              SDL_GPUShaderStage stage,
                              int num_samplers,
                              int num_storage_textures,
                              int num_storage_buffers,
                              int num_uniform_buffers);
    void releaseShader(SDL_GPUShader* shader);

    SDL_GPUTexture* createRenderTarget(int width, int height);
    SDL_GPUTexture* createDepthBuffer(int width, int height);

    void downloadTexture(SDL_GPUTexture* texture, int width, int height, ppl7::grafix::Image& target);
};

GPUContext& getGlobalGPUContext();

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
    uint32_t contextSwitchCount; // For debugging: Count how many times we switch GPU context (render pass)
    uint32_t totalSpriteCount;
    uint32_t totalPrimitivesCount;

    GPUBatcher();
    ~GPUBatcher();
    void resetContextSwitchCount();
    void init(GPUContext* gpu);
    void updateMatrices(int screenWidth, int screenHeight);
    void updateMatrices(const ppl7::grafix::Size& size);

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

#endif // INCLUDE_GPU_H
