#include "game.h"
#include "ui/menue.h"
#include "ui/statusbar.h"

ppl7::grafix::Point GetViewPos()
{
    // TODO, wird von AudioInstance benutzt
    return ppl7::grafix::Point(0, 0);
}

static Game* game = NULL;

Game& GetGame()
{
    if (!game) throw ppl7::Exception("Game not initialized!");
    return *game;
}

Game::Game(GPUContext& gpu)
    : ppltk::Window(),
      gpu(gpu)
{
    wm = (ppltk::WindowManager_SDL3*)ppltk::GetWindowManager();
    // wm->enableGPURenderer(gpu.gpu);
    Style.setStyle(ppltk::WidgetStyle::Dark);
    wm->setWidgetStyle(Style);
    quitGame = false;
    render_target_layer = NULL;
    render_target_tmp1 = NULL;
    render_target_tmp2 = NULL;
    depthTexture = NULL;
    mainmenue = NULL;
    statusbar = NULL;
}

Game::~Game()
{
    if (render_target_layer) {
        gpu.destroyGPUTexture(render_target_layer);
        render_target_layer = NULL;
    }
    if (render_target_tmp1) {
        gpu.destroyGPUTexture(render_target_tmp1);
        render_target_tmp1 = NULL;
    }
    if (render_target_tmp2) {
        gpu.destroyGPUTexture(render_target_tmp2);
        render_target_tmp2 = NULL;
    }
    if (depthTexture) {
        gpu.destroyGPUTexture(depthTexture);
        depthTexture = NULL;
    }
}

void Game::init()
{
    wm->useGPUAPI(gpu.gpu);
    // wm->enableGPURenderer(gpu.gpu);
    createWindow();
    gpu.initializeWindow((SDL_Window*)getSDLWindow());
    sdl.setGPUDevice(gpu.gpu);
    gpu_batcher.init(&gpu); // Now using Storage Buffers instead of vertex buffer instancing

    renderPipelines.init(gpu.gpu, (SDL_Window*)getSDLWindow());
    // Initialize projection/view matrices for rendering
    gpu_batcher.updateMatrices(1920, 1080);
}

void Game::createWindow()
{
    int flags = ppltk::Window::WaitVsync;
    config.windowMode = Config::WindowMode::Window;
    config.ScreenResolution = ppl7::grafix::Size(1920, 1080);
    if (config.windowMode == Config::WindowMode::Window) {
        flags |= ppltk::Window::Resizeable;
    } else if (config.windowMode == Config::WindowMode::Fullscreen) {
        flags |= ppltk::Window::Fullscreen | ppltk::Window::Resizeable;
    } else {
        flags |= ppltk::Window::FullscreenDesktop | ppltk::Window::Resizeable;
    }
    setFlags(flags);
    enableFixedUiSize(true, 1920, 1080);
    setWindowTitle("GD2 Prototype");
    ppl7::grafix::Image icon;
    icon.load("res/icon_128.png");
    setWindowIcon(icon);
    setRGBFormat(ppl7::grafix::RGBFormat::A8R8G8B8);
    setBackgroundColor(ppl7::grafix::Color(0, 0, 0, 0));
    setSize(config.ScreenResolution);
    wm->createWindow(*this);
    sdl_window = (SDL_Window*)getSDLWindow();
    sdl_renderer = (SDL_Renderer*)getRenderer();
    sdl.setRenderer(sdl_renderer);
    // setPos(0,0);
    // SDL_RenderSetLogicalSize(renderer, 1920, 1080);
    wm->setGameControllerFocus(this);

    // SDL_HideCursor();
    SDL_HideCursor();

    // WidgetDrawbuffer.create(1920, 1080, ppl7::grafix::RGBFormat::A8R8G8B8);
    // this->setWidgetDrawbuffer(&WidgetDrawbuffer);

    mainmenue = new MainMenue(0, 0, 1920, 30, this);
    this->addChild(mainmenue);

    statusbar = new StatusBar(0, 1080 - 30, 1920, 30);
    this->addChild(statusbar);
}

void Game::init_grafix()
{
    resources.load(gpu);

    ppl7::PrintDebug("Grafix initialized\n");
}

void Game::createRenderTargetsIfRequired(const ppl7::grafix::Size& size)
{
    if (size == render_target_size) return;
    render_target_size = size;

    if (render_target_layer) {
        gpu.destroyGPUTexture(render_target_layer);
    }
    render_target_layer = gpu.createRenderTarget(size.width, size.height);

    if (render_target_tmp1) {
        gpu.destroyGPUTexture(render_target_tmp1);
    }
    render_target_tmp1 = gpu.createRenderTarget(size.width, size.height);

    if (render_target_tmp2) {
        gpu.destroyGPUTexture(render_target_tmp2);
    }
    render_target_tmp2 = gpu.createRenderTarget(size.width, size.height);

    if (depthTexture) {
        gpu.destroyGPUTexture(depthTexture);
    }
    depthTexture = gpu.createDepthBuffer(size.width, size.height);
}

void Game::run()
{
    SDL_ShowCursor();
    sdl.setCursor(resources.Cursor.getDrawable(1), resources.Cursor.getPivot(1));
    ppl7::ppl_time_t last_second = ppl7::GetTime();
    quitGame = false;
    while (!quitGame) {
        double start_time = ppl7::GetMicrotime();

        ppl7::ppl_time_t current_second = ppl7::GetTime();
        if (current_second > last_second) {
            last_second = current_second;
            // TODO: Update Metrics
        }
        wm->handleEvents();
        ppltk::MouseState mouse = wm->getMouseState();
        updateUi(mouse);
        gpu_batcher.clearQueues();

        SDL_GPUCommandBuffer* cmdbuf = SDL_AcquireGPUCommandBuffer(gpu.gpu);
        if (cmdbuf == NULL) {
            SDL_Log("AcquireGPUCommandBuffer failed: %s", SDL_GetError());
            continue;
        }
        SDL_GPUTexture* swapchainTexture;
        if (!SDL_WaitAndAcquireGPUSwapchainTexture(cmdbuf, sdl_window, &swapchainTexture, NULL, NULL)) {
            SDL_Log("WaitAndAcquireGPUSwapchainTexture failed: %s", SDL_GetError());
            continue;
        }
        if (swapchainTexture == NULL) {
            // Das kann passieren, wenn das Fenster minimiert ist
            SDL_SubmitGPUCommandBuffer(cmdbuf);
            continue;
        }

        // Ensure Depth Buffer matches window size
        int w, h;
        SDL_GetWindowSizeInPixels(sdl_window, &w, &h);
        createRenderTargetsIfRequired(ppl7::grafix::Size(w, h));

        fps.update();

        drawWorld(cmdbuf, swapchainTexture);
        // HUD
        drawHUD(cmdbuf, swapchainTexture);

        // Ui and Mouse if enabled
        drawUi(cmdbuf, swapchainTexture, mouse);

        // Frame done
        SDL_SubmitGPUCommandBuffer(cmdbuf);
        double frame_time = ppl7::GetMicrotime() - start_time;
        frame_count++;
        time_accumulator += frame_time;
        if ((frame_count % 60) == 0) {
            ppl7::PrintDebug("Frametime: %0.3f ms\n", 1000.0 * (frame_time / frame_count));
            frame_count = 0;
            time_accumulator = 0.0f;
        }
    }
}

void Game::updateUi(const ppltk::MouseState& mouse)
{
}

void Game::drawUi(SDL_GPUCommandBuffer* cmdbuf, SDL_GPUTexture* swapchainTexture, const ppltk::MouseState& mouse)
{
    if (!showui) return;
    statusbar->setFps(fps.getFPS());

    // 1. Draw widgets into PPLTK internal texture
    this->drawWidgets();
    wm->updateGPUTexture(*this, cmdbuf);

    SDL_GPUTexture* gpuTex = (SDL_GPUTexture*)wm->getGPUTexture(*this);

    if (gpuTex) {
        // 2. Draw PPLTK texture as overlay in final swapchain texture
        // ppl7::PrintDebug("Drawing UI overlay\n");
        SDL_GPUColorTargetInfo targetInfo = {0};
        targetInfo.texture = swapchainTexture;
        // Load existing content (game world), don't clear
        targetInfo.load_op = SDL_GPU_LOADOP_LOAD;
        targetInfo.store_op = SDL_GPU_STOREOP_STORE;

        SDL_GPURenderPass* renderPass = SDL_BeginGPURenderPass(cmdbuf, &targetInfo, 1, NULL);
        // Viewport/Scissor to match window
        SDL_SetGPUViewport(renderPass, NULL);
        SDL_SetGPUScissor(renderPass, NULL);

        SDL_BindGPUGraphicsPipeline(renderPass, renderPipelines.uiPipeline);

        SDL_GPUTextureSamplerBinding binding;
        binding.texture = gpuTex;
        binding.sampler = renderPipelines.samplerClamp;
        SDL_BindGPUFragmentSamplers(renderPass, 0, &binding, 1);

        // Draw Fullscreen Quad
        SDL_DrawGPUPrimitives(renderPass, 3, 1, 0, 0);
        SDL_EndGPURenderPass(renderPass);
    } else {
        ppl7::PrintDebug("ERROR: Could not get GPU Texture from PPLTK UI Surface!\n");
    }
}

struct BlurParams
{
    float blurStrength;
    float padding; // WICHTIG: 4 Bytes Füllmaterial für std140 Alignment
    float texelSizeX;
    float texelSizeY;
};

void Game::drawWorld(SDL_GPUCommandBuffer* cmdbuf, SDL_GPUTexture* swapchainTexture)
{
    // Start render pass (resets z-order counter)
    gpu_batcher.startRenderPass();

    // Collect all sprites to batch

    int x = 100, y = 200;
    for (int i = 0; i < 100; i++) {
        gpu_batcher.addSprite(resources.Player, i, x, y);
        x += 100;
        if (x >= 1800) {
            x = 100;
            y += 200;
        }
    }

    /*
    for (int i = 0; i < 100; i++) {
        gpu_batcher.addSprite(resources.Player, ppl7::rand(0, 200), ppl7::rand(0, 1920), ppl7::rand(0,
    1080), 1.0f, 1.0f, 0.0f);
    }
    */

    gpu_batcher.addSprite(resources.Player, 27, 100, 200, 1.0f, 1.0f, 0.0f);

    gpu_batcher.addSprite(resources.Player, 27, 1920 / 2, 1080 / 2, 1.0f, 1.0f, 0.0f, ppl7::grafix::Color(255, 0, 0, 255));
    gpu_batcher.addSprite(resources.Player, 27, 1920 / 2, 1080 / 2, 1.0f, 1.0f, 90.0f, ppl7::grafix::Color(0, 255, 0, 255));
    gpu_batcher.addSprite(resources.Player, 27, 1920 / 2, 1080 / 2, 1.0f, 1.0f, 180.0f, ppl7::grafix::Color(0, 0, 255, 255));
    gpu_batcher.addSprite(resources.Player, 27, 1920 / 2, 1080 / 2, 1.0f, 1.0f, 270.0f);

    // Example Usage
    gpu_batcher.addLine(100, 100, 200, 200, ppl7::grafix::Color(255, 0, 0, 255), 10.0f);
    gpu_batcher.addRect(300, 100, 50, 50, ppl7::grafix::Color(0, 255, 0, 255), 10.0f);
    gpu_batcher.addFilledRect(400, 100, 50, 50, ppl7::grafix::Color(0, 0, 255, 255));

    // Upload instance data to GPU (must be BEFORE BeginGPURenderPass)
    gpu_batcher.prepareInstanceData(cmdbuf);

    SDL_GPUColorTargetInfo colorTargetInfo = {0};
    colorTargetInfo.texture = render_target_layer;
    colorTargetInfo.clear_color = (SDL_FColor){0.3f, 0.0f, 0.0f, 1.0f}; // Black background
    colorTargetInfo.load_op = SDL_GPU_LOADOP_CLEAR;
    colorTargetInfo.store_op = SDL_GPU_STOREOP_STORE;
    colorTargetInfo.cycle = false; // CRITICAL: SDL examples use false!

    SDL_GPUDepthStencilTargetInfo depthTargetInfo = {0};
    depthTargetInfo.texture = depthTexture;
    depthTargetInfo.clear_depth = 1.0f;
    depthTargetInfo.load_op = SDL_GPU_LOADOP_CLEAR;
    depthTargetInfo.store_op = SDL_GPU_STOREOP_DONT_CARE;
    depthTargetInfo.stencil_load_op = SDL_GPU_LOADOP_DONT_CARE;
    depthTargetInfo.stencil_store_op = SDL_GPU_STOREOP_DONT_CARE;
    depthTargetInfo.cycle = false;

    SDL_GPURenderPass* renderPass = SDL_BeginGPURenderPass(cmdbuf, &colorTargetInfo, 1, &depthTargetInfo);

    // Set viewport and scissor
    SDL_GPUViewport viewport = {0.0f, 0.0f, 1920.0f, 1080.0f, 0.0f, 1.0f};
    SDL_Rect scissor = {0, 0, 1920, 1080};
    SDL_SetGPUViewport(renderPass, NULL);
    SDL_SetGPUScissor(renderPass, NULL);

    // Draw all batched sprites
    // gpu_batcher.prepareInstanceData(cmdbuf);
    gpu_batcher.endRenderPass(cmdbuf, renderPass);
    SDL_EndGPURenderPass(renderPass);

    SDL_GPUColorTargetInfo targetInfo = {};
    SDL_GPUTextureSamplerBinding binding = {};

    // *******************************************************
    // Postprocessing passes would go here...

    BlurParams params;
    params.blurStrength = 0.5f;
    params.padding = 0.0f;                                       // Egal was hier steht
    params.texelSizeX = 1.0f / (float)render_target_size.width;  // Breite der Textur
    params.texelSizeY = 1.0f / (float)render_target_size.height; // Höhe der Textur

    // Slot Index 0 (passend zu binding = 0 im Shader, Set 3 ist implizit für Fragment Uniforms)
    SDL_PushGPUFragmentUniformData(cmdbuf, 0, &params, sizeof(BlurParams));

    targetInfo.texture = render_target_tmp1; // Ziel: Temp Textur
    targetInfo.load_op = SDL_GPU_LOADOP_DONT_CARE;
    targetInfo.store_op = SDL_GPU_STOREOP_STORE;

    renderPass = SDL_BeginGPURenderPass(cmdbuf, &targetInfo, 1, NULL);
    SDL_SetGPUViewport(renderPass, &viewport);
    SDL_SetGPUScissor(renderPass, &scissor);

    SDL_BindGPUGraphicsPipeline(renderPass, renderPipelines.blurHorizontalPipeline);
    binding.texture = render_target_layer;
    binding.sampler = renderPipelines.samplerClamp; // Einen Clamp-Sampler benutzen!
    SDL_BindGPUFragmentSamplers(renderPass, 0, &binding, 1);

    // Fullscreen Triangle zeichnen (3 Vertices, Shader generiert Coords)
    SDL_DrawGPUPrimitives(renderPass, 3, 1, 0, 0);

    SDL_EndGPURenderPass(renderPass);

    targetInfo.texture = render_target_tmp2;
    targetInfo.load_op = SDL_GPU_LOADOP_DONT_CARE; // Wir überschreiben alles
    targetInfo.store_op = SDL_GPU_STOREOP_STORE;

    renderPass = SDL_BeginGPURenderPass(cmdbuf, &targetInfo, 1, NULL);
    SDL_SetGPUViewport(renderPass, &viewport);
    SDL_SetGPUScissor(renderPass, &scissor);

    SDL_BindGPUGraphicsPipeline(renderPass, renderPipelines.blurVerticalPipeline);

    // Eingabe-Textur binden (das Bild aus Pass 2)
    binding.texture = render_target_tmp1;
    binding.sampler = renderPipelines.samplerClamp;
    SDL_BindGPUFragmentSamplers(renderPass, 0, &binding, 1);

    SDL_DrawGPUPrimitives(renderPass, 3, 1, 0, 0);
    SDL_EndGPURenderPass(renderPass);

    // Copy to swapchain
    targetInfo.texture = swapchainTexture;
    targetInfo.load_op = SDL_GPU_LOADOP_DONT_CARE;
    targetInfo.store_op = SDL_GPU_STOREOP_STORE;

    renderPass = SDL_BeginGPURenderPass(cmdbuf, &targetInfo, 1, NULL);
    SDL_SetGPUViewport(renderPass, NULL);
    SDL_SetGPUScissor(renderPass, NULL);

    SDL_BindGPUGraphicsPipeline(renderPass, renderPipelines.copyPipeline);
    binding.texture = render_target_tmp2;
    binding.sampler = renderPipelines.samplerClamp; // Einen Clamp-Sampler benutzen!
    SDL_BindGPUFragmentSamplers(renderPass, 0, &binding, 1);

    // Fullscreen Triangle zeichnen (3 Vertices, Shader generiert Coords)
    SDL_DrawGPUPrimitives(renderPass, 3, 1, 0, 0);

    SDL_EndGPURenderPass(renderPass);
}

void Game::drawHUD(SDL_GPUCommandBuffer* cmdbuf, SDL_GPUTexture* swapchainTexture)
{
}

void Game::quitEvent(ppltk::Event* event)
{
    ppl7::PrintDebug("Quit event received\n");
    quitGame = true;
}

void Game::closeEvent(ppltk::Event* event)
{
    ppl7::PrintDebug("Close event received\n");
    quitGame = true;
}

void Game::updateSpriteFromUi()
{
    // TODO
}