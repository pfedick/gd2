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

    GPUException(const char* msg, ...) noexcept {
        va_list args;
        va_start(args, msg);
        copyText(msg, args);
        va_end(args);
    }


    const char* what() const noexcept override {
        return "GPUException";
    }
};


class GPUContext
{
private:


    SDL_Window* window;
public:
    // Es wäre besser, wenn dass hier ein GPUContext wäre, den wir anstelle von
    // SDL_Renderer verwenden könnten.

    SDL_GPUDevice* gpu;

    GPUContext();
    ~GPUContext();

    void init(SDL_Window* window);
    void shutdown();

    void initGPUDevice();
    SDL_GPUTexture* createGPUTexture(const ppl7::grafix::Drawable& surface);
    void destroyGPUTexture(SDL_GPUTexture* texture);
    void updateGPUTexture(SDL_GPUTexture* texture, const ppl7::grafix::Drawable& surface);

    SDL_GPUShader* loadShader(const ppl7::String& filename, SDL_GPUShaderStage stage, int num_samplers, int num_storage_textures, int num_storage_buffers, int num_uniform_buffers);
    void releaseShader(SDL_GPUShader* shader);


};

GPUContext& getGlobalGPUContext();


class GPUBatcher
{
private:
    float z;
    GPUContext* gpu;

    SDL_GPUShader* fragShader;
    SDL_GPUShader* vertShader;
    SDL_GPUGraphicsPipeline* spritePipeline;
    SDL_GPUSampler* sampler;
    SDL_GPUBuffer* vertexBuffer;
    SDL_GPUBuffer* indexBuffer;
    SDL_GPUBuffer* instanceBuffer;

    struct Vertex {
        float x, y;        // Position
        float u, v;        // Texcoords
        float r, g, b, a;  // Color
    };

    struct SpriteInstance {
        float pos_x, pos_y;           // Sprite world position (pixels)
        float size_w, size_h;         // Sprite size in pixels
        float scale_x, scale_y;       // Sprite scale factors
        float angle;                  // Rotation angle (radians)
        float uv_x, uv_y, uv_w, uv_h; // UV rect (normalized 0-1)
        float pivot_x, pivot_y;       // Pivot point (pixels)
        float offset_x, offset_y;     // Offset (pixels)
    };

    void loadShaders();
    void createPipeline();
    void createBuffers();
    void cleanup();
    void bindTexture(SDL_GPURenderPass* render_pass, SDL_GPUTexture* texture);
    void drawSprites(SDL_GPUCommandBuffer* cmd, SDL_GPURenderPass* render_pass, const std::list<SpriteCommand>& sprites);

    class PrimitiveCommand
    {
    public:
        enum class Type {
            Line,
            Rect,
            FilledRect
        };

        Type type;
        float x1, y1, x2, y2;
        float w, h;
        ppl7::grafix::Color color;
        float thickness;

        PrimitiveCommand(Type type, float x1, float y1, float x2, float y2, const ppl7::grafix::Color& color, float thickness)
            : type(type), x1(x1), y1(y1), x2(x2), y2(y2), color(color), thickness(thickness) {
        }

        PrimitiveCommand(Type type, float x, float y, float w, float h, const ppl7::grafix::Color& color)
            : type(type), x1(x), y1(y), w(w), h(h), color(color), thickness(0.0f) {
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

        SpriteCommand(const SpriteTexture* sprite, int sprite_id, float x, float y, float z, float scale_x, float scale_y, float angle, const ppl7::grafix::Color& color_modulation)
            : sprite(sprite), sprite_id(sprite_id), x(x), y(y), z(z), scale_x(scale_x), scale_y(scale_y), angle(angle), color_modulation(color_modulation) {
        }
    };

    std::list<PrimitiveCommand> primitiveCommands;
    std::map<uint64_t, std::list<SpriteCommand>> spriteCommands;


public:
    GPUBatcher();
    ~GPUBatcher();
    void init(GPUContext* gpu);

    void clearQueues(); // temporary?
    void startRenderPass();
    void endRenderPass(SDL_GPUCommandBuffer* cmd, SDL_GPURenderPass* render_pass);


    void addSprite(const SpriteTexture& sprite, int sprite_id, float x, float y, float scale_x = 1.0f, float scale_y = 1.0f, float angle = 0.0f, const ppl7::grafix::Color& color_modulation = ppl7::grafix::Color(255, 255, 255, 255));
    void addLine(float x1, float y1, float x2, float y2, const ppl7::grafix::Color& color, float thickness = 1.0f);
    void addRect(float x, float y, float w, float h, const ppl7::grafix::Color& color, float thickness = 1.0f);
    void addFilledRect(float x, float y, float w, float h, const ppl7::grafix::Color& color);


};

#endif // INCLUDE_GPU_H

