#include "renderpipelines.h"
#include "sdl.h"

RenderPipelines::RenderPipelines()
{
    gpu_device = nullptr;
    window = nullptr;
    vertexShader = nullptr;
    blurHorizontalShader = nullptr;
    blurVerticalShader = nullptr;
    blurHorizontalPipeline = nullptr;
    blurVerticalPipeline = nullptr;
    samplerClamp = NULL;
    copyPipeline = nullptr;
    copyShader = nullptr;
}

RenderPipelines::~RenderPipelines()
{
    if (samplerClamp) {
        SDL_ReleaseGPUSampler(gpu_device, samplerClamp);
    }
    if (blurHorizontalPipeline) {
        SDL_ReleaseGPUGraphicsPipeline(gpu_device, blurHorizontalPipeline);
    }
    if (blurVerticalPipeline) {
        SDL_ReleaseGPUGraphicsPipeline(gpu_device, blurVerticalPipeline);
    }
    if (copyPipeline) {
        SDL_ReleaseGPUGraphicsPipeline(gpu_device, copyPipeline);
    }
    releaseShader(copyShader);
    releaseShader(vertexShader);
    releaseShader(blurHorizontalShader);
    releaseShader(blurVerticalShader);
}


void RenderPipelines::init(SDL_GPUDevice* gpu, SDL_Window* window)
{
    gpu_device = gpu;
    this->window = window;
    loadShaders();
    createPipelines();
    createSamplers();
}

SDL_GPUDevice* RenderPipelines::getGPUDevice()
{
    return gpu_device;
}

SDL_GPUShader* RenderPipelines::loadShader(const ppl7::String& filename, SDL_GPUShaderStage stage, int num_samplers, int num_storage_textures, int num_storage_buffers, int num_uniform_buffers)
{
    if (!gpu_device) {
        ppl7::PrintDebug("GPU device is not initialized\n");
        throw SDLException("GPU device is not initialized");
    }

    ppl7::ByteArray buffer;

    try {
        buffer = ppl7::File::load(filename);
    }
    catch (const ppl7::Exception& e) {
        ppl7::PrintDebug("Failed to load shader file: %s: %s\n", (const char*)filename, e.what());
        throw SDLException("Failed to load shader file: %s", (const char*)filename);
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

    SDL_GPUShader* shader = SDL_CreateGPUShader(gpu_device, &shaderInfo);
    if (!shader) {
        ppl7::PrintDebug("SDL_CreateGPUShader failed for %s: %s\n", (const char*)filename, SDL_GetError());
        throw SDLException("SDL_CreateGPUShader failed for %s: %s", (const char*)filename, SDL_GetError());
    }
    return shader;
}

void RenderPipelines::releaseShader(SDL_GPUShader* shader)
{
    if (gpu_device && shader) {
        SDL_ReleaseGPUShader(gpu_device, shader);
    }
}


void RenderPipelines::loadShaders()
{
    vertexShader = loadShader("res/shaders/vulkan/ndc_textured.vert.spv", SDL_GPU_SHADERSTAGE_VERTEX, 0, 0, 0, 0);
    blurHorizontalShader = loadShader("res/shaders/vulkan/blur_horizontal.spv", SDL_GPU_SHADERSTAGE_FRAGMENT, 1, 0, 0, 1);
    blurVerticalShader = loadShader("res/shaders/vulkan/blur_vertical.spv", SDL_GPU_SHADERSTAGE_FRAGMENT, 1, 0, 0, 1);
    copyShader = loadShader("res/shaders/vulkan/copy.frag.spv", SDL_GPU_SHADERSTAGE_FRAGMENT, 1, 0, 0, 0);
}


void RenderPipelines::createPipelines()
{
    // Zielformat definieren (muss zum Framebuffer/Texture passen, auf die gerendert wird)
    SDL_GPUColorTargetDescription targetDesc = {};
    targetDesc.format = SDL_GPU_TEXTUREFORMAT_B8G8R8A8_UNORM; // Standardformat angenommen
    targetDesc.blend_state.enable_blend = false; // Blur ersetzt meist den Inhalt
    targetDesc.blend_state.color_write_mask = 0xF;

    SDL_GPUGraphicsPipelineCreateInfo pipelineInfo = {};
    pipelineInfo.vertex_shader = vertexShader;
    pipelineInfo.fragment_shader = blurHorizontalShader;
    pipelineInfo.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;
    pipelineInfo.target_info.num_color_targets = 1;
    pipelineInfo.target_info.color_target_descriptions = &targetDesc;

    // Pipeline erstellen für horizontalen Blur
    blurHorizontalPipeline = SDL_CreateGPUGraphicsPipeline(gpu_device, &pipelineInfo);
    if (!blurHorizontalPipeline) {
        ppl7::PrintDebug("SDL_CreateGPUGraphicsPipeline failed for horizontal blur: %s\n", SDL_GetError());
        throw SDLException("SDL_CreateGPUGraphicsPipeline failed for horizontal blur: %s", SDL_GetError());
    }

    // Pipeline erstellen für vertikalen Blur
    pipelineInfo.fragment_shader = blurVerticalShader;
    blurVerticalPipeline = SDL_CreateGPUGraphicsPipeline(gpu_device, &pipelineInfo);
    if (!blurVerticalPipeline) {
        ppl7::PrintDebug("SDL_CreateGPUGraphicsPipeline failed for vertical blur: %s\n", SDL_GetError());
        throw SDLException("SDL_CreateGPUGraphicsPipeline failed for vertical blur: %s", SDL_GetError());
    }

    // Pipeline erstellen für Copy
    //targetDesc.format = SDL_GPU_TEXTUREFORMAT_B8G8R8A8_UNORM; // Standardformat angenommen
    targetDesc.format = SDL_GetGPUSwapchainTextureFormat(gpu_device,window);
    targetDesc.blend_state.enable_blend = false; // Blur ersetzt meist den Inhalt
    targetDesc.blend_state.color_write_mask = 0xF;

    pipelineInfo.fragment_shader = copyShader;
    copyPipeline = SDL_CreateGPUGraphicsPipeline(gpu_device, &pipelineInfo);
    if (!copyPipeline) {
        ppl7::PrintDebug("SDL_CreateGPUGraphicsPipeline failed for copy: %s\n", SDL_GetError());
        throw SDLException("SDL_CreateGPUGraphicsPipeline failed for copy: %s", SDL_GetError());
    }
}

void RenderPipelines::createSamplers()
{
    SDL_GPUSamplerCreateInfo samplerInfo = {
        .min_filter = SDL_GPU_FILTER_LINEAR,
        .mag_filter = SDL_GPU_FILTER_LINEAR,
        .mipmap_mode = SDL_GPU_SAMPLERMIPMAPMODE_LINEAR,
        .address_mode_u = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
        .address_mode_v = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
        .address_mode_w = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
    };
    samplerClamp = SDL_CreateGPUSampler(gpu_device, &samplerInfo);
    if (!samplerClamp) {
        ppl7::PrintDebug("Failed to create sampler: %s\n", SDL_GetError());
        throw SDLException("Failed to create sampler: %s", SDL_GetError());
    }
}
   