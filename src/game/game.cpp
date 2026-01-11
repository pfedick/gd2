#include "game.h"


ppl7::grafix::Point GetViewPos()
{
    // TODO, wird von AudioInstance benutzt
    return ppl7::grafix::Point(0, 0);
}



Game::Game()
{
    wm = ppltk::GetWindowManager();
    Style.setStyle(ppltk::WidgetStyle::Dark);
    quitGame = false;

}
void Game::init()
{
    createWindow();
    gpu.init((SDL_Window*)getSDLWindow());
    sdl.setGPUDevice(gpu.gpu);
    gpu_batcher.init(&gpu);  // Now using Storage Buffers instead of vertex buffer instancing

    // Initialize projection/view matrices for rendering
    gpu_batcher.updateMatrices(1920, 1080);
}

void Game::createWindow()
{
    int flags = ppltk::Window::WaitVsync | ppltk::Window::NoSDLRenderer;
    config.windowMode = Config::WindowMode::Window;
    config.ScreenResolution = ppl7::grafix::Size(1920, 1080);
    if (config.windowMode == Config::WindowMode::Window) {
        flags |= ppltk::Window::Resizeable;
    }
    else if (config.windowMode == Config::WindowMode::Fullscreen) {
        flags |= ppltk::Window::Fullscreen | ppltk::Window::Resizeable;
    }
    else {
        flags |= ppltk::Window::FullscreenDesktop | ppltk::Window::Resizeable;
    }
    setFlags(flags);
    enableFixedUiSize(true, 1920, 1080);
    setWindowTitle("George Decker");
    ppl7::grafix::Image icon;
    icon.load("res/icon_128.png");
    setWindowIcon(icon);
    setRGBFormat(ppl7::grafix::RGBFormat::A8R8G8B8);
    setBackgroundColor(ppl7::grafix::Color(0, 0, 0, 0));
    setSize(config.ScreenResolution);
    wm->createWindow(*this);
    sdl_window = (SDL_Window*)getSDLWindow();
    //setPos(0,0);
    //SDL_RenderSetLogicalSize(renderer, 1920, 1080);
    wm->setGameControllerFocus(this);

    //SDL_HideCursor();
    SDL_HideCursor();

    WidgetDrawbuffer.create(1920, 1080, ppl7::grafix::RGBFormat::A8R8G8B8);
    this->setWidgetDrawbuffer(&WidgetDrawbuffer);

}


void Game::init_grafix()
{
    resources.load(gpu);

    ppl7::PrintDebug("Grafix initialized\n");
}


void Game::run()
{
    SDL_ShowCursor();
    sdl.setCursor(resources.Cursor.getDrawable(1), resources.Cursor.getPivot(1));
    ppl7::ppl_time_t last_second = ppl7::GetTime();
    quitGame = false;
    //fps.enableDebug(true);
    while (!quitGame) {
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
        if (cmdbuf == NULL)
        {
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
        fps.update();


        drawWorld(cmdbuf, swapchainTexture);
        // HUD
        drawHUD(cmdbuf, swapchainTexture);

        // Ui and Mouse if enabled
        drawUi(cmdbuf, swapchainTexture, mouse);

        // Frame done
        SDL_SubmitGPUCommandBuffer(cmdbuf);
        ppl7::PrintDebug("FPS: %d\n", fps.getFPS());

    }
}

void Game::updateUi(const ppltk::MouseState& mouse)
{

}

void Game::drawUi(SDL_GPUCommandBuffer* cmdbuf, SDL_GPUTexture* swapchainTexture, const ppltk::MouseState& mouse)
{
    if (!showui) return;
    gpu_batcher.startRenderPass();
    //gpu_batcher.addSprite(resources.Cursor, 1, mouse.p.x, mouse.p.y);

    gpu_batcher.prepareInstanceData(cmdbuf);
    SDL_GPUColorTargetInfo colorTargetInfo = { 0 };
    colorTargetInfo.texture = swapchainTexture;
    colorTargetInfo.clear_color = (SDL_FColor){ 0.0f, 0.0f, 0.0f, 1.0f };  // Black background
    colorTargetInfo.load_op = SDL_GPU_LOADOP_LOAD;
    colorTargetInfo.store_op = SDL_GPU_STOREOP_STORE;
    colorTargetInfo.cycle = false;  // CRITICAL: SDL examples use false!

    SDL_GPURenderPass* renderPass = SDL_BeginGPURenderPass(cmdbuf, &colorTargetInfo, 1, NULL);

    gpu_batcher.endRenderPass(cmdbuf, renderPass);

    SDL_EndGPURenderPass(renderPass);
    //drawWidgets();
}

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

    gpu_batcher.addSprite(resources.Player, 27, 1920 / 2, 1080 / 2, 2.0f, 2.0f);
    gpu_batcher.addSprite(resources.Player, 27, 1920 / 2, 1080 / 2, 0.5f, 0.5f);
    gpu_batcher.addSprite(resources.Player, 27, 1920 / 2, 1080 / 2, 1.0f, 1.0f, 90.0f);
    gpu_batcher.addSprite(resources.Player, 27, 1920 / 2, 1080 / 2, 1.0f, 1.0f, 180.0f);
    gpu_batcher.addSprite(resources.Player, 27, 1920 / 2, 1080 / 2, 1.0f, 1.0f, 270.0f);


    // Upload instance data to GPU (must be BEFORE BeginGPURenderPass)
    gpu_batcher.prepareInstanceData(cmdbuf);

    SDL_GPUColorTargetInfo colorTargetInfo = { 0 };
    colorTargetInfo.texture = swapchainTexture;
    colorTargetInfo.clear_color = (SDL_FColor){ 0.0f, 0.0f, 0.0f, 1.0f };  // Black background
    colorTargetInfo.load_op = SDL_GPU_LOADOP_CLEAR;
    colorTargetInfo.store_op = SDL_GPU_STOREOP_STORE;
    colorTargetInfo.cycle = false;  // CRITICAL: SDL examples use false!

    SDL_GPURenderPass* renderPass = SDL_BeginGPURenderPass(cmdbuf, &colorTargetInfo, 1, NULL);

    // Set viewport and scissor
    SDL_GPUViewport viewport = { 0.0f, 0.0f, 1920.0f, 1080.0f, 0.0f, 1.0f };
    SDL_SetGPUViewport(renderPass, &viewport);
    SDL_Rect scissor = { 0, 0, 1920, 1080 };
    SDL_SetGPUScissor(renderPass, &scissor);

    // Draw all batched sprites
    //gpu_batcher.prepareInstanceData(cmdbuf);
    gpu_batcher.endRenderPass(cmdbuf, renderPass);

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


