#include <ppl7.h>
#include <ppl7-grafix.h>
#include "gpu.h"

static GPUContext* globalGPUContext = NULL;

GPUContext& getGlobalGPUContext()
{
    if (!globalGPUContext) {
        throw GPUException("Global GPUContext is not initialized");
    }
    return *globalGPUContext;
}

GPUContext::GPUContext()
{
    gpu = NULL;
    window = NULL;
    globalGPUContext = this;
}

GPUContext::~GPUContext()
{
    shutdown();
}

void GPUContext::initializeGPUDevice()
{
    if (gpu) shutdown();
    // Force SPIRV (Vulkan) since we only have SPIRV shaders compiled
    gpu = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV, // Only SPIRV -> forces Vulkan backend
                              true,                       // debug mode
                              NULL);
    if (!gpu) {
        throw GPUException("SDL_CreateGPUDevice failed: %s", SDL_GetError());
    }
}

void GPUContext::initializeWindow(SDL_Window* window)
{
    // Claim window for GPU device and initialize swapchain
    if (!SDL_ClaimWindowForGPUDevice(gpu, window)) {
        SDL_DestroyGPUDevice(gpu);
        gpu = NULL;
        throw GPUException("SDL_ClaimWindowForGPUDevice failed: %s", SDL_GetError());
    }

    // Limit frames in flight to 2 to minimize input lag (Double Buffering behavior)
    // This prevents the CPU from running too far ahead of the GPU
    SDL_SetGPUAllowedFramesInFlight(gpu, 1);

    // Use VSYNC for tearing-free 60 FPS, but rely on FramesInFlight=2 to keep latency low
    SDL_SetGPUSwapchainParameters(gpu, window, SDL_GPU_SWAPCHAINCOMPOSITION_SDR, SDL_GPU_PRESENTMODE_VSYNC);
    this->window = window;
}

void GPUContext::shutdown()
{
    if (gpu) {
        if (window) SDL_ReleaseWindowFromGPUDevice(gpu, window);
        SDL_DestroyGPUDevice(gpu);
        gpu = NULL;
    }
}

SDL_GPUTexture* GPUContext::createGPUTexture(const ppl7::grafix::Drawable& surface)
{
    if (!gpu) {
        throw GPUException("GPU device is not initialized");
    }
    // Textur-Beschreibung
    SDL_GPUTextureCreateInfo texture_info = {
        .type = SDL_GPU_TEXTURETYPE_2D,
        .format = SDL_GPU_TEXTUREFORMAT_B8G8R8A8_UNORM, // BGRA matches PPL7 byte order
        .usage = SDL_GPU_TEXTUREUSAGE_SAMPLER,          // Für Shader-Sampling
        .width = (Uint32)surface.width(),
        .height = (Uint32)surface.height(),
        .layer_count_or_depth = 1,
        .num_levels = 1,
    };
    // Textur erstellen
    SDL_GPUTexture* texture = SDL_CreateGPUTexture(gpu, &texture_info);
    if (!texture) {
        throw GPUException("SDL_CreateGPUTexture failed: %s", SDL_GetError());
    }

    // Daten in GPU hochladen
    SDL_GPUTransferBufferCreateInfo transfer_info = {
        .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
        .size = (Uint32)surface.width() * (Uint32)surface.height() * 4, // RGBA8 = 4 Bytes/Pixel
    };
    SDL_GPUTransferBuffer* transfer_buffer = SDL_CreateGPUTransferBuffer(gpu, &transfer_info);
    if (!transfer_buffer) {
        SDL_ReleaseGPUTexture(gpu, texture);
        throw GPUException("SDL_CreateGPUTransferBuffer failed: %s", SDL_GetError());
    }

    // Pixel-Daten kopieren
    void* mapped = SDL_MapGPUTransferBuffer(gpu, transfer_buffer, false);
    if (!mapped) {
        SDL_ReleaseGPUTransferBuffer(gpu, transfer_buffer);
        SDL_ReleaseGPUTexture(gpu, texture);
        throw GPUException("SDL_MapGPUTransferBuffer failed: %s", SDL_GetError());
    }
    memcpy(mapped, surface.adr(), surface.width() * surface.height() * 4);
    SDL_UnmapGPUTransferBuffer(gpu, transfer_buffer);

    // Mit Command Buffer zur GPU transferieren
    SDL_GPUCommandBuffer* cmd = SDL_AcquireGPUCommandBuffer(gpu);
    if (!cmd) {
        const char* e = SDL_GetError();
        SDL_ReleaseGPUTransferBuffer(gpu, transfer_buffer);
        SDL_ReleaseGPUTexture(gpu, texture);
        throw GPUException("SDL_AcquireGPUCommandBuffer failed: %s", e);
    }
    SDL_GPUCopyPass* copy_pass = SDL_BeginGPUCopyPass(cmd);
    if (!copy_pass) {
        const char* e = SDL_GetError();
        SDL_ReleaseGPUTransferBuffer(gpu, transfer_buffer);
        SDL_ReleaseGPUTexture(gpu, texture);
        throw GPUException("SDL_BeginGPUCopyPass failed: %s", e);
    }
    SDL_GPUTextureTransferInfo transfer_region = {
        .transfer_buffer = transfer_buffer, .offset = 0, .pixels_per_row = 0, .rows_per_layer = 0};
    SDL_GPUTextureRegion texture_region = {.texture = texture,
                                           .mip_level = 0,
                                           .layer = 0,
                                           .x = 0,
                                           .y = 0,
                                           .z = 0,
                                           .w = (Uint32)surface.width(),
                                           .h = (Uint32)surface.height(),
                                           .d = 1};
    SDL_UploadToGPUTexture(copy_pass, &transfer_region, &texture_region, false);
    SDL_EndGPUCopyPass(copy_pass);
    SDL_SubmitGPUCommandBuffer(cmd);
    SDL_WaitForGPUIdle(gpu); // Wartet bis GPU fertig ist
    SDL_ReleaseGPUTransferBuffer(gpu, transfer_buffer);
    return texture;
}

void GPUContext::destroyGPUTexture(SDL_GPUTexture* texture)
{
    if (!gpu) {
        throw GPUException("GPU device is not initialized");
    }
    if (texture) {
        SDL_ReleaseGPUTexture(gpu, texture);
    }
}

void GPUContext::updateGPUTexture(SDL_GPUTexture* texture, const ppl7::grafix::Drawable& surface)
{
    if (!gpu) {
        throw GPUException("GPU device is not initialized");
    }
    // Daten in GPU hochladen
    SDL_GPUTransferBufferCreateInfo transfer_info = {
        .size = (Uint32)surface.width() * (Uint32)surface.height() * 4, // RGBA8 = 4 Bytes/Pixel
    };
    SDL_GPUTransferBuffer* transfer_buffer = SDL_CreateGPUTransferBuffer(gpu, &transfer_info);
    if (!transfer_buffer) {
        throw GPUException("SDL_CreateGPUTransferBuffer failed: %s", SDL_GetError());
    }

    // Pixel-Daten kopieren
    void* mapped = SDL_MapGPUTransferBuffer(gpu, transfer_buffer, false);
    memcpy(mapped, surface.adr(), surface.width() * surface.height() * 4);
    SDL_UnmapGPUTransferBuffer(gpu, transfer_buffer);

    // Mit Command Buffer zur GPU transferieren
    SDL_GPUCommandBuffer* cmd = SDL_AcquireGPUCommandBuffer(gpu);
    SDL_GPUCopyPass* copy_pass = SDL_BeginGPUCopyPass(cmd);
    SDL_GPUTextureTransferInfo transfer_region = {.transfer_buffer = transfer_buffer, .offset = 0};
    SDL_GPUTextureRegion texture_region = {.texture = texture,
                                           .mip_level = 0,
                                           .layer = 0,
                                           .x = 0,
                                           .y = 0,
                                           .z = 0,
                                           .w = (Uint32)surface.width(),
                                           .h = (Uint32)surface.height(),
                                           .d = 1};
    SDL_UploadToGPUTexture(copy_pass, &transfer_region, &texture_region, false);
    SDL_EndGPUCopyPass(copy_pass);
    SDL_SubmitGPUCommandBuffer(cmd);
    SDL_ReleaseGPUTransferBuffer(gpu, transfer_buffer);
}

SDL_GPUShader* GPUContext::loadShader(const ppl7::String& filename,
                                      SDL_GPUShaderStage stage,
                                      int num_samplers,
                                      int num_storage_textures,
                                      int num_storage_buffers,
                                      int num_uniform_buffers)
{
    if (!gpu) {
        throw GPUException("GPU device is not initialized");
    }

    ppl7::ByteArray buffer;

    try {
        buffer = ppl7::File::load(filename);
    }
    catch (const ppl7::Exception& e) {
        throw GPUException("Failed to load shader file: %s", (const char*)filename);
    }

    SDL_GPUShaderCreateInfo shaderInfo = {.code_size = (size_t)buffer.size(),
                                          .code = (const Uint8*)buffer.adr(),
                                          .entrypoint = "main",
                                          .format = SDL_GPU_SHADERFORMAT_SPIRV,
                                          .stage = stage,
                                          .num_samplers = (Uint32)num_samplers,
                                          .num_storage_textures = (Uint32)num_storage_textures,
                                          .num_storage_buffers = (Uint32)num_storage_buffers,
                                          .num_uniform_buffers = (Uint32)num_uniform_buffers};

    SDL_GPUShader* shader = SDL_CreateGPUShader(gpu, &shaderInfo);
    if (!shader) {
        throw GPUException("SDL_CreateGPUShader failed for %s: %s", (const char*)filename, SDL_GetError());
    }
    return shader;
}

void GPUContext::releaseShader(SDL_GPUShader* shader)
{
    if (gpu && shader) {
        SDL_ReleaseGPUShader(gpu, shader);
    }
}

SDL_GPUTexture* GPUContext::createRenderTarget(int width, int height)
{
    if (!gpu) {
        throw GPUException("GPU device is not initialized");
    }
    // Textur-Beschreibung
    SDL_GPUTextureCreateInfo texture_info = {
        .type = SDL_GPU_TEXTURETYPE_2D,
        .format = SDL_GPU_TEXTUREFORMAT_B8G8R8A8_UNORM,                            // BGRA matches PPL7 byte order
        .usage = SDL_GPU_TEXTUREUSAGE_COLOR_TARGET | SDL_GPU_TEXTUREUSAGE_SAMPLER, // For rendering and shader sampling
        .width = (Uint32)width,
        .height = (Uint32)height,
        .layer_count_or_depth = 1,
        .num_levels = 1,
    };
    // Textur erstellen
    SDL_GPUTexture* texture = SDL_CreateGPUTexture(gpu, &texture_info);
    if (!texture) {
        throw GPUException("SDL_CreateGPUTexture failed: %s", SDL_GetError());
    }
    return texture;
}

SDL_GPUTexture* GPUContext::createDepthBuffer(int width, int height)
{
    if (!gpu) {
        throw GPUException("GPU device is not initialized");
    }

    SDL_GPUTextureCreateInfo depthInfo = {
        .type = SDL_GPU_TEXTURETYPE_2D,
        .format = SDL_GPU_TEXTUREFORMAT_D16_UNORM, // 16-bit depth is enough for 2D
        .usage = SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET,
        .width = (Uint32)width,
        .height = (Uint32)height,
        .layer_count_or_depth = 1,
        .num_levels = 1,
    };
    SDL_GPUTexture* depthTexture = SDL_CreateGPUTexture(gpu, &depthInfo);
    if (!depthTexture) {
        throw GPUException("SDL_CreateGPUTexture failed: %s", SDL_GetError());
    }
    return depthTexture;
}

GPUStreamingTexture::GPUStreamingTexture(SDL_GPUDevice* gpu, int width, int height)
{
    this->gpu = gpu;
    this->texture = NULL;
    this->transfer_buffer = NULL;
    needs_update = true;
    this->size = ppl7::grafix::Size(width, height);
    resize(width, height);
}

GPUStreamingTexture::~GPUStreamingTexture()
{
    if (texture) {
        SDL_ReleaseGPUTexture(gpu, texture);
        texture = NULL;
    }
    if (transfer_buffer) {
        SDL_ReleaseGPUTransferBuffer(gpu, transfer_buffer);
        transfer_buffer = NULL;
    }
}
ppl7::grafix::Drawable GPUStreamingTexture::lock()
{
    if (!transfer_buffer) {
        throw GPUException("Transfer buffer is not initialized");
    }
    Uint8* pixels = (Uint8*)SDL_MapGPUTransferBuffer(gpu, transfer_buffer, false);

    if (pixels) {
        // wrapper für ppltk erstellen (Pitch = Breite * 4 Bytes)
        return ppl7::grafix::Drawable(pixels, size.width * 4, size.width, size.height, ppl7::grafix::RGBFormat::A8R8G8B8);
    } else {
        throw GPUException("SDL_MapGPUTransferBuffer failed: %s", SDL_GetError());
    }
}

void GPUStreamingTexture::unlock()
{
    if (transfer_buffer) {
        SDL_UnmapGPUTransferBuffer(gpu, transfer_buffer);
        needs_update = true;
    }
}

ppl7::grafix::Size GPUStreamingTexture::getSize() const
{
    return size;
}

void GPUStreamingTexture::resize(int width, int height)
{
    if (texture) {
        SDL_ReleaseGPUTexture(gpu, texture);
        texture = NULL;
    }
    if (transfer_buffer) {
        SDL_ReleaseGPUTransferBuffer(gpu, transfer_buffer);
        transfer_buffer = NULL;
    }
    needs_update = true;
    size = ppl7::grafix::Size(width, height);

    SDL_GPUTextureCreateInfo textureInfo;
    memset(&textureInfo, 0, sizeof(SDL_GPUTextureCreateInfo));
    textureInfo.type = SDL_GPU_TEXTURETYPE_2D;
    textureInfo.format = SDL_GPU_TEXTUREFORMAT_B8G8R8A8_UNORM;
    textureInfo.usage = SDL_GPU_TEXTUREUSAGE_SAMPLER; // WICHTIG: Soll im Shader gelesen werden
    textureInfo.width = size.width;
    textureInfo.height = size.height;
    textureInfo.layer_count_or_depth = 1;
    textureInfo.num_levels = 1;
    // ppl7::PrintDebug("SDL_CreateGPUTexture for Window\n");
    texture = SDL_CreateGPUTexture(gpu, &textureInfo);
    if (!texture) throw GPUException("SDL_CreateGPUTexture ERROR: %s", SDL_GetError());

    SDL_GPUTransferBufferCreateInfo transferInfo = {.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
                                                    .size = (Uint32)(size.width * size.height * 4)};
    transfer_buffer = SDL_CreateGPUTransferBuffer(gpu, &transferInfo);
    if (!transfer_buffer) {
        SDL_ReleaseGPUTexture(gpu, texture);
        texture = NULL;
        throw GPUException("SDL_CreateGPUTransferBuffer ERROR: %s", SDL_GetError());
    }
}

void GPUStreamingTexture::updateTexture(SDL_GPUCommandBuffer* cmdbuf)
{
    if (needs_update) {
        SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(cmdbuf);
        SDL_GPUTextureTransferInfo transferInfo = {
            .transfer_buffer = transfer_buffer, .offset = 0, .pixels_per_row = (Uint32)size.width, .rows_per_layer = (Uint32)size.height};
        SDL_GPUTextureRegion textureRegion = {.texture = texture,
                                              .mip_level = 0,
                                              .layer = 0,
                                              .x = 0,
                                              .y = 0,
                                              .z = 0,
                                              .w = (Uint32)size.width,
                                              .h = (Uint32)size.height,
                                              .d = 1};
        SDL_UploadToGPUTexture(copyPass, &transferInfo, &textureRegion, false);
        SDL_EndGPUCopyPass(copyPass);
        needs_update = false;
    }
}

SDL_GPUTexture* GPUStreamingTexture::getTexture() const
{
    return texture;
}

void GPUContext::downloadTexture(SDL_GPUTexture* texture, int width, int height, ppl7::grafix::Image& target)
{
    if (!gpu) {
        throw GPUException("GPU device is not initialized");
    }
    // 1. Transfer Buffer für den Download erstellen
    Uint32 bufferSize = width * height * 4;
    SDL_GPUTransferBufferCreateInfo transferInfo = {.usage = SDL_GPU_TRANSFERBUFFERUSAGE_DOWNLOAD, .size = bufferSize};
    SDL_GPUTransferBuffer* downloadBuffer = SDL_CreateGPUTransferBuffer(gpu, &transferInfo);

    // 2. Download-Befehl abschicken
    SDL_GPUCommandBuffer* cmd = SDL_AcquireGPUCommandBuffer(gpu);
    SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(cmd);

    SDL_GPUTextureRegion sourceRegion = {.texture = texture, .w = (Uint32)width, .h = (Uint32)height, .d = 1};
    SDL_GPUTextureTransferInfo destInfo = {.transfer_buffer = downloadBuffer, .offset = 0};

    SDL_DownloadFromGPUTexture(copyPass, &sourceRegion, &destInfo);
    SDL_EndGPUCopyPass(copyPass);

    // 3. Warten, bis die GPU fertig ist (Synchroner Download)
    SDL_SubmitGPUCommandBuffer(cmd);
    SDL_WaitForGPUIdle(gpu);
    // 4. Daten mappen und in PPL7 Image kopieren
    void* mapped = SDL_MapGPUTransferBuffer(gpu, downloadBuffer, false);
    target.create(width, height, ppl7::grafix::RGBFormat::A8R8G8B8);
    memcpy(target.adr(), mapped, bufferSize);

    SDL_UnmapGPUTransferBuffer(gpu, downloadBuffer);
    SDL_ReleaseGPUTransferBuffer(gpu, downloadBuffer);
}
