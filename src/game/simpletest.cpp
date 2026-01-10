#include "game.h"
#include "simpletest.h"
#include <ppl7.h>

// Textured quad - simple NDC version (WORKING)
struct SimpleVertex {
    float x, y;
    float u, v;
};

SimpleQuadTest::SimpleQuadTest() : gpu(nullptr), pipeline(nullptr), vertShader(nullptr),
fragShader(nullptr), vertexBuffer(nullptr), indexBuffer(nullptr),
sampler(nullptr), testTexture(nullptr)
{
    memset(projection, 0, sizeof(projection));
}

SimpleQuadTest::~SimpleQuadTest()
{
    cleanup();
}

void SimpleQuadTest::init(GPUContext* gpuCtx, int screenWidth, int screenHeight)
{
    gpu = gpuCtx;

    // TEST: Use identity matrix first to see if shader receives data at all
    memset(projection, 0, sizeof(projection));
    projection[0] = 1.0f;   // Identity
    projection[5] = 1.0f;
    projection[10] = 1.0f;
    projection[15] = 1.0f;

    ppl7::PrintDebugTime("TEST: Using identity matrix (should pass through NDC coords unchanged)\n");

    ppl7::PrintDebugTime("SimpleQuadTest: Loading shaders...\n");
    loadShaders();
    ppl7::PrintDebugTime("SimpleQuadTest: Creating pipeline...\n");
    createPipeline();
    ppl7::PrintDebugTime("SimpleQuadTest: Creating buffers...\n");
    createBuffers();
    ppl7::PrintDebugTime("SimpleQuadTest: Loading test texture...\n");
    loadTestTexture();
    ppl7::PrintDebugTime("SimpleQuadTest: Initialization complete\n");
}

void SimpleQuadTest::loadShaders()
{
    // WORKING: NDC textured quad (no matrix)
    ppl7::ByteArray vertCode = ppl7::File::load("res/shaders/vulkan/ndc_textured.vert.spv");

    SDL_GPUShaderCreateInfo vertInfo = {
        .code_size = (size_t)vertCode.size(),
        .code = (const Uint8*)vertCode.ptr(),
        .entrypoint = "main",
        .format = SDL_GPU_SHADERFORMAT_SPIRV,
        .stage = SDL_GPU_SHADERSTAGE_VERTEX,
        .num_samplers = 0,
        .num_storage_textures = 0,
        .num_storage_buffers = 0,
        .num_uniform_buffers = 0
    };
    vertShader = SDL_CreateGPUShader(gpu->gpu, &vertInfo);
    if (!vertShader) {
        throw GPUException("Failed to load color vertex shader: %s", SDL_GetError());
    }

    // Load fragment shader
    ppl7::ByteArray fragCode = ppl7::File::load("res/shaders/vulkan/ndc_textured.frag.spv");

    SDL_GPUShaderCreateInfo fragInfo = {
        .code_size = (size_t)fragCode.size(),
        .code = (const Uint8*)fragCode.ptr(),
        .entrypoint = "main",
        .format = SDL_GPU_SHADERFORMAT_SPIRV,
        .stage = SDL_GPU_SHADERSTAGE_FRAGMENT,
        .num_samplers = 1,  // One texture sampler
        .num_storage_textures = 0,
        .num_storage_buffers = 0,
        .num_uniform_buffers = 0
    };
    fragShader = SDL_CreateGPUShader(gpu->gpu, &fragInfo);
    if (!fragShader) {
        throw GPUException("Failed to load color fragment shader: %s", SDL_GetError());
    }

    ppl7::PrintDebugTime("  Textured shaders loaded\n");
}

void SimpleQuadTest::createPipeline()
{
    // Create sampler
    SDL_GPUSamplerCreateInfo samplerInfo = {
        .min_filter = SDL_GPU_FILTER_LINEAR,
        .mag_filter = SDL_GPU_FILTER_LINEAR,
        .mipmap_mode = SDL_GPU_SAMPLERMIPMAPMODE_NEAREST,
        .address_mode_u = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
        .address_mode_v = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
    };
    sampler = SDL_CreateGPUSampler(gpu->gpu, &samplerInfo);

    // Vertex attributes: position (vec2) and UV (vec2)
    SDL_GPUVertexAttribute attributes[] = {
        {.location = 0, .buffer_slot = 0, .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2, .offset = 0 },
        {.location = 1, .buffer_slot = 0, .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2, .offset = sizeof(float) * 2 }
    };

    SDL_GPUVertexBufferDescription bufferDesc = {
        .slot = 0,
        .pitch = sizeof(SimpleVertex),
        .input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX,
        .instance_step_rate = 0
    };

    SDL_GPUVertexInputState vertexInputState = {
        .vertex_buffer_descriptions = &bufferDesc,
        .num_vertex_buffers = 1,
        .vertex_attributes = attributes,
        .num_vertex_attributes = 2
    };

    // Get swapchain format from GPUContext's window
    SDL_GPUTextureFormat swapchainFormat = SDL_GetGPUSwapchainTextureFormat(gpu->gpu, gpu->window);
    ppl7::PrintDebugTime("  Swapchain format: %d\n", (int)swapchainFormat);

    SDL_GPUColorTargetDescription colorTarget = {
        .format = swapchainFormat,
        .blend_state = {
            .src_color_blendfactor = SDL_GPU_BLENDFACTOR_SRC_ALPHA,
            .dst_color_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
            .color_blend_op = SDL_GPU_BLENDOP_ADD,
            .src_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE,
            .dst_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
            .alpha_blend_op = SDL_GPU_BLENDOP_ADD,
            .color_write_mask = 0xF,
            .enable_blend = true
        }
    };

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

    pipeline = SDL_CreateGPUGraphicsPipeline(gpu->gpu, &pipelineInfo);
    if (!pipeline) {
        throw GPUException("Failed to create simple pipeline: %s", SDL_GetError());
    }
}

void SimpleQuadTest::createBuffers()
{
    // WORKING: Simple centered NDC quad (fullscreen at -1 to 1)
    SimpleVertex vertices[4] = {
        { -1.0f,  1.0f, 0.0f, 0.0f },  // Top-left
        {  1.0f,  1.0f, 1.0f, 0.0f },  // Top-right
        { -1.0f, -1.0f, 0.0f, 1.0f },  // Bottom-left
        {  1.0f, -1.0f, 1.0f, 1.0f }   // Bottom-right
    };

    ppl7::PrintDebugTime("  Fullscreen NDC quad (WORKING VERSION)\n");

    Uint16 indices[6] = { 0, 1, 2, 1, 3, 2 };  // Standard winding

    // Create vertex buffer
    SDL_GPUBufferCreateInfo vertexBufferInfo = {
        .usage = SDL_GPU_BUFFERUSAGE_VERTEX,
        .size = sizeof(vertices),
    };
    vertexBuffer = SDL_CreateGPUBuffer(gpu->gpu, &vertexBufferInfo);

    // Create index buffer
    SDL_GPUBufferCreateInfo indexBufferInfo = {
        .usage = SDL_GPU_BUFFERUSAGE_INDEX,
        .size = sizeof(indices),
    };
    indexBuffer = SDL_CreateGPUBuffer(gpu->gpu, &indexBufferInfo);

    // Upload data
    SDL_GPUCommandBuffer* cmd = SDL_AcquireGPUCommandBuffer(gpu->gpu);
    SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(cmd);

    // Upload vertices
    SDL_GPUTransferBufferCreateInfo transferInfo = {
        .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
        .size = sizeof(vertices)
    };
    SDL_GPUTransferBuffer* transferBuffer = SDL_CreateGPUTransferBuffer(gpu->gpu, &transferInfo);
    void* mapped = SDL_MapGPUTransferBuffer(gpu->gpu, transferBuffer, false);
    memcpy(mapped, vertices, sizeof(vertices));
    SDL_UnmapGPUTransferBuffer(gpu->gpu, transferBuffer);

    SDL_GPUTransferBufferLocation transferLoc = { .transfer_buffer = transferBuffer, .offset = 0 };
    SDL_GPUBufferRegion bufferRegion = { .buffer = vertexBuffer, .offset = 0, .size = sizeof(vertices) };
    SDL_UploadToGPUBuffer(copyPass, &transferLoc, &bufferRegion, false);
    SDL_ReleaseGPUTransferBuffer(gpu->gpu, transferBuffer);

    // Upload indices
    transferInfo.size = sizeof(indices);
    transferBuffer = SDL_CreateGPUTransferBuffer(gpu->gpu, &transferInfo);
    mapped = SDL_MapGPUTransferBuffer(gpu->gpu, transferBuffer, false);
    memcpy(mapped, indices, sizeof(indices));
    SDL_UnmapGPUTransferBuffer(gpu->gpu, transferBuffer);

    transferLoc.transfer_buffer = transferBuffer;
    bufferRegion.buffer = indexBuffer;
    bufferRegion.size = sizeof(indices);
    SDL_UploadToGPUBuffer(copyPass, &transferLoc, &bufferRegion, false);
    SDL_ReleaseGPUTransferBuffer(gpu->gpu, transferBuffer);

    SDL_EndGPUCopyPass(copyPass);
    SDL_SubmitGPUCommandBuffer(cmd);

    ppl7::PrintDebugTime("  Quad buffers created: 400x400 at (100,100)\n");
}

void SimpleQuadTest::loadTestTexture()
{
    ppl7::PrintDebugTime("  Creating test texture...\n");
    // Create a test texture with a pattern (A8R8G8B8 for GPU compatibility)
    ppl7::grafix::Image img;
    img.create(256, 256, ppl7::grafix::RGBFormat::A8R8G8B8);
    ppl7::PrintDebugTime("  Image created: %dx%d\n", img.width(), img.height());

    // Draw a simple pattern: red/white checkerboard
    for (int y = 0; y < 256; y++) {
        for (int x = 0; x < 256; x++) {
            bool checkX = (x / 32) % 2 == 0;
            bool checkY = (y / 32) % 2 == 0;
            ppl7::grafix::Color color = (checkX ^ checkY) ?
                ppl7::grafix::Color(255, 0, 0, 255) : // Red
                ppl7::grafix::Color(255, 255, 255, 255); // White
            img.putPixel(x, y, color);
        }
    }
    ppl7::PrintDebugTime("  Pattern drawn\n");

    testTexture = gpu->createGPUTexture(img);
    ppl7::PrintDebugTime("  Test texture created: %p\n", testTexture);
    if (!testTexture) {
        throw GPUException("Failed to create test texture");
    }
}

void SimpleQuadTest::draw(SDL_GPUCommandBuffer* cmd, SDL_GPURenderPass* renderPass)
{
    if (!pipeline) {
        ppl7::PrintDebugTime("SimpleQuadTest::draw - Pipeline is NULL!\n");
        return;
    }
    if (!testTexture) {
        ppl7::PrintDebugTime("SimpleQuadTest::draw - Texture is NULL!\n");
        return;
    }

    ppl7::PrintDebugTime("SimpleQuadTest::draw - Drawing textured quad...\n");

    // Bind pipeline
    SDL_BindGPUGraphicsPipeline(renderPass, pipeline);

    // NO MATRIX - using NDC coordinates directly (WORKS)

    // Bind vertex buffer
    SDL_GPUBufferBinding vertexBinding = { .buffer = vertexBuffer, .offset = 0 };
    SDL_BindGPUVertexBuffers(renderPass, 0, &vertexBinding, 1);

    // Bind index buffer
    SDL_GPUBufferBinding indexBinding = { .buffer = indexBuffer, .offset = 0 };
    SDL_BindGPUIndexBuffer(renderPass, &indexBinding, SDL_GPU_INDEXELEMENTSIZE_16BIT);

    // Bind texture
    SDL_GPUTextureSamplerBinding textureSampler = {
        .texture = testTexture,
        .sampler = sampler
    };
    SDL_BindGPUFragmentSamplers(renderPass, 0, &textureSampler, 1);

    ppl7::PrintDebugTime("  Drawing 6 indices...\n");
    SDL_DrawGPUIndexedPrimitives(renderPass, 6, 1, 0, 0, 0);
    ppl7::PrintDebugTime("  Draw complete\n");
}

void SimpleQuadTest::cleanup()
{
    if (!gpu || !gpu->gpu) return;

    if (pipeline) SDL_ReleaseGPUGraphicsPipeline(gpu->gpu, pipeline);
    if (vertShader) SDL_ReleaseGPUShader(gpu->gpu, vertShader);
    if (fragShader) SDL_ReleaseGPUShader(gpu->gpu, fragShader);
    if (vertexBuffer) SDL_ReleaseGPUBuffer(gpu->gpu, vertexBuffer);
    if (indexBuffer) SDL_ReleaseGPUBuffer(gpu->gpu, indexBuffer);
    if (sampler) SDL_ReleaseGPUSampler(gpu->gpu, sampler);
    if (testTexture) SDL_ReleaseGPUTexture(gpu->gpu, testTexture);

    pipeline = nullptr;
    vertShader = nullptr;
    fragShader = nullptr;
    vertexBuffer = nullptr;
    indexBuffer = nullptr;
    sampler = nullptr;
    testTexture = nullptr;
}

// Global instance
SimpleQuadTest* g_simpleTest = nullptr;
