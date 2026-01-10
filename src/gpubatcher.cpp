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
    instanceBuffer = nullptr;
    uniformBuffer = nullptr;
    memset(&currentUniforms, 0, sizeof(UniformData));
}

GPUBatcher::~GPUBatcher()
{
    cleanup();
}

void GPUBatcher::init(GPUContext* gpu)
{
    this->gpu = gpu;
    loadShaders();
    ppl7::PrintDebugTime("GPUBatcher::init - Shaders loaded: vert=%p, frag=%p\n", vertShader, fragShader);
    createPipeline();
    ppl7::PrintDebugTime("GPUBatcher::init - Pipeline created: %p\n", spritePipeline);
    createBuffers();
    ppl7::PrintDebugTime("GPUBatcher::init - Buffers created: vert=%p, index=%p, instance=%p\n",
        vertexBuffer, indexBuffer, instanceBuffer);
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

void GPUBatcher::prepareInstanceData(SDL_GPUCommandBuffer* cmd)
{
    if (!gpu || !gpu->gpu || !instanceBuffer) {
        ppl7::PrintDebugTime("ERROR: prepareInstanceData - missing resources: gpu=%p, instanceBuffer=%p\n", gpu, instanceBuffer);
        return;
    }

    ppl7::PrintDebugTime("GPUBatcher::prepareInstanceData: %zu texture batches\n", spriteCommands.size());

    // Collect ALL sprites into one big list (ignoring texture batching for now)
    const size_t MAX_INSTANCES_PER_BATCH = 16384;
    std::vector<SpriteInstance> instances;
    instances.reserve(MAX_INSTANCES_PER_BATCH);

    // Upload instance data for all sprite batches
    for (const auto& [textureId, spriteList] : spriteCommands) {
        if (spriteList.empty()) continue;

        ppl7::PrintDebugTime("  Processing batch for texture %llu: %zu sprites\n", textureId, spriteList.size());

        // Collect instance data
        for (const SpriteCommand& spriteCmd : spriteList) {
            const SpriteTexture::SpriteIndexItem* item = spriteCmd.sprite->getSpriteIndex(spriteCmd.sprite_id);
            if (item) {
                SpriteInstance inst;
                inst.pos_x = spriteCmd.x;
                inst.pos_y = spriteCmd.y;
                inst.size_w = (float)item->r.w;
                inst.size_h = (float)item->r.h;
                inst.scale_x = spriteCmd.scale_x;
                inst.scale_y = spriteCmd.scale_y;
                inst.angle = spriteCmd.angle;
                inst.uv_x = item->uv.x;
                inst.uv_y = item->uv.y;
                inst.uv_w = item->uv.w;
                inst.uv_h = item->uv.h;
                inst.pivot_x = (float)item->Pivot.x;
                inst.pivot_y = (float)item->Pivot.y;
                inst.offset_x = (float)item->Offset.x;
                inst.offset_y = (float)item->Offset.y;
                instances.push_back(inst);
            }
        }
    }

    if (instances.empty()) {
        ppl7::PrintDebugTime("  No instances to upload!\n");
        return;
    }

    ppl7::PrintDebugTime("  Uploading %zu total instances\n", instances.size());
    ppl7::PrintDebugTime("  First sprite: pos=(%.1f,%.1f) size=(%.1f,%.1f) uv=(%.3f,%.3f,%.3f,%.3f)\n",
        instances[0].pos_x, instances[0].pos_y,
        instances[0].size_w, instances[0].size_h,
        instances[0].uv_x, instances[0].uv_y, instances[0].uv_w, instances[0].uv_h);

    // Upload ALL instance data at once
    size_t instanceDataSize = sizeof(SpriteInstance) * instances.size();
    SDL_GPUTransferBufferCreateInfo transferInfo = {
        .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
        .size = (Uint32)instanceDataSize
    };
    SDL_GPUTransferBuffer* transferBuffer = SDL_CreateGPUTransferBuffer(gpu->gpu, &transferInfo);
    if (transferBuffer) {
        void* mapped = SDL_MapGPUTransferBuffer(gpu->gpu, transferBuffer, false);
        if (mapped) {
            memcpy(mapped, instances.data(), instanceDataSize);
            SDL_UnmapGPUTransferBuffer(gpu->gpu, transferBuffer);

            SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(cmd);
            SDL_GPUTransferBufferLocation transferLocation = {
                .transfer_buffer = transferBuffer,
                .offset = 0
            };
            SDL_GPUBufferRegion bufferRegion = {
                .buffer = instanceBuffer,
                .offset = 0,
                .size = (Uint32)instanceDataSize
            };
            SDL_UploadToGPUBuffer(copyPass, &transferLocation, &bufferRegion, false);
            SDL_EndGPUCopyPass(copyPass);
        }
        SDL_ReleaseGPUTransferBuffer(gpu->gpu, transferBuffer);
    }

    ppl7::PrintDebugTime("  prepareInstanceData complete\n");
}

void GPUBatcher::endRenderPass(SDL_GPUCommandBuffer* cmd, SDL_GPURenderPass* render_pass)
{
    if (!spritePipeline) return;

    // Count total sprites
    size_t totalSprites = 0;
    for (const auto& [textureId, spriteList] : spriteCommands) {
        totalSprites += spriteList.size();
    }

    if (totalSprites == 0) {
        spriteCommands.clear();
        return;
    }

    ppl7::PrintDebugTime("  endRenderPass: Drawing %zu sprites\n", totalSprites);

    // Bind sprite pipeline
    SDL_BindGPUGraphicsPipeline(render_pass, spritePipeline);

    // Push uniform data (projection/view matrices)
    ppl7::PrintDebugTime("    Pushing matrices: proj[0]=%.3f, proj[5]=%.3f, proj[12]=%.3f, proj[13]=%.3f\n",
        currentUniforms.projection[0], currentUniforms.projection[5],
        currentUniforms.projection[12], currentUniforms.projection[13]);
    SDL_PushGPUVertexUniformData(cmd, 0, &currentUniforms, sizeof(UniformData));

    // Bind vertex and instance buffers
    SDL_GPUBufferBinding vertexBinding = {
        .buffer = vertexBuffer,
        .offset = 0
    };
    SDL_BindGPUVertexBuffers(render_pass, 0, &vertexBinding, 1);

    SDL_GPUBufferBinding instanceBinding = {
        .buffer = instanceBuffer,
        .offset = 0
    };
    SDL_BindGPUVertexBuffers(render_pass, 1, &instanceBinding, 1);

    // For now: bind first texture we find and draw ALL sprites
    // TODO: Proper multi-texture batching later
    for (const auto& [textureId, spriteList] : spriteCommands) {
        if (spriteList.empty()) continue;

        const SpriteCommand& firstSprite = spriteList.front();
        const SpriteTexture::SpriteIndexItem* indexItem = firstSprite.sprite->getSpriteIndex(firstSprite.sprite_id);
        if (!indexItem || !indexItem->tex) continue;

        // Bind texture
        ppl7::PrintDebugTime("    Binding texture %p\n", indexItem->tex);
        bindTexture(render_pass, indexItem->tex);

        // Draw ALL sprites (since we uploaded them all together)
        SDL_GPUBufferBinding indexBinding = {
            .buffer = indexBuffer,
            .offset = 0
        };
        SDL_BindGPUIndexBuffer(render_pass, &indexBinding, SDL_GPU_INDEXELEMENTSIZE_16BIT);

        ppl7::PrintDebugTime("    DrawIndexedPrimitives: indices=6, instances=%zu\n", totalSprites);
        SDL_DrawGPUIndexedPrimitives(render_pass, 6, (Uint32)totalSprites, 0, 0, 0);

        // Only draw once with first texture found
        break;
    }

    primitiveCommands.clear();
    spriteCommands.clear();
}



void GPUBatcher::addSprite(const SpriteTexture& sprite, int sprite_id, float x, float y, float scale_x, float scale_y, float angle, const ppl7::grafix::Color& color_modulation)
{
    SpriteCommand cmd(&sprite, sprite_id, x, y, z, scale_x, scale_y, angle, color_modulation);
    z += 0.0001f; // Slightly increase Z to ensure correct layering
    uint64_t texId = sprite.getUniqueTextureId(sprite_id);
    spriteCommands[texId].push_back(cmd);

    static int debug_counter = 0;
    if (debug_counter++ < 5) {
        ppl7::PrintDebugTime("GPUBatcher::addSprite: id=%d, pos=(%.1f,%.1f), texId=%llu\n", sprite_id, x, y, texId);
    }
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
        0); // num_uniform_buffers (using push constants instead)

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
        // Per-vertex attributes (slot 0)
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
        },
        // Per-instance attributes (slot 1)
        // Sprite position (location 3)
        {
            .location = 3,
            .buffer_slot = 1,
            .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2,
            .offset = 0
        },
        // Sprite size (location 4)
        {
            .location = 4,
            .buffer_slot = 1,
            .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2,
            .offset = sizeof(float) * 2
        },
        // Sprite scale (location 5)
        {
            .location = 5,
            .buffer_slot = 1,
            .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2,
            .offset = sizeof(float) * 4
        },
        // Sprite angle (location 6)
        {
            .location = 6,
            .buffer_slot = 1,
            .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT,
            .offset = sizeof(float) * 6
        },
        // Sprite UV rect (location 7)
        {
            .location = 7,
            .buffer_slot = 1,
            .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4,
            .offset = sizeof(float) * 7
        },
        // Sprite pivot (location 8)
        {
            .location = 8,
            .buffer_slot = 1,
            .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2,
            .offset = sizeof(float) * 11
        },
        // Sprite offset (location 9)
        {
            .location = 9,
            .buffer_slot = 1,
            .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2,
            .offset = sizeof(float) * 13
        }
    };

    // Define vertex buffer layout
    SDL_GPUVertexBufferDescription vertexBufferDesc[] = {
        // Slot 0: Per-vertex data
        {
            .slot = 0,
            .pitch = sizeof(Vertex),
            .input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX,
            .instance_step_rate = 0
        },
        // Slot 1: Per-instance data
        {
            .slot = 1,
            .pitch = sizeof(SpriteInstance),
            .input_rate = SDL_GPU_VERTEXINPUTRATE_INSTANCE,
            .instance_step_rate = 1
        }
    };

    SDL_GPUVertexInputState vertexInputState = {
        .vertex_buffer_descriptions = vertexBufferDesc,
        .num_vertex_buffers = 2,
        .vertex_attributes = vertexAttributes,
        .num_vertex_attributes = 10
    };

    // Get swapchain texture format from GPUContext's window
    SDL_GPUTextureFormat swapchainFormat = SDL_GetGPUSwapchainTextureFormat(gpu->gpu, gpu->window);
    ppl7::PrintDebugTime("GPUBatcher: Swapchain format = %d\n", (int)swapchainFormat);

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

    // Create vertex buffer (for single quad - reused for all sprites)
    SDL_GPUBufferCreateInfo vertexBufferInfo = {
        .usage = SDL_GPU_BUFFERUSAGE_VERTEX,
        .size = sizeof(Vertex) * 4,  // Single quad only
    };
    vertexBuffer = SDL_CreateGPUBuffer(gpu->gpu, &vertexBufferInfo);
    if (!vertexBuffer) {
        throw GPUException("Failed to create vertex buffer: %s", SDL_GetError());
    }

    // Create index buffer (for single quad - reused for all sprites)
    SDL_GPUBufferCreateInfo indexBufferInfo = {
        .usage = SDL_GPU_BUFFERUSAGE_INDEX,
        .size = sizeof(Uint16) * 6,  // Single quad only
    };
    indexBuffer = SDL_CreateGPUBuffer(gpu->gpu, &indexBufferInfo);
    if (!indexBuffer) {
        throw GPUException("Failed to create index buffer: %s", SDL_GetError());
    }

    // Create instance buffer (for sprite instance data)
    // Support up to 16K sprites per batch (should handle most cases)
    SDL_GPUBufferCreateInfo instanceBufferInfo = {
        .usage = SDL_GPU_BUFFERUSAGE_VERTEX,
        .size = sizeof(SpriteInstance) * 16384,
    };
    instanceBuffer = SDL_CreateGPUBuffer(gpu->gpu, &instanceBufferInfo);
    if (!instanceBuffer) {
        throw GPUException("Failed to create instance buffer: %s", SDL_GetError());
    }

    // Note: Uniform data will be pushed via SDL_PushGPUVertexUniformData, no buffer needed

    // Upload static quad data once (never changes)
    uploadStaticQuadData();
}

void GPUBatcher::uploadStaticQuadData()
{
    if (!gpu || !gpu->gpu || !vertexBuffer || !indexBuffer) return;

    // Unit quad vertices (0-1 range, transformed by instance data in shader)
    Vertex quadVertices[4] = {
        { 0.0f, 0.0f,  0.0f, 0.0f,  1.0f, 1.0f, 1.0f, 1.0f },  // Top-left
        { 1.0f, 0.0f,  1.0f, 0.0f,  1.0f, 1.0f, 1.0f, 1.0f },  // Top-right
        { 0.0f, 1.0f,  0.0f, 1.0f,  1.0f, 1.0f, 1.0f, 1.0f },  // Bottom-left
        { 1.0f, 1.0f,  1.0f, 1.0f,  1.0f, 1.0f, 1.0f, 1.0f }   // Bottom-right
    };

    // Quad indices (two triangles with counter-clockwise winding)
    // First triangle: 0→2→1 (top-left → bottom-left → top-right)
    // Second triangle: 1→2→3 (top-right → bottom-left → bottom-right)
    Uint16 quadIndices[6] = { 0, 2, 1, 1, 2, 3 };

    SDL_GPUCommandBuffer* cmd = SDL_AcquireGPUCommandBuffer(gpu->gpu);
    if (!cmd) return;

    // Upload vertices
    SDL_GPUTransferBufferCreateInfo transferInfo = {
        .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
        .size = sizeof(quadVertices)
    };
    SDL_GPUTransferBuffer* transferBuffer = SDL_CreateGPUTransferBuffer(gpu->gpu, &transferInfo);
    if (transferBuffer) {
        void* mapped = SDL_MapGPUTransferBuffer(gpu->gpu, transferBuffer, false);
        if (mapped) {
            memcpy(mapped, quadVertices, sizeof(quadVertices));
            SDL_UnmapGPUTransferBuffer(gpu->gpu, transferBuffer);

            SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(cmd);
            SDL_GPUTransferBufferLocation transferLocation = { .transfer_buffer = transferBuffer, .offset = 0 };
            SDL_GPUBufferRegion bufferRegion = { .buffer = vertexBuffer, .offset = 0, .size = sizeof(quadVertices) };
            SDL_UploadToGPUBuffer(copyPass, &transferLocation, &bufferRegion, false);
            SDL_EndGPUCopyPass(copyPass);
        }
        SDL_ReleaseGPUTransferBuffer(gpu->gpu, transferBuffer);
    }

    // Upload indices
    SDL_GPUTransferBufferCreateInfo indexTransferInfo = {
        .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
        .size = sizeof(quadIndices)
    };
    SDL_GPUTransferBuffer* indexTransferBuffer = SDL_CreateGPUTransferBuffer(gpu->gpu, &indexTransferInfo);
    if (indexTransferBuffer) {
        void* mapped = SDL_MapGPUTransferBuffer(gpu->gpu, indexTransferBuffer, false);
        if (mapped) {
            memcpy(mapped, quadIndices, sizeof(quadIndices));
            SDL_UnmapGPUTransferBuffer(gpu->gpu, indexTransferBuffer);

            SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(cmd);
            SDL_GPUTransferBufferLocation transferLocation = { .transfer_buffer = indexTransferBuffer, .offset = 0 };
            SDL_GPUBufferRegion bufferRegion = { .buffer = indexBuffer, .offset = 0, .size = sizeof(quadIndices) };
            SDL_UploadToGPUBuffer(copyPass, &transferLocation, &bufferRegion, false);
            SDL_EndGPUCopyPass(copyPass);
        }
        SDL_ReleaseGPUTransferBuffer(gpu->gpu, indexTransferBuffer);
    }

    SDL_SubmitGPUCommandBuffer(cmd);
    SDL_WaitForGPUIdle(gpu->gpu);
}

void GPUBatcher::updateMatrices(int screenWidth, int screenHeight)
{
    if (!gpu || !gpu->gpu) return;

    ppl7::PrintDebugTime("GPUBatcher::updateMatrices: screen %dx%d\n", screenWidth, screenHeight);

    // Create orthographic projection matrix for 2D rendering
    // Maps screen coordinates (0,0) to (screenWidth, screenHeight) to NDC (-1,-1) to (1,1)
    float left = 0.0f;
    float right = (float)screenWidth;
    float bottom = 0.0f;
    float top = (float)screenHeight;
    float near = -1.0f;
    float far = 1.0f;

    // Column-major orthographic projection matrix
    currentUniforms.projection[0] = 2.0f / (right - left);
    currentUniforms.projection[1] = 0.0f;
    currentUniforms.projection[2] = 0.0f;
    currentUniforms.projection[3] = 0.0f;

    currentUniforms.projection[4] = 0.0f;
    currentUniforms.projection[5] = 2.0f / (top - bottom);
    currentUniforms.projection[6] = 0.0f;
    currentUniforms.projection[7] = 0.0f;

    currentUniforms.projection[8] = 0.0f;
    currentUniforms.projection[9] = 0.0f;
    currentUniforms.projection[10] = -2.0f / (far - near);
    currentUniforms.projection[11] = 0.0f;

    currentUniforms.projection[12] = -(right + left) / (right - left);
    currentUniforms.projection[13] = -(top + bottom) / (top - bottom);
    currentUniforms.projection[14] = -(far + near) / (far - near);
    currentUniforms.projection[15] = 1.0f;

    // Identity view matrix (no camera transform for now)
    for (int i = 0; i < 16; i++) {
        currentUniforms.view[i] = (i % 5 == 0) ? 1.0f : 0.0f;
    }

    // Debug: Print first row of matrices
    ppl7::PrintDebugTime("  Projection[0-3]: %.3f, %.3f, %.3f, %.3f\n",
        currentUniforms.projection[0], currentUniforms.projection[1],
        currentUniforms.projection[2], currentUniforms.projection[3]);
    ppl7::PrintDebugTime("  Projection[12-15]: %.3f, %.3f, %.3f, %.3f\n",
        currentUniforms.projection[12], currentUniforms.projection[13],
        currentUniforms.projection[14], currentUniforms.projection[15]);
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
    if (instanceBuffer) {
        SDL_ReleaseGPUBuffer(gpu->gpu, instanceBuffer);
        instanceBuffer = nullptr;
    }
    // uniformBuffer removed - using push constants instead
    if (vertShader) {
        gpu->releaseShader(vertShader);
        vertShader = nullptr;
    }
    if (fragShader) {
        gpu->releaseShader(fragShader);
        fragShader = nullptr;
    }
}
void GPUBatcher::bindTexture(SDL_GPURenderPass* render_pass, SDL_GPUTexture* texture)
{
    if (!texture || !sampler) {
        ppl7::PrintDebugTime("  ERROR: bindTexture failed - texture=%p, sampler=%p\n", texture, sampler);
        return;
    }

    // Bind texture and sampler for fragment shader
    SDL_GPUTextureSamplerBinding textureSamplerBinding = {
        .texture = texture,
        .sampler = sampler
    };

    SDL_BindGPUFragmentSamplers(render_pass, 0, &textureSamplerBinding, 1);

    static int bind_count = 0;
    if (bind_count++ < 3) {
        ppl7::PrintDebugTime("  Texture bound: %p\n", texture);
    }
}

void GPUBatcher::drawSprites(SDL_GPUCommandBuffer* cmd, SDL_GPURenderPass* render_pass, const std::list<SpriteCommand>& sprites)
{
    if (sprites.empty() || !vertexBuffer || !indexBuffer || !instanceBuffer) return;

    ppl7::PrintDebugTime("GPUBatcher::drawSprites: %zu sprites\n", sprites.size());

    // Bind static buffers (vertex and index data already uploaded at init)
    SDL_GPUBufferBinding vertexBinding = { .buffer = vertexBuffer, .offset = 0 };
    SDL_BindGPUVertexBuffers(render_pass, 0, &vertexBinding, 1);

    SDL_GPUBufferBinding indexBinding = { .buffer = indexBuffer, .offset = 0 };
    SDL_BindGPUIndexBuffer(render_pass, &indexBinding, SDL_GPU_INDEXELEMENTSIZE_16BIT);

    // Bind instance buffer (data was uploaded before render pass)
    SDL_GPUBufferBinding instanceBinding = { .buffer = instanceBuffer, .offset = 0 };
    SDL_BindGPUVertexBuffers(render_pass, 1, &instanceBinding, 1);

    // Draw all instances
    SDL_DrawGPUIndexedPrimitives(render_pass, 6, (Uint32)sprites.size(), 0, 0, 0);

    ppl7::PrintDebugTime("  DrawIndexedPrimitives: indices=6, instances=%zu\n", sprites.size());
}
