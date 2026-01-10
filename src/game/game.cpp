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
    gpu_batcher.init(&gpu);

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
    ppl7::ppl_time_t last_second = ppl7::GetTime();
    quitGame = false;
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
        drawWorld();
        // HUD
        //drawHUD();

        // UI
        //drawWidgets();

        // Mouse
        //drawCursor(mouse);

    }
}

void Game::updateUi(const ppltk::MouseState& mouse)
{

}

void Game::drawWorld()
{
    SDL_GPUCommandBuffer* cmdbuf = SDL_AcquireGPUCommandBuffer(gpu.gpu);
    if (cmdbuf == NULL)
    {
        SDL_Log("AcquireGPUCommandBuffer failed: %s", SDL_GetError());
        return;
    }
    SDL_GPUTexture* swapchainTexture;
    if (!SDL_WaitAndAcquireGPUSwapchainTexture(cmdbuf, sdl_window, &swapchainTexture, NULL, NULL)) {
        SDL_Log("WaitAndAcquireGPUSwapchainTexture failed: %s", SDL_GetError());
        return;
    }
    if (swapchainTexture == NULL) {
        // Das kann passieren, wenn das Fenster minimiert ist
        return;
    }
    SDL_GPUColorTargetInfo colorTargetInfo = { 0 };
    colorTargetInfo.texture = swapchainTexture;
    colorTargetInfo.clear_color = (SDL_FColor){ 0.3f, 0.6f, 0.5f, 1.0f };
    colorTargetInfo.load_op = SDL_GPU_LOADOP_CLEAR;
    colorTargetInfo.store_op = SDL_GPU_STOREOP_STORE;

    SDL_GPURenderPass* renderPass = SDL_BeginGPURenderPass(cmdbuf, &colorTargetInfo, 1, NULL);

    gpu_batcher.startRenderPass();
    int x = 0, y = 0;
    for (int i = 0;i < 100;i++) {
        gpu_batcher.addSprite(resources.Player, i, x, y, 1.0f, 1.0f, 0.0f);
        x += 100;
        if (x > 1800) {
            x = 0;
            y += 100;
        }
    }

    gpu_batcher.endRenderPass(cmdbuf, renderPass);
    SDL_EndGPURenderPass(renderPass);

    SDL_SubmitGPUCommandBuffer(cmdbuf);

}

void Game::drawHUD()
{
}

void Game::drawCursor(const ppltk::MouseState& mouse)
{
    if (showui) {
        gpu_batcher.addSprite(resources.Cursor, 1, mouse.p.x, mouse.p.y);
    }
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


