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

    void initGPUDevice();
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
};

GPUContext& getGlobalGPUContext();

class GPUBatcher
{
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
    Uint32 primitiveTriangleVertexCount;
    Uint32 primitiveLineVertexCount;

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
        float pad2;                               // PADDING
        float uv_x, uv_y, uv_w, uv_h;             // UV rect (normalized 0-1)
        float pivot_x, pivot_y;                   // Pivot point (0..1)
        float offset_x, offset_y;                 // Unused
        float color_r, color_g, color_b, color_a; // Color Modulation
    };

    class PrimitiveCommand
    {
    public:
        enum class Type
        {
            Line,
            Rect,
            FilledRect
        };

        Type type = Type::Line;
        float x1 = 0, y1 = 0, x2 = 0, y2 = 0;
        float w = 0, h = 0;
        ppl7::grafix::Color color;
        float thickness = 0.0f;

        PrimitiveCommand() = default;

        static PrimitiveCommand Line(float x1, float y1, float x2, float y2, const ppl7::grafix::Color& color, float thickness)
        {
            PrimitiveCommand cmd;
            cmd.type = Type::Line;
            cmd.x1 = x1;
            cmd.y1 = y1;
            cmd.x2 = x2;
            cmd.y2 = y2;
            cmd.color = color;
            cmd.thickness = thickness;
            return cmd;
        }

        static PrimitiveCommand Rect(
            Type type, float x, float y, float w, float h, const ppl7::grafix::Color& color, float thickness = 0.0f)
        {
            PrimitiveCommand cmd;
            cmd.type = type;
            cmd.x1 = x;
            cmd.y1 = y;
            cmd.w = w;
            cmd.h = h;
            cmd.color = color;
            cmd.thickness = thickness;
            return cmd;
        }
    };

    class SpriteCommand
    {
    public:
        const SpriteTexture* sprite;
        int sprite_id;
        float x, y, z;
        float scale_x, scale_y;
        float angle;
        ppl7::grafix::Color color_modulation;

        SpriteCommand(const SpriteTexture* sprite,
                      int sprite_id,
                      float x,
                      float y,
                      float z,
                      float scale_x,
                      float scale_y,
                      float angle,
                      const ppl7::grafix::Color& color_modulation)
            : sprite(sprite),
              sprite_id(sprite_id),
              x(x),
              y(y),
              z(z),
              scale_x(scale_x),
              scale_y(scale_y),
              angle(angle),
              color_modulation(color_modulation)
        {
        }
    };

    void loadShaders();
    void createPipeline();
    void createBuffers();
    void uploadStaticQuadData();
    void cleanup();
    void bindTexture(SDL_GPURenderPass* render_pass, SDL_GPUTexture* texture);
    void drawSprites(SDL_GPUCommandBuffer* cmd, SDL_GPURenderPass* render_pass, const std::list<SpriteCommand>& sprites);
    void drawPrimitives(SDL_GPURenderPass* render_pass);

    std::list<PrimitiveCommand> primitiveCommands;

    std::map<SDL_GPUTexture*, std::list<SpriteCommand>> spriteCommands;
    std::vector<SpriteInstance> instanceData;

public:
    GPUBatcher();
    ~GPUBatcher();
    void init(GPUContext* gpu);
    void updateMatrices(int screenWidth, int screenHeight);

    void clearQueues(); // temporary?
    void startRenderPass();
    void prepareInstanceData(SDL_GPUCommandBuffer* cmd); // Upload instance data before render pass
    void endRenderPass(SDL_GPUCommandBuffer* cmd, SDL_GPURenderPass* render_pass);

    void addSprite(const SpriteTexture& sprite,
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

#endif // INCLUDE_GPU_H
