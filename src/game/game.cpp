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

}
void Game::init()
{
    createWindow();
    gpu.init((SDL_Window*)getSDLWindow());
    sdl.setRenderer((SDL_Renderer*)getRenderer());
    sdl.setGPUDevice(gpu.gpu);

}

void Game::createWindow()
{
    if (config.windowMode == Config::WindowMode::Window) {
        setFlags(ppltk::Window::WaitVsync | ppltk::Window::Resizeable);
    }
    else if (config.windowMode == Config::WindowMode::Fullscreen) {
        setFlags(ppltk::Window::WaitVsync | ppltk::Window::Fullscreen | ppltk::Window::Resizeable);
    }
    else {
        setFlags(ppltk::Window::WaitVsync | ppltk::Window::FullscreenDesktop | ppltk::Window::Resizeable);
    }
    enableFixedUiSize(true, 1920, 1080);
    setWindowTitle("George Decker");
    ppl7::grafix::Image icon;
    icon.load("res/icon_128.png");
    setWindowIcon(icon);
    setRGBFormat(ppl7::grafix::RGBFormat::A8R8G8B8);
    setBackgroundColor(ppl7::grafix::Color(0, 0, 0, 0));
    setSize(config.ScreenResolution);
    wm->createWindow(*this);
    SDL_Renderer* renderer = (SDL_Renderer*)getRenderer();
    sdl.setRenderer(renderer);

    gpu.init((SDL_Window*)getSDLWindow());

    //setPos(0,0);
    //SDL_RenderSetLogicalSize(renderer, 1920, 1080);
    wm->setGameControllerFocus(this);

    SDL_HideCursor();


}


void Game::init_grafix()
{
    resources.load(gpu);
}


void Game::run()
{
    ppl7::ppl_time_t last_second = ppl7::GetTime();
    while (!quitGame) {
        ppl7::ppl_time_t current_second = ppl7::GetTime();
        if (current_second > last_second) {
            last_second = current_second;
            // TODO: Update Metrics
        }
        wm->handleEvents();
        ppltk::MouseState mouse = wm->getMouseState();
        updateUi(mouse);

        gpu.clearQueues();
        drawWorld();
        // HUD
        drawHUD();

        // UI
        drawWidgets();

        // Mouse
        drawCursor(mouse);

    }
}

void Game::updateUi(const ppltk::MouseState& mouse)
{

}

void Game::drawWorld()
{

}

void Game::drawHUD()
{
}

void Game::drawCursor(const ppltk::MouseState& mouse)
{
    if (showui) {
        gpu.drawSprite(resources.Cursor, 1, mouse.p.x, mouse.p.y);
    }
}


void Game::quitEvent(ppltk::Event* event)
{
    quitGame = true;
}

void Game::closeEvent(ppltk::Event* event)
{
    quitGame = true;
}


