#define _USE_MATH_DEFINES
#include <math.h>
#include <ppl7.h>
#include <ppl7-grafix.h>
#include "gamerenderer.h"
#include "sprite.h"

GPUBatcher::GPUBatcher()
{
    gpu = nullptr;
    screenWidth = 1920;
    screenHeight = 1080;
    fragShader = nullptr;
    vertShader = nullptr;
    primitiveVertShader = nullptr;
    primitiveFragShader = nullptr;
    spritePipeline = nullptr;
    spriteOutlinePipeline = nullptr;
    outlineFragShader = nullptr;
    primitivePipeline = nullptr;
    primitiveFillPipeline = nullptr;
    sampler = nullptr;
    vertexBuffer = nullptr;
    indexBuffer = nullptr;
    storageBuffer = nullptr;
    primitiveVertexBuffer = nullptr;
    storageBufferCapacity = 0;
    primitiveVertexCapacity = 0;
    uniformBuffer = nullptr;
    contextSwitchCount = 0;
    totalPrimitivesCount = 0;
    totalSpriteCount = 0;
    memset(&currentUniforms, 0, sizeof(UniformData));
}

GPUBatcher::~GPUBatcher()
{
    cleanup();
}

void GPUBatcher::init(GPUContext* gpu, SDL_GPUSampler* sampler)
{
    this->gpu = gpu;
    this->sampler = sampler;
    loadShaders();
    createPipeline();
    createBuffers();
}

void GPUBatcher::startRenderPass()
{
    spriteInstances.clear();
    primitiveVertices.clear();
    batches.clear();
}

void GPUBatcher::finishCurrentBatch()
{
    if (batches.empty()) return;

    RenderBatch& current = batches.back();
    if (current.type == BatchType::Sprites) {
        current.count = (uint32_t)spriteInstances.size() - current.offset;
    } else {
        current.count = (uint32_t)primitiveVertices.size() - current.offset;
    }
}

void GPUBatcher::prepareInstanceData(SDL_GPUCommandBuffer* cmd)
{
    if (!gpu || !gpu->gpu || !storageBuffer) return;

    // --- Prepare Sprites ---
    if (!spriteInstances.empty()) {
        size_t dataSize = spriteInstances.size() * sizeof(SpriteInstance);
        if (dataSize > storageBufferCapacity) {
            if (storageBuffer) SDL_ReleaseGPUBuffer(gpu->gpu, storageBuffer);
            storageBufferCapacity = (Uint32)(dataSize * 1.5);
            SDL_GPUBufferCreateInfo desc = {.usage = SDL_GPU_BUFFERUSAGE_GRAPHICS_STORAGE_READ, .size = storageBufferCapacity};
            storageBuffer = SDL_CreateGPUBuffer(gpu->gpu, &desc);
        }

        SDL_GPUTransferBufferCreateInfo tInfo = {.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD, .size = (Uint32)dataSize};
        SDL_GPUTransferBuffer* tBuf = SDL_CreateGPUTransferBuffer(gpu->gpu, &tInfo);
        if (tBuf) {
            void* map = SDL_MapGPUTransferBuffer(gpu->gpu, tBuf, false);
            if (map) {
                memcpy(map, spriteInstances.data(), dataSize);
                SDL_UnmapGPUTransferBuffer(gpu->gpu, tBuf);
                SDL_GPUCopyPass* cp = SDL_BeginGPUCopyPass(cmd);
                SDL_GPUTransferBufferLocation loc = {.transfer_buffer = tBuf, .offset = 0};
                SDL_GPUBufferRegion reg = {.buffer = storageBuffer, .offset = 0, .size = (Uint32)dataSize};
                SDL_UploadToGPUBuffer(cp, &loc, &reg, false);
                SDL_EndGPUCopyPass(cp);
            }
            SDL_ReleaseGPUTransferBuffer(gpu->gpu, tBuf);
        }
    }

    // --- Prepare Primitives ---
    if (!primitiveVertices.empty()) {
        size_t dataSize = primitiveVertices.size() * sizeof(PrimitiveVertex);
        if (dataSize > primitiveVertexCapacity) {
            if (primitiveVertexBuffer) SDL_ReleaseGPUBuffer(gpu->gpu, primitiveVertexBuffer);
            primitiveVertexCapacity = (Uint32)(dataSize * 1.5);
            SDL_GPUBufferCreateInfo desc = {.usage = SDL_GPU_BUFFERUSAGE_VERTEX, .size = primitiveVertexCapacity};
            primitiveVertexBuffer = SDL_CreateGPUBuffer(gpu->gpu, &desc);
        }

        SDL_GPUTransferBufferCreateInfo tInfo = {.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD, .size = (Uint32)dataSize};
        SDL_GPUTransferBuffer* tBuf = SDL_CreateGPUTransferBuffer(gpu->gpu, &tInfo);
        if (tBuf) {
            void* map = SDL_MapGPUTransferBuffer(gpu->gpu, tBuf, false);
            if (map) {
                memcpy(map, primitiveVertices.data(), dataSize);
                SDL_UnmapGPUTransferBuffer(gpu->gpu, tBuf);

                SDL_GPUCopyPass* cp = SDL_BeginGPUCopyPass(cmd);
                SDL_GPUTransferBufferLocation loc = {.transfer_buffer = tBuf, .offset = 0};
                SDL_GPUBufferRegion reg = {.buffer = primitiveVertexBuffer, .offset = 0, .size = (Uint32)dataSize};
                SDL_UploadToGPUBuffer(cp, &loc, &reg, false);
                SDL_EndGPUCopyPass(cp);
            }
            SDL_ReleaseGPUTransferBuffer(gpu->gpu, tBuf);
        }
    }
}

void GPUBatcher::endRenderPass(SDL_GPUCommandBuffer* cmd, SDL_GPURenderPass* render_pass)
{
    if (batches.empty()) return;

    // Static setup (doesn't change per batch type)
    if (!spriteInstances.empty()) {
        SDL_BindGPUVertexStorageBuffers(render_pass, 0, &storageBuffer, 1);
        SDL_BindGPUFragmentStorageBuffers(render_pass, 0, &storageBuffer, 1);
        SDL_GPUBufferBinding iBinding = {.buffer = indexBuffer, .offset = 0};
        SDL_BindGPUIndexBuffer(render_pass, &iBinding, SDL_GPU_INDEXELEMENTSIZE_16BIT);
    }

    SDL_GPUGraphicsPipeline* curPipe = nullptr;
    SDL_GPUTexture* curTex = nullptr;
    BatchType curType = BatchType::None;

    for (const auto& b : batches) {
        if (b.type == BatchType::Sprites) {
            // Re-bind vertex buffer if we switched from primitives
            if (curType != BatchType::Sprites) {
                SDL_GPUBufferBinding vBinding = {.buffer = vertexBuffer, .offset = 0};
                SDL_BindGPUVertexBuffers(render_pass, 0, &vBinding, 1);
                curType = BatchType::Sprites;
            }

            SDL_GPUGraphicsPipeline* nextPipe = b.outline ? spriteOutlinePipeline : spritePipeline;
            if (nextPipe != curPipe) {
                SDL_BindGPUGraphicsPipeline(render_pass, nextPipe);
                curPipe = nextPipe;
            }
            if (b.texture != curTex) {
                bindTexture(render_pass, b.texture);
                curTex = b.texture;
            }
            SDL_DrawGPUIndexedPrimitives(render_pass, 6, b.count, 0, 0, b.offset);
        } else {
            // Re-bind vertex buffer if we switched from sprites or different primitive buffer
            if (curType != b.type) {
                SDL_GPUBufferBinding vBinding = {.buffer = primitiveVertexBuffer, .offset = 0};
                SDL_BindGPUVertexBuffers(render_pass, 0, &vBinding, 1);
                curType = b.type;
            }

            SDL_GPUGraphicsPipeline* nextPipe = (b.type == BatchType::PrimitiveTriangles) ? primitiveFillPipeline : primitivePipeline;
            if (nextPipe != curPipe) {
                SDL_BindGPUGraphicsPipeline(render_pass, nextPipe);
                curPipe = nextPipe;
            }
            SDL_DrawGPUPrimitives(render_pass, b.count, 1, b.offset, 0);
        }
    }
    totalSpriteCount += spriteInstances.size();
    totalPrimitivesCount += primitiveVertices.size();
    contextSwitchCount += batches.size();
    batches.clear();
    spriteInstances.clear();
    primitiveVertices.clear();
}

void GPUBatcher::addBoundingBox(
    const SpriteTexture& sprite, int id, float x, float y, float scale_x, float scale_y, float angle, const ppl7::grafix::Color& color)
{
    ppl7::grafix::Rect box = sprite.spriteBoundary(id, scale_x, scale_y, angle, x, y);

    addRect(box.left(), box.top(), box.width(), box.height(), color, 4);
}

void GPUBatcher::addSprite(const SpriteTexture& sprite,
                           int id,
                           float x,
                           float y,
                           float scale_x,
                           float scale_y,
                           float angle,
                           const ppl7::grafix::Color& color_modulation)
{
    addSpriteInternal(sprite, id, x, y, scale_x, scale_y, angle, color_modulation, false);
}

void GPUBatcher::addSpriteOutline(const SpriteTexture& sprite,
                                  int id,
                                  float x,
                                  float y,
                                  float scale_x,
                                  float scale_y,
                                  float angle,
                                  const ppl7::grafix::Color& color_modulation)
{
    addSpriteInternal(sprite, id, x, y, scale_x, scale_y, angle, color_modulation, true);
}

void GPUBatcher::addSpriteInternal(const SpriteTexture& sprite,
                                   int id,
                                   float x,
                                   float y,
                                   float scale_x,
                                   float scale_y,
                                   float angle,
                                   const ppl7::grafix::Color& color,
                                   bool outline)
{
    const SpriteTexture::SpriteIndexItem* item = sprite.getSpriteIndex(id);
    if (!item) return;

    if (batches.empty() || batches.back().type != BatchType::Sprites || batches.back().texture != item->tex ||
        batches.back().outline != outline) {
        finishCurrentBatch();
        batches.push_back(
            {.type = BatchType::Sprites, .offset = (uint32_t)spriteInstances.size(), .count = 0, .texture = item->tex, .outline = outline});
    }

    float rad = angle * (M_PI / 180.0f);
    float c = cosf(rad);
    float s = sinf(rad);
    float sw = (float)item->r.w * scale_x;
    float sh = (float)item->r.h * scale_y;

    SpriteInstance inst;
    inst.pos_x = (x * 2.0f / (float)screenWidth) - 1.0f;
    inst.pos_y = 1.0f - (y * 2.0f / (float)screenHeight);
    inst.m00 = (2.0f / (float)screenWidth) * sw * c;
    inst.m01 = (2.0f / (float)screenWidth) * sh * (-s);
    inst.m10 = -(2.0f / (float)screenHeight) * sw * s;
    inst.m11 = -(2.0f / (float)screenHeight) * sh * c;
    inst.pos_z = 0.0f;
    inst.pad = 0.0f;
    inst.uv_x = item->uv.x;
    inst.uv_y = item->uv.y;
    inst.uv_w = item->uv.w;
    inst.uv_h = item->uv.h;
    inst.u_min = item->uv.x;
    inst.v_min = item->uv.y;
    inst.u_max = item->uv.x + item->uv.w;
    inst.v_max = item->uv.y + item->uv.h;
    inst.pivot_x = (item->r.w > 0) ? (float)(item->Pivot.x - item->Offset.x) / (float)item->r.w : 0.0f;
    inst.pivot_y = (item->r.h > 0) ? (float)(item->Pivot.y - item->Offset.y) / (float)item->r.h : 0.0f;

    SDL_FColor col = toSDLFPMAColor(color);
    inst.color_r = col.r;
    inst.color_g = col.g;
    inst.color_b = col.b;
    inst.color_a = col.a;

    spriteInstances.push_back(inst);
    batches.back().count++;
}

void GPUBatcher::addLine(float x1, float y1, float x2, float y2, const ppl7::grafix::Color& color, int thickness)
{
    if (thickness <= 1) {
        if (batches.empty() || batches.back().type != BatchType::PrimitiveLines) {
            finishCurrentBatch();
            batches.push_back({.type = BatchType::PrimitiveLines, .offset = (uint32_t)primitiveVertices.size(), .count = 0});
        }

        auto pushV = [&](float vx, float vy) {
            SDL_FColor c = toSDLFPMAColor(color);
            primitiveVertices.push_back(
                {(vx * 2.0f / (float)screenWidth) - 1.0f, 1.0f - (vy * 2.0f / (float)screenHeight), 0.0f, c.r, c.g, c.b, c.a});
        };

        pushV(x1, y1);
        pushV(x2, y2);
        batches.back().count += 2;
    } else {
        // Draw thicker line as filled rectangle
        float dx = x2 - x1;
        float dy = y2 - y1;
        float length = sqrtf(dx * dx + dy * dy);
        if (length > 0) {
            float nx = -dy / length * (float)thickness * 0.5f;
            float ny = dx / length * (float)thickness * 0.5f;

            if (batches.empty() || batches.back().type != BatchType::PrimitiveTriangles) {
                finishCurrentBatch();
                batches.push_back({.type = BatchType::PrimitiveTriangles, .offset = (uint32_t)primitiveVertices.size(), .count = 0});
            }

            auto pushV = [&](float vx, float vy) {
                SDL_FColor c = toSDLFPMAColor(color);
                primitiveVertices.push_back(
                    {(vx * 2.0f / (float)screenWidth) - 1.0f, 1.0f - (vy * 2.0f / (float)screenHeight), 0.0f, c.r, c.g, c.b, c.a});
            };

            // Two triangles for the thick line
            pushV(x1 + nx, y1 + ny);
            pushV(x2 + nx, y2 + ny);
            pushV(x2 - nx, y2 - ny);
            pushV(x1 + nx, y1 + ny);
            pushV(x2 - nx, y2 - ny);
            pushV(x1 - nx, y1 - ny);
            batches.back().count += 6;
        }
    }
}

void GPUBatcher::addRect(float x, float y, float w, float h, const ppl7::grafix::Color& color, int thickness)
{
    if (thickness <= 1) {
        if (batches.empty() || batches.back().type != BatchType::PrimitiveLines) {
            finishCurrentBatch();
            batches.push_back({.type = BatchType::PrimitiveLines, .offset = (uint32_t)primitiveVertices.size(), .count = 0});
        }

        auto pushV = [&](float vx, float vy) {
            SDL_FColor c = toSDLFPMAColor(color);
            primitiveVertices.push_back(
                {(vx * 2.0f / (float)screenWidth) - 1.0f, 1.0f - (vy * 2.0f / (float)screenHeight), 0.0f, c.r, c.g, c.b, c.a});
        };

        pushV(x, y);
        pushV(x + w, y);
        pushV(x + w, y);
        pushV(x + w, y + h);
        pushV(x + w, y + h);
        pushV(x, y + h);
        pushV(x, y + h);
        pushV(x, y);
        batches.back().count += 8;
    } else {
        // Draw thicker rect using 4 filled rects (lines)
        float t = (float)thickness;
        addFilledRect(x - t * 0.5f, y - t * 0.5f, w + t, t, color);     // Top
        addFilledRect(x - t * 0.5f, y + h - t * 0.5f, w + t, t, color); // Bottom
        addFilledRect(x - t * 0.5f, y + t * 0.5f, t, h - t, color);     // Left
        addFilledRect(x + w - t * 0.5f, y + t * 0.5f, t, h - t, color); // Right
    }
}

void GPUBatcher::addFilledRect(float x, float y, float w, float h, const ppl7::grafix::Color& color)
{
    if (batches.empty() || batches.back().type != BatchType::PrimitiveTriangles) {
        finishCurrentBatch();
        batches.push_back({.type = BatchType::PrimitiveTriangles, .offset = (uint32_t)primitiveVertices.size(), .count = 0});
    }

    auto pushV = [&](float vx, float vy) {
        SDL_FColor c = toSDLFPMAColor(color);
        primitiveVertices.push_back(
            {(vx * 2.0f / (float)screenWidth) - 1.0f, 1.0f - (vy * 2.0f / (float)screenHeight), 0.0f, c.r, c.g, c.b, c.a});
    };

    // Rect as 2 triangles with clockwise winding
    pushV(x, y);
    pushV(x + w, y);
    pushV(x + w, y + h);
    pushV(x, y);
    pushV(x + w, y + h);
    pushV(x, y + h);
    batches.back().count += 6;
}

void GPUBatcher::loadShaders()
{
    if (!gpu || !gpu->gpu) {
        throw GPUException("GPUContext not initialized");
    }

    // Load vertex shader (SPIR-V compiled from GLSL)
    // Using NDC version with storage buffer (CPU-side transformation like SimpleQuadTest)
    vertShader = gpu->loadShader("res/shaders/vulkan/sprite_ndc_storage.vert.spv", SDL_GPU_SHADERSTAGE_VERTEX,
                                 0,  // num_samplers
                                 0,  // num_storage_textures
                                 1,  // num_storage_buffers
                                 0); // num_uniform_buffers

    // Load fragment shader
    fragShader = gpu->loadShader("res/shaders/vulkan/sprite_ndc_storage.frag.spv", SDL_GPU_SHADERSTAGE_FRAGMENT,
                                 1,  // num_samplers
                                 0,  // num_storage_textures
                                 0,  // num_storage_buffers
                                 0); // num_uniform_buffers

    // Load outline fragment shader
    outlineFragShader = gpu->loadShader("res/shaders/vulkan/sprite_outline.frag.spv", SDL_GPU_SHADERSTAGE_FRAGMENT,
                                        1,  // num_samplers
                                        0,  // num_storage_textures
                                        0,  // num_storage_buffers
                                        0); // num_uniform_buffers

    // Load primitive shaders
    primitiveVertShader = gpu->loadShader("res/shaders/vulkan/primitive.vert.spv", SDL_GPU_SHADERSTAGE_VERTEX, 0, 0, 0, 0);
    primitiveFragShader = gpu->loadShader("res/shaders/vulkan/primitive.frag.spv", SDL_GPU_SHADERSTAGE_FRAGMENT, 0, 0, 0, 0);
}

void GPUBatcher::createPipeline()
{
    if (!gpu || !gpu->gpu || !vertShader || !fragShader) {
        throw GPUException("Shaders not loaded");
    }

    // Define vertex attributes (per-vertex data only)
    SDL_GPUVertexAttribute vertexAttributes[] = {
        // Position (location 0)
        {.location = 0, .buffer_slot = 0, .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2, .offset = 0},
        // Texcoord (location 1)
        {.location = 1, .buffer_slot = 0, .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2, .offset = sizeof(float) * 2},
        // Color (location 2)
        {.location = 2, .buffer_slot = 0, .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4, .offset = sizeof(float) * 4}};

    // Define vertex buffer layout (only per-vertex data)
    SDL_GPUVertexBufferDescription vertexBufferDesc[] = {
        // Slot 0: Per-vertex data
        {.slot = 0, .pitch = sizeof(Vertex), .input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX, .instance_step_rate = 0}};

    SDL_GPUVertexInputState vertexInputState = {.vertex_buffer_descriptions = vertexBufferDesc,
                                                .num_vertex_buffers = 1,
                                                .vertex_attributes = vertexAttributes,
                                                .num_vertex_attributes = 3};

    // Get swapchain texture format from GPUContext's window
    SDL_GPUTextureFormat swapchainFormat = SDL_GetGPUSwapchainTextureFormat(gpu->gpu, gpu->window);
    // ppl7::PrintDebugTime("GPUBatcher: Swapchain format = %d\n", (int)swapchainFormat);

    // Color target description
    SDL_GPUColorTargetDescription colorTarget = {.format = swapchainFormat,
                                                 .blend_state = {.src_color_blendfactor = SDL_GPU_BLENDFACTOR_ONE, // PMA output
                                                                 .dst_color_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
                                                                 .color_blend_op = SDL_GPU_BLENDOP_ADD,
                                                                 .src_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE,
                                                                 .dst_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
                                                                 .alpha_blend_op = SDL_GPU_BLENDOP_ADD,
                                                                 .color_write_mask = SDL_GPU_COLORCOMPONENT_R | SDL_GPU_COLORCOMPONENT_G |
                                                                                     SDL_GPU_COLORCOMPONENT_B | SDL_GPU_COLORCOMPONENT_A,
                                                                 .enable_blend = true}};

    // Create graphics pipeline
    SDL_GPUGraphicsPipelineCreateInfo pipelineInfo = {.vertex_shader = vertShader,
                                                      .fragment_shader = fragShader,
                                                      .vertex_input_state = vertexInputState,
                                                      .primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
                                                      .rasterizer_state =
                                                          {
                                                              .fill_mode = SDL_GPU_FILLMODE_FILL,
                                                              .cull_mode = SDL_GPU_CULLMODE_NONE,
                                                              .front_face = SDL_GPU_FRONTFACE_COUNTER_CLOCKWISE,
                                                          },
                                                      .multisample_state =
                                                          {
                                                              .sample_count = SDL_GPU_SAMPLECOUNT_1,
                                                          },
                                                      .depth_stencil_state =
                                                          {
                                                              .compare_op = SDL_GPU_COMPAREOP_ALWAYS,
                                                              .enable_depth_test = false,
                                                              .enable_depth_write = false,
                                                          },
                                                      .target_info = {.color_target_descriptions = &colorTarget,
                                                                      .num_color_targets = 1,
                                                                      .depth_stencil_format = (SDL_GPUTextureFormat)0,
                                                                      .has_depth_stencil_target = false}};

    spritePipeline = SDL_CreateGPUGraphicsPipeline(gpu->gpu, &pipelineInfo);
    if (!spritePipeline) {
        throw GPUException("Failed to create graphics pipeline: %s", SDL_GetError());
    }

    // Create outline graphics pipeline
    pipelineInfo.fragment_shader = outlineFragShader;
    spriteOutlinePipeline = SDL_CreateGPUGraphicsPipeline(gpu->gpu, &pipelineInfo);
    if (!spriteOutlinePipeline) {
        throw GPUException("Failed to create outline graphics pipeline: %s", SDL_GetError());
    }

    // --- Create Primitive Pipeline (LineList) ---
    SDL_GPUVertexAttribute primitiveAttributes[] = {
        // Position (location 0) - vec3
        {.location = 0, .buffer_slot = 0, .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3, .offset = 0},
        // Color (location 1) - vec4
        {.location = 1, .buffer_slot = 0, .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4, .offset = sizeof(float) * 3}};

    SDL_GPUVertexBufferDescription primitiveBufferDesc[] = {
        {.slot = 0, .pitch = sizeof(PrimitiveVertex), .input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX, .instance_step_rate = 0}};

    SDL_GPUVertexInputState primitiveVertexInput = {.vertex_buffer_descriptions = primitiveBufferDesc,
                                                    .num_vertex_buffers = 1,
                                                    .vertex_attributes = primitiveAttributes,
                                                    .num_vertex_attributes = 2};

    SDL_GPUColorTargetDescription primitiveColorTargets[] = {
        {.format = SDL_GetGPUSwapchainTextureFormat(gpu->gpu, gpu->window),
         .blend_state = {
             .src_color_blendfactor = SDL_GPU_BLENDFACTOR_ONE,                 // Korrektur für Pre-multiplied
             .dst_color_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA, // Bleibt gleich
             .color_blend_op = SDL_GPU_BLENDOP_ADD,
             .src_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE,                 // ONE für Coverage/Pre-multiplied
             .dst_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA, // Bleibt gleich
             .alpha_blend_op = SDL_GPU_BLENDOP_ADD,
             .enable_blend = true,
         }}};

    SDL_GPUGraphicsPipelineCreateInfo primitivePipelineInfo = {.vertex_shader = primitiveVertShader,
                                                               .fragment_shader = primitiveFragShader,
                                                               .vertex_input_state = primitiveVertexInput,
                                                               .primitive_type = SDL_GPU_PRIMITIVETYPE_LINELIST,
                                                               .rasterizer_state =
                                                                   {
                                                                       .fill_mode = SDL_GPU_FILLMODE_FILL,
                                                                       .cull_mode = SDL_GPU_CULLMODE_NONE,
                                                                       .front_face = SDL_GPU_FRONTFACE_CLOCKWISE,
                                                                   },
                                                               .multisample_state =
                                                                   {
                                                                       .sample_count = SDL_GPU_SAMPLECOUNT_1,
                                                                   },
                                                               .depth_stencil_state =
                                                                   {
                                                                       .compare_op = SDL_GPU_COMPAREOP_ALWAYS, // Overlay: Ignore depth
                                                                       .back_stencil_state = {},
                                                                       .front_stencil_state = {},
                                                                       .compare_mask = 0,
                                                                       .write_mask = 0,
                                                                       .enable_depth_test = false, // Disable depth testing
                                                                       .enable_depth_write = false,
                                                                       .enable_stencil_test = false,
                                                                   },
                                                               .target_info = {
                                                                   .color_target_descriptions = primitiveColorTargets,
                                                                   .num_color_targets = 1,
                                                                   .depth_stencil_format = (SDL_GPUTextureFormat)0,
                                                                   .has_depth_stencil_target = false,
                                                               }};

    primitivePipeline = SDL_CreateGPUGraphicsPipeline(gpu->gpu, &primitivePipelineInfo);
    if (!primitivePipeline) {
        throw GPUException("Failed to create primitive pipeline: %s", SDL_GetError());
    }

    // --- Create Primitive Fill Pipeline (TriangleList) ---
    primitivePipelineInfo.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;
    primitiveFillPipeline = SDL_CreateGPUGraphicsPipeline(gpu->gpu, &primitivePipelineInfo);
    if (!primitiveFillPipeline) {
        throw GPUException("Failed to create primitive fill pipeline: %s", SDL_GetError());
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
        .size = sizeof(Vertex) * 4, // Single quad only
    };
    vertexBuffer = SDL_CreateGPUBuffer(gpu->gpu, &vertexBufferInfo);
    if (!vertexBuffer) {
        throw GPUException("Failed to create vertex buffer: %s", SDL_GetError());
    }

    // Create index buffer (for single quad - reused for all sprites)
    SDL_GPUBufferCreateInfo indexBufferInfo = {
        .usage = SDL_GPU_BUFFERUSAGE_INDEX,
        .size = sizeof(Uint16) * 6, // Single quad only
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
        {0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f}, // Top-left
        {1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f}, // Top-right
        {0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f}, // Bottom-left
        {1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f}  // Bottom-right
    };

    // Quad indices (two triangles with counter-clockwise winding)
    // First triangle: 0→2→1 (top-left → bottom-left → top-right)
    // Second triangle: 1→2→3 (top-right → bottom-left → bottom-right)
    Uint16 quadIndices[6] = {0, 2, 1, 1, 2, 3};

    SDL_GPUCommandBuffer* cmd = SDL_AcquireGPUCommandBuffer(gpu->gpu);
    if (!cmd) return;

    // Upload vertices
    SDL_GPUTransferBufferCreateInfo transferInfo = {.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD, .size = sizeof(quadVertices)};
    SDL_GPUTransferBuffer* transferBuffer = SDL_CreateGPUTransferBuffer(gpu->gpu, &transferInfo);
    if (transferBuffer) {
        void* mapped = SDL_MapGPUTransferBuffer(gpu->gpu, transferBuffer, false);
        if (mapped) {
            memcpy(mapped, quadVertices, sizeof(quadVertices));
            SDL_UnmapGPUTransferBuffer(gpu->gpu, transferBuffer);

            SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(cmd);
            SDL_GPUTransferBufferLocation transferLocation = {.transfer_buffer = transferBuffer, .offset = 0};
            SDL_GPUBufferRegion bufferRegion = {.buffer = vertexBuffer, .offset = 0, .size = sizeof(quadVertices)};
            SDL_UploadToGPUBuffer(copyPass, &transferLocation, &bufferRegion, false);
            SDL_EndGPUCopyPass(copyPass);
        }
        SDL_ReleaseGPUTransferBuffer(gpu->gpu, transferBuffer);
    }

    // Upload indices
    SDL_GPUTransferBufferCreateInfo indexTransferInfo = {.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD, .size = sizeof(quadIndices)};
    SDL_GPUTransferBuffer* indexTransferBuffer = SDL_CreateGPUTransferBuffer(gpu->gpu, &indexTransferInfo);
    if (indexTransferBuffer) {
        void* mapped = SDL_MapGPUTransferBuffer(gpu->gpu, indexTransferBuffer, false);
        if (mapped) {
            memcpy(mapped, quadIndices, sizeof(quadIndices));
            SDL_UnmapGPUTransferBuffer(gpu->gpu, indexTransferBuffer);

            SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(cmd);
            SDL_GPUTransferBufferLocation transferLocation = {.transfer_buffer = indexTransferBuffer, .offset = 0};
            SDL_GPUBufferRegion bufferRegion = {.buffer = indexBuffer, .offset = 0, .size = sizeof(quadIndices)};
            SDL_UploadToGPUBuffer(copyPass, &transferLocation, &bufferRegion, false);
            SDL_EndGPUCopyPass(copyPass);
        }
        SDL_ReleaseGPUTransferBuffer(gpu->gpu, indexTransferBuffer);
    }

    SDL_SubmitGPUCommandBuffer(cmd);
    SDL_WaitForGPUIdle(gpu->gpu);
}

void GPUBatcher::setLogicalRenderSize(const ppl7::grafix::Size& size)
{
    setLogicalRenderSize(size.width, size.height);
}

void GPUBatcher::setLogicalRenderSize(int screenWidth, int screenHeight)
{
    this->screenWidth = screenWidth;
    this->screenHeight = screenHeight;

    // Create orthographic projection matrix for 2D rendering (Standard Y-UP NDC)
    // Maps screen coordinates (0,0) top-left to (screenWidth, screenHeight) bottom-right to NDC ((-1,1) to (1,-1))
    float left = 0.0f;
    float right = (float)screenWidth;
    float top = 0.0f;
    float bottom = (float)screenHeight;
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
    if (spriteOutlinePipeline) {
        SDL_ReleaseGPUGraphicsPipeline(gpu->gpu, spriteOutlinePipeline);
        spriteOutlinePipeline = nullptr;
    }
    if (primitivePipeline) {
        SDL_ReleaseGPUGraphicsPipeline(gpu->gpu, primitivePipeline);
        primitivePipeline = nullptr;
    }
    if (primitiveFillPipeline) {
        SDL_ReleaseGPUGraphicsPipeline(gpu->gpu, primitiveFillPipeline);
        primitiveFillPipeline = nullptr;
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
    if (primitiveVertexBuffer) {
        SDL_ReleaseGPUBuffer(gpu->gpu, primitiveVertexBuffer);
        primitiveVertexBuffer = nullptr;
    }
    if (uniformBuffer) {
        SDL_ReleaseGPUBuffer(gpu->gpu, uniformBuffer);
        uniformBuffer = nullptr;
    }

    if (fragShader) {
        SDL_ReleaseGPUShader(gpu->gpu, fragShader);
        fragShader = nullptr;
    }
    if (outlineFragShader) {
        SDL_ReleaseGPUShader(gpu->gpu, outlineFragShader);
        outlineFragShader = nullptr;
    }
    if (vertShader) {
        SDL_ReleaseGPUShader(gpu->gpu, vertShader);
        vertShader = nullptr;
    }
    if (primitiveFragShader) {
        SDL_ReleaseGPUShader(gpu->gpu, primitiveFragShader);
        primitiveFragShader = nullptr;
    }
    if (primitiveVertShader) {
        SDL_ReleaseGPUShader(gpu->gpu, primitiveVertShader);
        primitiveVertShader = nullptr;
    }
}

void GPUBatcher::bindTexture(SDL_GPURenderPass* render_pass, SDL_GPUTexture* texture)
{
    if (!texture || !sampler) {
        ppl7::PrintDebugTime("  ERROR: bindTexture failed - texture=%p, sampler=%p\n", texture, sampler);
        return;
    }

    // Bind texture and sampler for fragment shader
    SDL_GPUTextureSamplerBinding textureSamplerBinding = {.texture = texture, .sampler = sampler};

    SDL_BindGPUFragmentSamplers(render_pass, 0, &textureSamplerBinding, 1);
}
