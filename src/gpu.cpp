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

void GPUContext::init(SDL_Window* window)
{
    shutdown();
    // Force SPIRV (Vulkan) since we only have SPIRV shaders compiled
    gpu = SDL_CreateGPUDevice(
        SDL_GPU_SHADERFORMAT_SPIRV,  // Only SPIRV -> forces Vulkan backend
        true,  // debug mode
        NULL
    );
    if (!gpu) {
        throw GPUException("SDL_CreateGPUDevice failed: %s", SDL_GetError());
    }

    // Log which backend was chosen
    const char* driver = SDL_GetGPUDeviceDriver(gpu);
    ppl7::PrintDebugTime("GPU Device created with driver: %s\n", driver ? driver : "unknown");

    // Claim window for GPU device and initialize swapchain
    if (!SDL_ClaimWindowForGPUDevice(gpu, window)) {
        SDL_DestroyGPUDevice(gpu);
        gpu = NULL;
        throw GPUException("SDL_ClaimWindowForGPUDevice failed: %s", SDL_GetError());
    }
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
    .format = SDL_GPU_TEXTUREFORMAT_B8G8R8A8_UNORM,  // BGRA matches PPL7 byte order
    .usage = SDL_GPU_TEXTUREUSAGE_SAMPLER,  // Für Shader-Sampling
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
    SDL_GPUTransferBufferCreateInfo  transfer_info = {
        .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
        .size = (Uint32)surface.width() * (Uint32)surface.height() * 4,  // RGBA8 = 4 Bytes/Pixel
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
        .transfer_buffer = transfer_buffer,
        .offset = 0,
        .pixels_per_row = 0,
        .rows_per_layer = 0
    };
    SDL_GPUTextureRegion texture_region = {
        .texture = texture,
        .mip_level = 0,
        .layer = 0,
        .x = 0,
        .y = 0,
        .z = 0,
        .w = (Uint32)surface.width(),
        .h = (Uint32)surface.height(),
        .d = 1
    };
    SDL_UploadToGPUTexture(copy_pass, &transfer_region, &texture_region, false);
    SDL_EndGPUCopyPass(copy_pass);
    SDL_SubmitGPUCommandBuffer(cmd);
    SDL_WaitForGPUIdle(gpu);  // Wartet bis GPU fertig ist
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
    SDL_GPUTransferBufferCreateInfo  transfer_info = {
        .size = (Uint32)surface.width() * (Uint32)surface.height() * 4,  // RGBA8 = 4 Bytes/Pixel
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
    SDL_GPUTextureTransferInfo transfer_region = {
        .transfer_buffer = transfer_buffer,
        .offset = 0
    };
    SDL_GPUTextureRegion texture_region = {
        .texture = texture,
        .mip_level = 0,
        .layer = 0,
        .x = 0,
        .y = 0,
        .z = 0,
        .w = (Uint32)surface.width(),
        .h = (Uint32)surface.height(),
        .d = 1
    };
    SDL_UploadToGPUTexture(copy_pass, &transfer_region, &texture_region, false);
    SDL_EndGPUCopyPass(copy_pass);
    SDL_SubmitGPUCommandBuffer(cmd);
    SDL_ReleaseGPUTransferBuffer(gpu, transfer_buffer);
}

SDL_GPUShader* GPUContext::loadShader(const ppl7::String& filename, SDL_GPUShaderStage stage, int num_samplers, int num_storage_textures, int num_storage_buffers, int num_uniform_buffers)
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

    SDL_GPUShaderCreateInfo shaderInfo = {
        .code_size = (size_t)buffer.size(),
        .code = (const Uint8*)buffer.adr(),
        .entrypoint = "main",
        .format = SDL_GPU_SHADERFORMAT_SPIRV,
        .stage = stage,
        .num_samplers = (Uint32)num_samplers,
        .num_storage_textures = (Uint32)num_storage_textures,
        .num_storage_buffers = (Uint32)num_storage_buffers,
        .num_uniform_buffers = (Uint32)num_uniform_buffers
    };

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

