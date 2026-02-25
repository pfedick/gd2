#include "gamerenderer.h"
#include "gpu.h"
#include "SDL3/SDL.h"

GameRenderer::GameRenderer()
{
    gpu = nullptr;
    window = nullptr;
    cmdbuf = nullptr;
    swapchainTexture = nullptr;

    blurHorizontalShader = nullptr;
    blurVerticalShader = nullptr;
    copyShader = nullptr;
    vertexShader = nullptr;

    blurHorizontalPipeline = nullptr;
    blurVerticalPipeline = nullptr;
    copyPipeline = nullptr;
    copyWithAlphablendingPipeline = nullptr;

    samplerClamp = nullptr;

    render_target = nullptr;
    render_layer = nullptr;
    render_lightmap = nullptr;
    blur_temp = nullptr;
    render_normal = nullptr;
}

GameRenderer::~GameRenderer()
{
    if (gpu) {
        gpu->releaseShader(blurHorizontalShader);
        gpu->releaseShader(blurVerticalShader);
        gpu->releaseShader(copyShader);
        gpu->releaseShader(vertexShader);

        if (blurHorizontalPipeline) {
            SDL_ReleaseGPUGraphicsPipeline(gpu->gpu, blurHorizontalPipeline);
        }
        if (blurVerticalPipeline) {
            SDL_ReleaseGPUGraphicsPipeline(gpu->gpu, blurVerticalPipeline);
        }
        if (copyPipeline) {
            SDL_ReleaseGPUGraphicsPipeline(gpu->gpu, copyPipeline);
        }
        if (copyWithAlphablendingPipeline) {
            SDL_ReleaseGPUGraphicsPipeline(gpu->gpu, copyWithAlphablendingPipeline);
        }

        if (samplerClamp) {
            SDL_ReleaseGPUSampler(gpu->gpu, samplerClamp);
        }
        if (render_target) gpu->destroyGPUTexture(render_target);
        if (render_layer) gpu->destroyGPUTexture(render_layer);
        if (render_lightmap) gpu->destroyGPUTexture(render_lightmap);
        if (blur_temp) gpu->destroyGPUTexture(blur_temp);
        if (render_normal) gpu->destroyGPUTexture(render_normal);
        render_target_size.setSize(0, 0);
    }
}

void GameRenderer::init(GPUContext& gpu, SDL_Window* window)
{
    this->gpu = &gpu;
    this->window = window;
    loadShaders();
    createPipelines();
    createSamplers();
    batcher.init(&gpu, samplerClamp);
}

void GameRenderer::loadShaders()
{
    if (!gpu) return;
    vertexShader = gpu->loadShader("res/shaders/vulkan/ndc_textured.vert.spv", SDL_GPU_SHADERSTAGE_VERTEX, 0, 0, 0, 0);
    blurHorizontalShader = gpu->loadShader("res/shaders/vulkan/blur_horizontal.spv", SDL_GPU_SHADERSTAGE_FRAGMENT, 1, 0, 0, 1);
    blurVerticalShader = gpu->loadShader("res/shaders/vulkan/blur_vertical.spv", SDL_GPU_SHADERSTAGE_FRAGMENT, 1, 0, 0, 1);
    copyShader = gpu->loadShader("res/shaders/vulkan/copy.frag.spv", SDL_GPU_SHADERSTAGE_FRAGMENT, 1, 0, 0, 0);
}

void GameRenderer::createPipelines()
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
        throw GPUException("SDL_CreateGPUGraphicsPipeline failed for horizontal blur: %s", SDL_GetError());
    }

    // Pipeline erstellen für vertikalen Blur
    pipelineInfo.fragment_shader = blurVerticalShader;
    blurVerticalPipeline = SDL_CreateGPUGraphicsPipeline(gpu->gpu, &pipelineInfo);
    if (!blurVerticalPipeline) {
        ppl7::PrintDebug("SDL_CreateGPUGraphicsPipeline failed for vertical blur: %s\n", SDL_GetError());
        throw GPUException("SDL_CreateGPUGraphicsPipeline failed for vertical blur: %s", SDL_GetError());
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
        throw GPUException("SDL_CreateGPUGraphicsPipeline failed for copy: %s", SDL_GetError());
    }

    // Pipeline für UI (Copy mit Blending)
    targetDesc.blend_state.enable_blend = true;
    targetDesc.blend_state.src_color_blendfactor = SDL_GPU_BLENDFACTOR_ONE;
    targetDesc.blend_state.dst_color_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
    targetDesc.blend_state.color_blend_op = SDL_GPU_BLENDOP_ADD;
    targetDesc.blend_state.src_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE;
    targetDesc.blend_state.dst_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
    targetDesc.blend_state.alpha_blend_op = SDL_GPU_BLENDOP_ADD;

    copyWithAlphablendingPipeline = SDL_CreateGPUGraphicsPipeline(gpu->gpu, &pipelineInfo);
    if (!copyWithAlphablendingPipeline) {
        ppl7::PrintDebug("SDL_CreateGPUGraphicsPipeline failed for UI: %s\n", SDL_GetError());
        throw GPUException("SDL_CreateGPUGraphicsPipeline failed for UI: %s", SDL_GetError());
    }
}

void GameRenderer::createSamplers()
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
        throw GPUException("Failed to create sampler: %s", SDL_GetError());
    }
}

void GameRenderer::resizeRenderBuffer(const ppl7::grafix::Size& size)
{
    if (!gpu) return;
    if (size != render_target_size) {
        // ppl7::PrintDebug("Resizing Level Render Targets to %dx%d\n", size.width, size.height);
        if (render_target) gpu->destroyGPUTexture(render_target);
        render_target = gpu->createRenderTarget(size.width, size.height);
        if (render_layer) gpu->destroyGPUTexture(render_layer);
        render_layer = gpu->createRenderTarget(size.width, size.height);
        if (render_lightmap) gpu->destroyGPUTexture(render_lightmap);
        render_lightmap = gpu->createRenderTarget(size.width, size.height);
        if (blur_temp) gpu->destroyGPUTexture(blur_temp);
        blur_temp = gpu->createRenderTarget(size.width, size.height);
        render_target_size = size;
    }
}

void GameRenderer::copyTextureToSwapchain(SDL_GPUTexture* source, const SDL_FRect& destRect)
{
    SDL_GPUColorTargetInfo colorTargetInfo = {0};
    colorTargetInfo.texture = swapchainTexture;
    colorTargetInfo.clear_color = (SDL_FColor){0.0f, 0.0f, 0.0f, 1.0f}; // Black background
    colorTargetInfo.load_op = SDL_GPU_LOADOP_CLEAR;
    colorTargetInfo.store_op = SDL_GPU_STOREOP_STORE;
    colorTargetInfo.cycle = false;

    SDL_GPURenderPass* renderPass = SDL_BeginGPURenderPass(cmdbuf, &colorTargetInfo, 1, NULL);

    // Hier setzen wir den Viewport auf die Zielgröße im Fenster
    SDL_GPUViewport gpuViewport = {
        .x = destRect.x, .y = destRect.y, .w = destRect.w, .h = destRect.h, .min_depth = 0.0f, .max_depth = 1.0f};

    SDL_SetGPUViewport(renderPass, &gpuViewport);
    SDL_SetGPUScissor(renderPass, NULL);

    SDL_BindGPUGraphicsPipeline(renderPass, copyPipeline);
    SDL_GPUTextureSamplerBinding binding = {};

    binding.texture = source;
    binding.sampler = samplerClamp; // Einen Clamp-Sampler benutzen!
    SDL_BindGPUFragmentSamplers(renderPass, 0, &binding, 1);

    // Fullscreen Triangle zeichnen (3 Vertices, Shader generiert Coords)
    SDL_DrawGPUPrimitives(renderPass, 3, 1, 0, 0);

    SDL_EndGPURenderPass(renderPass);
}

void GameRenderer::clearTexture(SDL_GPUTexture* texture, const ppl7::grafix::Color& color)
{
    SDL_GPUColorTargetInfo rtInfo = {0};
    rtInfo.texture = texture;
    rtInfo.clear_color = toSDLFColor(color);
    rtInfo.load_op = SDL_GPU_LOADOP_CLEAR;
    rtInfo.store_op = SDL_GPU_STOREOP_STORE;
    rtInfo.cycle = false;
    SDL_GPURenderPass* rtClearPass = SDL_BeginGPURenderPass(cmdbuf, &rtInfo, 1, NULL);
    SDL_EndGPURenderPass(rtClearPass);
}

void GameRenderer::copyTexture(SDL_GPUTexture* source, SDL_GPUTexture* target, bool alphablend)
{
    SDL_GPUColorTargetInfo targetInfo = {};
    SDL_GPUTextureSamplerBinding binding = {};
    targetInfo.texture = target;
    targetInfo.load_op = SDL_GPU_LOADOP_LOAD;
    targetInfo.store_op = SDL_GPU_STOREOP_STORE;

    SDL_GPURenderPass* renderPass = SDL_BeginGPURenderPass(cmdbuf, &targetInfo, 1, NULL);
    SDL_SetGPUViewport(renderPass, NULL);
    SDL_SetGPUScissor(renderPass, NULL);

    if (alphablend)
        SDL_BindGPUGraphicsPipeline(renderPass, copyWithAlphablendingPipeline);
    else
        SDL_BindGPUGraphicsPipeline(renderPass, copyPipeline);

    binding.texture = source;
    binding.sampler = samplerClamp; // Einen Clamp-Sampler benutzen!
    SDL_BindGPUFragmentSamplers(renderPass, 0, &binding, 1);

    // Fullscreen Triangle zeichnen (3 Vertices, Shader generiert Coords)
    SDL_DrawGPUPrimitives(renderPass, 3, 1, 0, 0);

    SDL_EndGPURenderPass(renderPass);
}

struct BlurParams
{
    float blurStrength;
    float padding; // WICHTIG: 4 Bytes Füllmaterial für std140 Alignment
    float texelSizeX;
    float texelSizeY;
};

void GameRenderer::blur(SDL_GPUTexture* source, SDL_GPUTexture* target, float blur_factor)
{
    SDL_GPUColorTargetInfo targetInfo = {};
    SDL_GPUTextureSamplerBinding binding = {};

    BlurParams params;
    float scale = (float)render_target_size.width / 3840.0f;
    params.blurStrength = blur_factor * scale;
    params.padding = 0.0f;                                       // Egal was hier steht
    params.texelSizeX = 1.0f / (float)render_target_size.width;  // Breite der Textur
    params.texelSizeY = 1.0f / (float)render_target_size.height; // Höhe der Textur

    // Slot Index 0 (passend zu binding = 0 im Shader, Set 3 ist implizit für Fragment Uniforms)
    SDL_PushGPUFragmentUniformData(cmdbuf, 0, &params, sizeof(BlurParams));

    targetInfo.texture = blur_temp; // Ziel: Temp Textur
    targetInfo.load_op = SDL_GPU_LOADOP_DONT_CARE;
    targetInfo.store_op = SDL_GPU_STOREOP_STORE;

    SDL_GPURenderPass* renderPass = SDL_BeginGPURenderPass(cmdbuf, &targetInfo, 1, NULL);
    SDL_SetGPUViewport(renderPass, NULL);
    SDL_SetGPUScissor(renderPass, NULL);

    SDL_BindGPUGraphicsPipeline(renderPass, blurHorizontalPipeline);
    binding.texture = source;
    binding.sampler = samplerClamp; // Einen Clamp-Sampler benutzen!
    SDL_BindGPUFragmentSamplers(renderPass, 0, &binding, 1);

    // Fullscreen Triangle zeichnen (3 Vertices, Shader generiert Coords)
    SDL_DrawGPUPrimitives(renderPass, 3, 1, 0, 0);

    SDL_EndGPURenderPass(renderPass);

    targetInfo.texture = target;
    targetInfo.load_op = SDL_GPU_LOADOP_DONT_CARE; // Wir überschreiben alles
    targetInfo.store_op = SDL_GPU_STOREOP_STORE;

    renderPass = SDL_BeginGPURenderPass(cmdbuf, &targetInfo, 1, NULL);
    SDL_SetGPUViewport(renderPass, NULL);
    SDL_SetGPUScissor(renderPass, NULL);

    SDL_BindGPUGraphicsPipeline(renderPass, blurVerticalPipeline);

    // Eingabe-Textur binden (das Bild aus Pass 2)
    binding.texture = blur_temp;
    binding.sampler = samplerClamp;
    SDL_BindGPUFragmentSamplers(renderPass, 0, &binding, 1);

    SDL_DrawGPUPrimitives(renderPass, 3, 1, 0, 0);
    SDL_EndGPURenderPass(renderPass);
}

bool GameRenderer::accuireGPUCommandBuffer()
{
    if (cmdbuf) {
        SDL_SubmitGPUCommandBuffer(cmdbuf);
        cmdbuf = nullptr;
    }
    cmdbuf = SDL_AcquireGPUCommandBuffer(gpu->gpu);
    if (cmdbuf == NULL) {
        SDL_Log("AcquireGPUCommandBuffer failed: %s", SDL_GetError());
        return false;
    }
    if (!SDL_WaitAndAcquireGPUSwapchainTexture(cmdbuf, window, &swapchainTexture, NULL, NULL)) {
        SDL_Log("WaitAndAcquireGPUSwapchainTexture failed: %s", SDL_GetError());
        SDL_SubmitGPUCommandBuffer(cmdbuf);
        cmdbuf = nullptr;
        return false;
    }
    if (swapchainTexture == NULL) {
        // Das kann passieren, wenn das Fenster minimiert ist
        SDL_SubmitGPUCommandBuffer(cmdbuf);
        return false;
    }
    return true;
}

void GameRenderer::submitGPUCommandBuffer()
{
    if (cmdbuf) {
        SDL_SubmitGPUCommandBuffer(cmdbuf);
        cmdbuf = nullptr;
    }
}

SDL_GPUCommandBuffer* GameRenderer::getCommandBuffer()
{
    return cmdbuf;
}

SDL_GPUTexture* GameRenderer::getSwapchainTexture()
{
    return swapchainTexture;
}

ppl7::grafix::Image GameRenderer::getScreenshot(int width, int height)
{
    if (!gpu) return ppl7::grafix::Image();
    ppl7::grafix::Image img;

    // 1. Kleine temporäre Target-Textur erstellen
    SDL_GPUTexture* thumbTex = gpu->createRenderTarget(width, height);

    SDL_GPUCommandBuffer* cmdbuf = SDL_AcquireGPUCommandBuffer(gpu->gpu);

    // 2. In die kleine Textur rendern (Skalierung)
    SDL_GPUColorTargetInfo targetInfo = {0};
    targetInfo.texture = thumbTex;
    targetInfo.load_op = SDL_GPU_LOADOP_CLEAR;
    targetInfo.store_op = SDL_GPU_STOREOP_STORE;

    SDL_GPURenderPass* pass = SDL_BeginGPURenderPass(cmdbuf, &targetInfo, 1, NULL);

    // Viewport auf die kleine Größe setzen
    SDL_GPUViewport viewport = {0, 0, (float)width, (float)height, 0, 1};
    SDL_SetGPUViewport(pass, &viewport);

    SDL_BindGPUGraphicsPipeline(pass, copyPipeline);
    SDL_GPUTextureSamplerBinding binding = {.texture = render_target, .sampler = samplerClamp};
    SDL_BindGPUFragmentSamplers(pass, 0, &binding, 1);

    SDL_DrawGPUPrimitives(pass, 3, 1, 0, 0);
    SDL_EndGPURenderPass(pass);
    SDL_SubmitGPUCommandBuffer(cmdbuf);

    gpu->downloadTexture(thumbTex, width, height, img);
    gpu->destroyGPUTexture(thumbTex);
    return img;
}

void ::GameRenderer::setLogicalRenderSize(int screenWidth, int screenHeight)
{
    batcher.setLogicalRenderSize(screenWidth, screenHeight);
}

void GameRenderer::setLogicalRenderSize(const ppl7::grafix::Size& size)
{
    batcher.setLogicalRenderSize(size.width, size.height);
}

void GameRenderer::startRenderPass()
{
    batcher.startRenderPass();
}

void GameRenderer::endRenderPass(SDL_GPUTexture* target_texture, SDL_GPULoadOp loadOp, const ppl7::grafix::Color& clearColor)
{
    batcher.prepareInstanceData(cmdbuf);
    SDL_GPUColorTargetInfo colorTargetInfo = {0};
    colorTargetInfo.texture = target_texture;
    colorTargetInfo.clear_color = toSDLFColor(clearColor);
    colorTargetInfo.load_op = loadOp;
    colorTargetInfo.store_op = SDL_GPU_STOREOP_STORE;
    colorTargetInfo.cycle = false; // CRITICAL: SDL examples use false!

    SDL_GPURenderPass* renderPass = SDL_BeginGPURenderPass(cmdbuf, &colorTargetInfo, 1, NULL);
    // SDL_GPURenderPass* renderPass = SDL_BeginGPURenderPass(renderstate.cmdbuf, &colorTargetInfo, 1, NULL);
    SDL_SetGPUViewport(renderPass, NULL);
    SDL_SetGPUScissor(renderPass, NULL);
    batcher.endRenderPass(cmdbuf, renderPass);
    SDL_EndGPURenderPass(renderPass);
}

void GameRenderer::addSprite(
    const SpriteTexture& sprite, int id, float x, float y, float scale_x, float scale_y, float angle, const ppl7::grafix::Color& color)
{
    batcher.addSpriteInternal(sprite, id, x, y, scale_x, scale_y, angle, color, false);
}

void GameRenderer::addSpriteOutline(
    const SpriteTexture& sprite, int id, float x, float y, float scale_x, float scale_y, float angle, const ppl7::grafix::Color& color)
{
    batcher.addSpriteInternal(sprite, id, x, y, scale_x, scale_y, angle, color, true);
}

void GameRenderer::addLine(float x1, float y1, float x2, float y2, const ppl7::grafix::Color& color, int thickness)
{
    batcher.addLine(x1, y1, x2, y2, color, thickness);
}
void GameRenderer::addRect(float x, float y, float w, float h, const ppl7::grafix::Color& color, int thickness)
{
    batcher.addRect(x, y, w, h, color, thickness);
}
void GameRenderer::addFilledRect(float x, float y, float w, float h, const ppl7::grafix::Color& color)
{
    batcher.addFilledRect(x, y, w, h, color);
}
