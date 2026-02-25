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

#endif // INCLUDE_GPU_H
