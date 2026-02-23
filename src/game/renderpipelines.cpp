#include "renderpipelines.h"
#include "sdl.h"
#include "gpu.h"

RenderPipelines::RenderPipelines()
{
    gpu = nullptr;
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
        SDL_ReleaseGPUSampler(gpu->gpu, samplerClamp);
    }
    if (blurHorizontalPipeline) {
        SDL_ReleaseGPUGraphicsPipeline(gpu->gpu, blurHorizontalPipeline);
    }
    if (blurVerticalPipeline) {
        SDL_ReleaseGPUGraphicsPipeline(gpu->gpu, blurVerticalPipeline);
    }
    if (copyPipeline) {
        SDL_ReleaseGPUGraphicsPipeline(gpu->gpu, copyPipeline);
    }
    if (gpu) {
        gpu->releaseShader(copyShader);
        gpu->releaseShader(vertexShader);
        gpu->releaseShader(blurHorizontalShader);
        gpu->releaseShader(blurVerticalShader);
    }
}

void RenderPipelines::init(GPUContext& gpu, SDL_Window* window)
{
    this->gpu = &gpu;
    this->window = window;
    loadShaders();
    createPipelines();
    createSamplers();
}

SDL_GPUDevice* RenderPipelines::getGPUDevice()
{
    return gpu->gpu;
}

void RenderPipelines::loadShaders()
{
    if (!gpu) return;
    vertexShader = gpu->loadShader("res/shaders/vulkan/ndc_textured.vert.spv", SDL_GPU_SHADERSTAGE_VERTEX, 0, 0, 0, 0);
    blurHorizontalShader = gpu->loadShader("res/shaders/vulkan/blur_horizontal.spv", SDL_GPU_SHADERSTAGE_FRAGMENT, 1, 0, 0, 1);
    blurVerticalShader = gpu->loadShader("res/shaders/vulkan/blur_vertical.spv", SDL_GPU_SHADERSTAGE_FRAGMENT, 1, 0, 0, 1);
    copyShader = gpu->loadShader("res/shaders/vulkan/copy.frag.spv", SDL_GPU_SHADERSTAGE_FRAGMENT, 1, 0, 0, 0);
}

void RenderPipelines::createPipelines()
{
    // Zielformat definieren (muss zum Framebuffer/Texture passen, auf die gerendert wird)
    SDL_GPUColorTargetDescription targetDesc = {};
    targetDesc.format = SDL_GPU_TEXTUREFORMAT_B8G8R8A8_UNORM; // Standardformat angenommen
    targetDesc.blend_state.enable_blend = false;              // Blur ersetzt meist den Inhalt
    targetDesc.blend_state.color_write_mask = 0xF;

    SDL_GPUGraphicsPipelineCreateInfo pipelineInfo = {};
    pipelineInfo.vertex_shader = vertexShader;
    pipelineInfo.fragment_shader = blurHorizontalShader;
    pipelineInfo.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;
    pipelineInfo.target_info.num_color_targets = 1;
    pipelineInfo.target_info.color_target_descriptions = &targetDesc;

    // Pipeline erstellen für horizontalen Blur
    blurHorizontalPipeline = SDL_CreateGPUGraphicsPipeline(gpu->gpu, &pipelineInfo);
    if (!blurHorizontalPipeline) {
        ppl7::PrintDebug("SDL_CreateGPUGraphicsPipeline failed for horizontal blur: %s\n", SDL_GetError());
        throw SDLException("SDL_CreateGPUGraphicsPipeline failed for horizontal blur: %s", SDL_GetError());
    }

    // Pipeline erstellen für vertikalen Blur
    pipelineInfo.fragment_shader = blurVerticalShader;
    blurVerticalPipeline = SDL_CreateGPUGraphicsPipeline(gpu->gpu, &pipelineInfo);
    if (!blurVerticalPipeline) {
        ppl7::PrintDebug("SDL_CreateGPUGraphicsPipeline failed for vertical blur: %s\n", SDL_GetError());
        throw SDLException("SDL_CreateGPUGraphicsPipeline failed for vertical blur: %s", SDL_GetError());
    }

    // Pipeline erstellen für Copy
    // targetDesc.format = SDL_GPU_TEXTUREFORMAT_B8G8R8A8_UNORM; // Standardformat angenommen
    targetDesc.format = SDL_GetGPUSwapchainTextureFormat(gpu->gpu, window);
    targetDesc.blend_state.enable_blend = false; // Blur ersetzt meist den Inhalt
    targetDesc.blend_state.color_write_mask = 0xF;

    pipelineInfo.fragment_shader = copyShader;
    copyPipeline = SDL_CreateGPUGraphicsPipeline(gpu->gpu, &pipelineInfo);
    if (!copyPipeline) {
        ppl7::PrintDebug("SDL_CreateGPUGraphicsPipeline failed for copy: %s\n", SDL_GetError());
        throw SDLException("SDL_CreateGPUGraphicsPipeline failed for copy: %s", SDL_GetError());
    }

    // Pipeline für UI (Copy mit Blending)
    targetDesc.blend_state.enable_blend = true;
    targetDesc.blend_state.src_color_blendfactor = SDL_GPU_BLENDFACTOR_ONE;
    targetDesc.blend_state.dst_color_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
    targetDesc.blend_state.color_blend_op = SDL_GPU_BLENDOP_ADD;
    targetDesc.blend_state.src_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE;
    targetDesc.blend_state.dst_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
    targetDesc.blend_state.alpha_blend_op = SDL_GPU_BLENDOP_ADD;

    uiPipeline = SDL_CreateGPUGraphicsPipeline(gpu->gpu, &pipelineInfo);
    if (!uiPipeline) {
        ppl7::PrintDebug("SDL_CreateGPUGraphicsPipeline failed for UI: %s\n", SDL_GetError());
        throw SDLException("SDL_CreateGPUGraphicsPipeline failed for UI: %s", SDL_GetError());
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
        .max_anisotropy = 8.0f,
        .max_lod = 1000.0f,
        .enable_anisotropy = true,
    };
    samplerClamp = SDL_CreateGPUSampler(gpu->gpu, &samplerInfo);
    if (!samplerClamp) {
        ppl7::PrintDebug("Failed to create sampler: %s\n", SDL_GetError());
        throw SDLException("Failed to create sampler: %s", SDL_GetError());
    }
}
