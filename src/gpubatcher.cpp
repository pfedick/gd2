#include <ppl7.h>
#include <ppl7-grafix.h>
#include "gpu.h"
#include "sprite.h"


GPUBatcher::GPUBatcher()
{
    z = 0.0f;
    gpu = nullptr;
    fragShader = nullptr;
    vertShader = nullptr;
    spritePipeline = nullptr;
    sampler = nullptr;
    vertexBuffer = nullptr;
    indexBuffer = nullptr;
}

GPUBatcher::~GPUBatcher()
{
    cleanup();
}

void GPUBatcher::init(GPUContext* gpu)
{
    this->gpu = gpu;
    loadShaders();
    createPipeline();
    createBuffers();
}

void GPUBatcher::clearQueues()
{
    primitiveCommands.clear();
    spriteCommands.clear();
}

void GPUBatcher::startRenderPass()
{
    z = 0.0f;
}

void GPUBatcher::endRenderPass(SDL_GPUCommandBuffer* cmd, SDL_GPURenderPass* render_pass)
{
    // Primitive zeichnen
    // TODO
    for (const PrimitiveCommand& prim : primitiveCommands) {
        switch (prim.type) {
        case PrimitiveCommand::Type::Line:
            // TODO : SDL_DrawGPULine existiert nicht, evtl. eigene Implementierung
            break;
        case PrimitiveCommand::Type::Rect:
            // TODO: Rechteck zeichnen
            break;
        case PrimitiveCommand::Type::FilledRect:
            // TODO: Gefülltes Rechteck zeichnen
            break;
        }
    }

    // Sprites zeichnen
    for (const auto& [key, spriteList] : spriteCommands) {
        // Batching der Sprites mit gleicher Textur
        // TODO: Textur binden

        // Jetzt über die Sprites mit dieser Textur iterieren
        for (const SpriteCommand& spriteCmd : spriteList) {
            // TODO: Sprite zeichnen
        }
    }
    primitiveCommands.clear();
    spriteCommands.clear();

}



void GPUBatcher::addSprite(const SpriteTexture& sprite, int sprite_id, float x, float y, float scale_x, float scale_y, float angle, const ppl7::grafix::Color& color_modulation)
{
    SpriteCommand cmd(&sprite, sprite_id, x, y, z, scale_x, scale_y, angle, color_modulation);
    z += 0.0001f; // Slightly increase Z to ensure correct layering
    spriteCommands[sprite.getUniqueTextureId(sprite_id)].push_back(cmd);
}

void GPUBatcher::addLine(float x1, float y1, float x2, float y2, const ppl7::grafix::Color& color, float thickness)
{
    PrimitiveCommand cmd(PrimitiveCommand::Type::Line, x1, y1, x2, y2, color, thickness);
    primitiveCommands.push_back(cmd);
}

void GPUBatcher::addRect(float x, float y, float w, float h, const ppl7::grafix::Color& color, float thickness)
{
    PrimitiveCommand cmd(PrimitiveCommand::Type::Rect, x, y, w, h, color);
    primitiveCommands.push_back(cmd);
}
void GPUBatcher::addFilledRect(float x, float y, float w, float h, const ppl7::grafix::Color& color)
{
    PrimitiveCommand cmd(PrimitiveCommand::Type::FilledRect, x, y, w, h, color);
    primitiveCommands.push_back(cmd);
}

void GPUBatcher::loadShaders()
{
    if (!gpu || !gpu->gpu) {
        throw GPUException("GPUContext not initialized");
    }

    // Load vertex shader (SPIR-V compiled from GLSL)
    // Note: Shaders need to be compiled to SPIR-V first using glslc or similar
    vertShader = gpu->loadShader("res/shaders/vulkan/sprite.vert.spv",
        SDL_GPU_SHADERSTAGE_VERTEX,
        0,  // num_samplers
        0,  // num_storage_textures
        0,  // num_storage_buffers
        1); // num_uniform_buffers

    // Load fragment shader
    fragShader = gpu->loadShader("res/shaders/vulkan/sprite.frag.spv",
        SDL_GPU_SHADERSTAGE_FRAGMENT,
        1,  // num_samplers (texture sampler)
        0,  // num_storage_textures
        0,  // num_storage_buffers
        0); // num_uniform_buffers
}

void GPUBatcher::createPipeline()
{
    if (!gpu || !gpu->gpu || !vertShader || !fragShader) {
        throw GPUException("Shaders not loaded");
    }

    // Create sampler for texture sampling
    SDL_GPUSamplerCreateInfo samplerInfo = {
        .min_filter = SDL_GPU_FILTER_LINEAR,
        .mag_filter = SDL_GPU_FILTER_LINEAR,
        .mipmap_mode = SDL_GPU_SAMPLERMIPMAPMODE_LINEAR,
        .address_mode_u = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
        .address_mode_v = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
        .address_mode_w = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
    };
    sampler = SDL_CreateGPUSampler(gpu->gpu, &samplerInfo);
    if (!sampler) {
        throw GPUException("Failed to create sampler: %s", SDL_GetError());
    }

    // Define vertex attributes
    SDL_GPUVertexAttribute vertexAttributes[] = {
        // Position (location 0)
        {
            .location = 0,
            .buffer_slot = 0,
            .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2,
            .offset = 0
        },
        // Texcoord (location 1)
        {
            .location = 1,
            .buffer_slot = 0,
            .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2,
            .offset = sizeof(float) * 2
        },
        // Color (location 2)
        {
            .location = 2,
            .buffer_slot = 0,
            .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4,
            .offset = sizeof(float) * 4
        }
    };

    // Define vertex buffer layout
    SDL_GPUVertexBufferDescription vertexBufferDesc[] = {
        {
            .slot = 0,
            .pitch = sizeof(Vertex),
            .input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX,
            .instance_step_rate = 0
        }
    };

    SDL_GPUVertexInputState vertexInputState = {
        .vertex_buffer_descriptions = vertexBufferDesc,
        .num_vertex_buffers = 1,
        .vertex_attributes = vertexAttributes,
        .num_vertex_attributes = 3
    };

    // Get swapchain texture format
    SDL_Window* window = SDL_GetWindowFromID(1); // TODO: Get from GPUContext
    SDL_GPUTextureFormat swapchainFormat = SDL_GetGPUSwapchainTextureFormat(gpu->gpu, window);

    // Color target description
    SDL_GPUColorTargetDescription colorTarget = {
        .format = swapchainFormat,
        .blend_state = {
            .src_color_blendfactor = SDL_GPU_BLENDFACTOR_SRC_ALPHA,
            .dst_color_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
            .color_blend_op = SDL_GPU_BLENDOP_ADD,
            .src_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE,
            .dst_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
            .alpha_blend_op = SDL_GPU_BLENDOP_ADD,
            .color_write_mask = SDL_GPU_COLORCOMPONENT_R | SDL_GPU_COLORCOMPONENT_G |
                                SDL_GPU_COLORCOMPONENT_B | SDL_GPU_COLORCOMPONENT_A,
            .enable_blend = true
        }
    };

    // Create graphics pipeline
    SDL_GPUGraphicsPipelineCreateInfo pipelineInfo = {
        .vertex_shader = vertShader,
        .fragment_shader = fragShader,
        .vertex_input_state = vertexInputState,
        .primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
        .rasterizer_state = {
            .fill_mode = SDL_GPU_FILLMODE_FILL,
            .cull_mode = SDL_GPU_CULLMODE_NONE,
            .front_face = SDL_GPU_FRONTFACE_COUNTER_CLOCKWISE,
        },
        .multisample_state = {
            .sample_count = SDL_GPU_SAMPLECOUNT_1,
        },
        .depth_stencil_state = {
            .enable_depth_test = false,
            .enable_depth_write = false,
        },
        .target_info = {
            .color_target_descriptions = &colorTarget,
            .num_color_targets = 1,
            .has_depth_stencil_target = false
        }
    };

    spritePipeline = SDL_CreateGPUGraphicsPipeline(gpu->gpu, &pipelineInfo);
    if (!spritePipeline) {
        throw GPUException("Failed to create graphics pipeline: %s", SDL_GetError());
    }
}

void GPUBatcher::createBuffers()
{
    if (!gpu || !gpu->gpu) {
        throw GPUException("GPUContext not initialized");
    }

    // Create vertex buffer (for quad vertices)
    SDL_GPUBufferCreateInfo vertexBufferInfo = {
        .usage = SDL_GPU_BUFFERUSAGE_VERTEX,
        .size = sizeof(Vertex) * 4 * 1024,  // Space for 1024 quads
    };
    vertexBuffer = SDL_CreateGPUBuffer(gpu->gpu, &vertexBufferInfo);
    if (!vertexBuffer) {
        throw GPUException("Failed to create vertex buffer: %s", SDL_GetError());
    }

    // Create index buffer (for quad indices)
    SDL_GPUBufferCreateInfo indexBufferInfo = {
        .usage = SDL_GPU_BUFFERUSAGE_INDEX,
        .size = sizeof(Uint16) * 6 * 1024,  // 6 indices per quad, 1024 quads
    };
    indexBuffer = SDL_CreateGPUBuffer(gpu->gpu, &indexBufferInfo);
    if (!indexBuffer) {
        throw GPUException("Failed to create index buffer: %s", SDL_GetError());
    }
}

void GPUBatcher::cleanup()
{
    if (!gpu || !gpu->gpu) return;

    if (spritePipeline) {
        SDL_ReleaseGPUGraphicsPipeline(gpu->gpu, spritePipeline);
        spritePipeline = nullptr;
    }
    if (sampler) {
        SDL_ReleaseGPUSampler(gpu->gpu, sampler);
        sampler = nullptr;
    }
    if (vertexBuffer) {
        SDL_ReleaseGPUBuffer(gpu->gpu, vertexBuffer);
        vertexBuffer = nullptr;
    }
    if (indexBuffer) {
        SDL_ReleaseGPUBuffer(gpu->gpu, indexBuffer);
        indexBuffer = nullptr;
    }
    if (vertShader) {
        gpu->releaseShader(vertShader);
        vertShader = nullptr;
    }
    if (fragShader) {
        gpu->releaseShader(fragShader);
        fragShader = nullptr;
    }
}
