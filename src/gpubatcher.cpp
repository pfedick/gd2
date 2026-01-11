#define _USE_MATH_DEFINES
#include <math.h>
#include <ppl7.h>
#include <ppl7-grafix.h>
#include "gpu.h"
#include "sprite.h"


GPUBatcher::GPUBatcher()
{
    z = 0.0f;
    gpu = nullptr;
    screenWidth = 1920;
    screenHeight = 1080;
    fragShader = nullptr;
    vertShader = nullptr;
    spritePipeline = nullptr;
    sampler = nullptr;
    vertexBuffer = nullptr;
    indexBuffer = nullptr;
    storageBuffer = nullptr;
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
    z = 0.5f;
}

void GPUBatcher::prepareInstanceData(SDL_GPUCommandBuffer* cmd)
{
    if (!gpu || !gpu->gpu || !storageBuffer) {
        return;
    }

    const size_t MAX_INSTANCES_PER_BATCH = 16384;
    instanceData.clear();
    // Pre-allocate to avoid reallocations, but don't shrink if it's already large enough
    if (instanceData.capacity() < MAX_INSTANCES_PER_BATCH) {
        instanceData.reserve(MAX_INSTANCES_PER_BATCH);
    }

    // Upload instance data for all sprite batches
    for (const auto& [textureId, spriteList] : spriteCommands) {
        if (spriteList.empty()) continue;

        // Collect instance data
        for (const SpriteCommand& spriteCmd : spriteList) {
            const SpriteTexture::SpriteIndexItem* item = spriteCmd.sprite->getSpriteIndex(spriteCmd.sprite_id);
            if (item) {
                // Determine Pivot Point in Texture Coordinates (Pixels relative to Texture Top-Left)
                // Legacy: center = Pivot - Offset
                float pivot_pixel_x = (float)item->Pivot.x - (float)item->Offset.x;
                float pivot_pixel_y = (float)item->Pivot.y - (float)item->Offset.y;

                // Set world position to the desired Pivot Point (x,y)
                // The shader assumes 'pos' is the center of rotation/scaling.
                float world_x = spriteCmd.x;
                float world_y = spriteCmd.y;

                // X: 0..W -> -1..1
                float ndc_x = (world_x * 2.0f / screenWidth) - 1.0f;
                // Y: +1 (Top) ... -1 (Bottom)
                float ndc_y = 1.0f - (world_y * 2.0f / screenHeight);

                float rad = spriteCmd.angle * (M_PI / 180.0f);
                float c = cosf(rad);
                float s = sinf(rad);

                // Pixel dimensions of the sprite
                float sw = (float)item->r.w * spriteCmd.scale_x;
                float sh = (float)item->r.h * spriteCmd.scale_y;

                // Matrix calculation to transform (0..1) unit vector to Rotated NDC
                float m00 = (2.0f / screenWidth) * sw * c;
                float m01 = (2.0f / screenWidth) * sh * (-s);
                float m10 = (-2.0f / screenHeight) * sw * s;
                float m11 = (-2.0f / screenHeight) * sh * c;

                SpriteInstance inst;
                inst.pos_x = ndc_x;
                inst.pos_y = ndc_y;

                inst.m00 = m00;
                inst.m01 = m01;
                inst.m10 = m10;
                inst.m11 = m11;

                inst.pos_z = spriteCmd.z;
                inst.pad2 = 0.0f;

                inst.uv_x = item->uv.x;
                inst.uv_y = item->uv.y;
                inst.uv_w = item->uv.w;
                inst.uv_h = item->uv.h;

                // Normalize pivot relative to texture size
                inst.pivot_x = (item->r.w > 0) ? pivot_pixel_x / (float)item->r.w : 0.0f;
                inst.pivot_y = (item->r.h > 0) ? pivot_pixel_y / (float)item->r.h : 0.0f;

                inst.offset_x = 0.0f;
                inst.offset_y = 0.0f;

                inst.color_r = (float)spriteCmd.color_modulation.red() / 255.0f;
                inst.color_g = (float)spriteCmd.color_modulation.green() / 255.0f;
                inst.color_b = (float)spriteCmd.color_modulation.blue() / 255.0f;
                inst.color_a = (float)spriteCmd.color_modulation.alpha() / 255.0f;

                instanceData.push_back(inst);
            }
        }
    }

    if (instanceData.empty()) {
        return;
    }

    // Upload ALL instance data at once
    size_t instanceDataSize = sizeof(SpriteInstance) * instanceData.size();

    // Check if we need to resize the storage buffer
    if (instanceDataSize > storageBufferCapacity) {
        if (storageBuffer) {
            SDL_ReleaseGPUBuffer(gpu->gpu, storageBuffer);
        }
        // Grow by power of 2 or large chunks
        storageBufferCapacity = (Uint32)(instanceDataSize * 1.5);
        SDL_GPUBufferCreateInfo storageBufferInfo = {
            .usage = SDL_GPU_BUFFERUSAGE_GRAPHICS_STORAGE_READ,
            .size = storageBufferCapacity,
        };
        storageBuffer = SDL_CreateGPUBuffer(gpu->gpu, &storageBufferInfo);
        if (!storageBuffer) {
            ppl7::PrintDebugTime("ERROR: Failed to resize storage buffer to %u bytes\n", storageBufferCapacity);
            return;
        }
        ppl7::PrintDebugTime("Resized Storage Buffer to %u bytes (%zu sprites)\n", storageBufferCapacity, storageBufferCapacity / sizeof(SpriteInstance));
    }

    SDL_GPUTransferBufferCreateInfo transferInfo = {
        .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
        .size = (Uint32)instanceDataSize
    };
    SDL_GPUTransferBuffer* transferBuffer = SDL_CreateGPUTransferBuffer(gpu->gpu, &transferInfo);
    if (transferBuffer) {
        void* mapped = SDL_MapGPUTransferBuffer(gpu->gpu, transferBuffer, false);
        if (mapped) {
            memcpy(mapped, instanceData.data(), instanceDataSize);
            SDL_UnmapGPUTransferBuffer(gpu->gpu, transferBuffer);

            SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(cmd);
            SDL_GPUTransferBufferLocation transferLocation = {
                .transfer_buffer = transferBuffer,
                .offset = 0
            };
            SDL_GPUBufferRegion bufferRegion = {
                .buffer = storageBuffer,
                .offset = 0,
                .size = (Uint32)instanceDataSize
            };
            SDL_UploadToGPUBuffer(copyPass, &transferLocation, &bufferRegion, false);
            SDL_EndGPUCopyPass(copyPass);
        }
        SDL_ReleaseGPUTransferBuffer(gpu->gpu, transferBuffer);
    }
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

    //ppl7::PrintDebugTime("  endRenderPass: Drawing %zu sprites\n", totalSprites);

    // Bind sprite pipeline
    SDL_BindGPUGraphicsPipeline(render_pass, spritePipeline);

    // Bind vertex buffer only
    SDL_GPUBufferBinding vertexBinding = {
        .buffer = vertexBuffer,
        .offset = 0
    };
    SDL_BindGPUVertexBuffers(render_pass, 0, &vertexBinding, 1);

    // Bind storage buffer for sprite instance data
    SDL_BindGPUVertexStorageBuffers(render_pass, 0, &storageBuffer, 1);

    // Draw by texture batch
    Uint32 instanceOffset = 0;

    // Bind index buffer once
    SDL_GPUBufferBinding indexBinding = {
        .buffer = indexBuffer,
        .offset = 0
    };
    SDL_BindGPUIndexBuffer(render_pass, &indexBinding, SDL_GPU_INDEXELEMENTSIZE_16BIT);

    for (const auto& [textureId, spriteList] : spriteCommands) {
        if (spriteList.empty()) continue;

        const SpriteCommand& firstSprite = spriteList.front();
        const SpriteTexture::SpriteIndexItem* indexItem = firstSprite.sprite->getSpriteIndex(firstSprite.sprite_id);
        if (!indexItem || !indexItem->tex) {
            instanceOffset += (Uint32)spriteList.size();
            continue;
        }

        // Bind texture for this batch
        bindTexture(render_pass, indexItem->tex);

        // Draw instances for this batch only
        // first_instance = instanceOffset
        SDL_DrawGPUIndexedPrimitives(render_pass, 6, (Uint32)spriteList.size(), 0, 0, instanceOffset);

        instanceOffset += (Uint32)spriteList.size();
    }

    primitiveCommands.clear();
    spriteCommands.clear();
}



void GPUBatcher::addSprite(const SpriteTexture& sprite, int sprite_id, float x, float y, float scale_x, float scale_y, float angle, const ppl7::grafix::Color& color_modulation)
{
    SpriteCommand cmd(&sprite, sprite_id, x, y, z, scale_x, scale_y, angle, color_modulation);
    z -= 0.0001f; // Slightly increase Z to ensure correct layering
    uint64_t texId = sprite.getUniqueTextureId(sprite_id);
    spriteCommands[texId].push_back(cmd);
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
    // Using NDC version with storage buffer (CPU-side transformation like SimpleQuadTest)
    vertShader = gpu->loadShader("res/shaders/vulkan/sprite_ndc_storage.vert.spv",
        SDL_GPU_SHADERSTAGE_VERTEX,
        0,  // num_samplers
        0,  // num_storage_textures
        1,  // num_storage_buffers
        0); // num_uniform_buffers

    // Load fragment shader
    fragShader = gpu->loadShader("res/shaders/vulkan/sprite_ndc_storage.frag.spv",
        SDL_GPU_SHADERSTAGE_FRAGMENT,
        1,  // num_samplers
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

    // Define vertex attributes (per-vertex data only)
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

    // Define vertex buffer layout (only per-vertex data)
    SDL_GPUVertexBufferDescription vertexBufferDesc[] = {
        // Slot 0: Per-vertex data
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

    // Get swapchain texture format from GPUContext's window
    SDL_GPUTextureFormat swapchainFormat = SDL_GetGPUSwapchainTextureFormat(gpu->gpu, gpu->window);
    //ppl7::PrintDebugTime("GPUBatcher: Swapchain format = %d\n", (int)swapchainFormat);

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
            .compare_op = SDL_GPU_COMPAREOP_LESS_OR_EQUAL,
            .enable_depth_test = true,
            .enable_depth_write = true,
        },
        .target_info = {
            .color_target_descriptions = &colorTarget,
            .num_color_targets = 1,
            .depth_stencil_format = SDL_GPU_TEXTUREFORMAT_D16_UNORM,
            .has_depth_stencil_target = true
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

    // Create storage buffer for sprite instance data (GPU-readable)
    // Support up to 16K sprites per batch (should handle most cases)
    storageBufferCapacity = sizeof(SpriteInstance) * 16384;
    SDL_GPUBufferCreateInfo storageBufferInfo = {
        .usage = SDL_GPU_BUFFERUSAGE_GRAPHICS_STORAGE_READ,
        .size = storageBufferCapacity,
    };
    storageBuffer = SDL_CreateGPUBuffer(gpu->gpu, &storageBufferInfo);
    if (!storageBuffer) {
        throw GPUException("Failed to create storage buffer: %s", SDL_GetError());
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

    this->screenWidth = screenWidth;
    this->screenHeight = screenHeight;
    //ppl7::PrintDebugTime("GPUBatcher::updateMatrices: screen %dx%d\n", screenWidth, screenHeight);

    // Create orthographic projection matrix for 2D rendering (Vulkan coordinate system)
    // Maps screen coordinates (0,0) top-left to (screenWidth, screenHeight) bottom-right to NDC (-1,-1) to (1,1)
    // Note: Vulkan's Y-axis points DOWN in screen space, so we flip it in the projection
    float left = 0.0f;
    float right = (float)screenWidth;
    float top = (float)screenHeight;  // Vulkan: top > bottom for proper Y-flip
    float bottom = 0.0f;
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
    if (storageBuffer) {
        SDL_ReleaseGPUBuffer(gpu->gpu, storageBuffer);
        storageBuffer = nullptr;
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

}

void GPUBatcher::drawSprites(SDL_GPUCommandBuffer* cmd, SDL_GPURenderPass* render_pass, const std::list<SpriteCommand>& sprites)
{
    if (sprites.empty() || !vertexBuffer || !indexBuffer || !storageBuffer) return;

    // Bind static buffers (vertex and index data already uploaded at init)
    SDL_GPUBufferBinding vertexBinding = { .buffer = vertexBuffer, .offset = 0 };
    SDL_BindGPUVertexBuffers(render_pass, 0, &vertexBinding, 1);

    SDL_GPUBufferBinding indexBinding = { .buffer = indexBuffer, .offset = 0 };
    SDL_BindGPUIndexBuffer(render_pass, &indexBinding, SDL_GPU_INDEXELEMENTSIZE_16BIT);

    // Bind storage buffer to Slot 0 (default)
    SDL_BindGPUVertexStorageBuffers(render_pass, 0, &storageBuffer, 1);

    // Draw all instances
    SDL_DrawGPUIndexedPrimitives(render_pass, 6, (Uint32)sprites.size(), 0, 0, 0);

}
