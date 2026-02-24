#include "game.h"
#include <ppltk.h>
#include "player.h"
#include "ui/menue.h"
#include "ui/statusbar.h"
#include "ui/worldwidget.h"
#include "constants.h"
#include "translate.h"

static Game* GameInstance = NULL;

Game& GetGame()
{
    if (!GameInstance) throw ppl7::Exception("Game not initialized!");
    return *GameInstance;
}

ppltk::Window* GetGameWindow()
{
    if (!GameInstance) throw ppl7::Exception("Game not initialized!");
    return &GameInstance->window();
}

ppl7::grafix::Point GetViewPos()
{
    // TODO, wird von AudioInstance benutzt, um die Position von Audioobjekten zu berechnen.
    const ppl7::grafix::PointF& worldcoords = GameInstance->getWorldCoords();
    const GameViewport& viewport = GameInstance->getGameViewport();
    ppl7::grafix::Point p = worldcoords;
    ppl7::grafix::Size render_size = viewport.getRenderSize();
    p.x += render_size.width / 2;
    p.y += render_size.height / 2;
    return p;
}

Game::Game(GPUContext& gpu)
    : ppltk::Window(),
      gpu(gpu)
{
    SDL_SetHint(SDL_HINT_TIMER_RESOLUTION, "1");
    GameInstance = this;
    wm = (ppltk::WindowManager_SDL3*)ppltk::GetWindowManager();
    // wm->enableGPURenderer(gpu.gpu);
    Style.setStyle(ppltk::WidgetStyle::Dark);
    wm->setWidgetStyle(Style);
    quitGame = false;
    showui = false;
    worldIsMoving = false;
    controlsEnabled = true;
    world_widget = NULL;
    filedialog = NULL;
    // game_viewport.setRenderSize(ppl7::grafix::Size(640, 360));
    game_viewport.setRenderSize(ppl7::grafix::Size(1920, 1080)); // Rendering ist 1080p
    // game_viewport.setRenderSize(ppl7::grafix::Size(3840, 2160)); // Rendering ist 1080p
    game_viewport.setLogicalSize(ppl7::grafix::Size(3840, 2160));
    game_viewport.setAspectRatio(16.0f / 9.0f);
    player = new Player(this);
    player->setSavePoint(ppl7::grafix::PointF(1920.0f, 1080.0f));
    player->move(1920.0f, 1080.0f);
    background.init(gpu);
}

Game::~Game()
{
    editor.closeAll();
    deleteUi();
    unloadLevel();
    if (world_widget) {
        this->removeChild(world_widget);
        delete world_widget;
        world_widget = NULL;
    }
    if (player) {
        delete player;
        player = NULL;
    }
}

ppltk::Window& Game::window()
{
    return *this;
}

void Game::init()
{
    translator.load();
    translator.setLanguage(config.TextLanguage);
    wm->useGPUAPI(gpu.gpu);
    // wm->enableGPURenderer(gpu.gpu);
    createWindow();
    gpu.initializeWindow(sdl_window);
    sdl.setGPUDevice(gpu.gpu);
    renderer.init(gpu, sdl_window);
    // Initialize projection/view matrices for rendering
    renderer.batcher.updateMatrices(game_viewport.getLogicalSize());
    initUi();
    initAudio();
    initGameController();
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
    enableFixedUiSize(false, 1920, 1080);
    setWindowTitle("GD2 Prototype");
    ppl7::grafix::Image icon;
    icon.load("res/icon_128.png");
    setWindowIcon(icon);
    setRGBFormat(ppl7::grafix::RGBFormat::A8R8G8B8);
    setBackgroundColor(ppl7::grafix::Color(0, 0, 0, 0));
    setSize(config.ScreenResolution);
    wm->createWindow(*this);
    sdl_window = (SDL_Window*)getSDLWindow();
    SDL_ShowWindow(sdl_window);
    sdl_renderer = (SDL_Renderer*)getRenderer();
    sdl.setRenderer(sdl_renderer);
    // setPos(0,0);
    // SDL_RenderSetLogicalSize(renderer, 1920, 1080);
    wm->setGameControllerFocus(this);

    // SDL_HideCursor();
    SDL_HideCursor();

    // WidgetDrawbuffer.create(1920, 1080, ppl7::grafix::RGBFormat::A8R8G8B8);
    // this->setWidgetDrawbuffer(&WidgetDrawbuffer);
    editor.init(*this);
}

void Game::init_grafix()
{
    resources.load(gpu);

    player->setSpriteResource(resources.Player);
    player->setTileTypeResource(resources.TileTypes);

    // ppl7::PrintDebug("Grafix initialized\n");
    level.setTileset(static_cast<int>(Resources::TileSets::Granit),
                     &resources.Tiles[static_cast<int>(Resources::TileSets::Granit)].Sprites);
    level.setTileTypeSpriteset(&resources.TileTypes);

    level.setSpriteset(static_cast<int>(Resources::SpriteSets::Trees),
                       &resources.SpriteSets[static_cast<int>(Resources::SpriteSets::Trees)].Sprites);

    level.setSpritesetResources(resources.object_spritesets);
}

void Game::initUi()
{
    ppl7::grafix::Grafix* gfx = ppl7::grafix::GetGrafix();
    gfx->loadFont("res/fonts/notosans.fnt6", "NotoSans");
    gfx->loadFont("res/fonts/notosans-black.fnt6", "NotoSansBlack");
    Style.labelFont.setName("NotoSans");
    Style.buttonFont.setName("NotoSans");
    Style.buttonFont.setBold(true);
    Style.inputFont.setName("NotoSans");
    wm->setWidgetStyle(Style);

    // const ppl7::grafix::Size& desktop=clientSize();
    ppl7::grafix::Size desktop;
    desktop.setSize(1920, 1080);
    // ppltk::Label *label;

    resizeMenueAndStatusbar();
    viewport.y1 = 33;
    viewport.y2 = desktop.height - 33;

    world_widget = new WorldWidget();
    world_widget->create(0, 32, desktop.width, desktop.height - 64);
    world_widget->setEventHandler(this);
    world_widget->setViewport(viewport);
    this->addChild(world_widget);
    wm->setKeyboardFocus(world_widget);
    game_viewport.setViewport(viewport);
}

void Game::initAudio()
{
    audiosystem.init();
    audiosystem.setGlobalVolume(config.volumeTotal);
    audiosystem.setVolume(AudioClass::Effect, config.volumeEffects);
    audiosystem.setVolume(AudioClass::Music, config.volumeMusic);
    audiopool.load();
    audiopool.load_speech(config.SpeechLanguage);
    audiopool.setAudioSystem(&audiosystem);
}

void Game::resizeMenueAndStatusbar()
{
    const ppl7::grafix::Size& desktop = clientSize();
    if (!editor.statusbar) {
        editor.statusbar = new StatusBar(0, desktop.height - 32, desktop.width, 32);
        this->addChild(editor.statusbar);
    } else {
        // ppl7::PrintDebug("Resizing statusbar to %d:%d, %dx%d\n", 0, desktop.height - 32, desktop.width, 32);
        editor.statusbar->resize(0, desktop.height - 32, desktop.width, 32);
    }

    if (!editor.mainmenue) {
        editor.mainmenue = new MainMenue(0, 0, desktop.width, 32, this);
        this->addChild(editor.mainmenue);
    } else {
        editor.mainmenue->resize(0, 0, desktop.width, 32);
    }
}

void Game::deleteUi()
{
    if (world_widget) {
        this->removeChild(world_widget);
        delete world_widget;
        world_widget = NULL;
    }
    if (editor.statusbar) {
        this->removeChild(editor.statusbar);
        delete editor.statusbar;
        editor.statusbar = NULL;
    }
    if (editor.mainmenue) {
        this->removeChild(editor.mainmenue);
        delete editor.mainmenue;
        editor.mainmenue = NULL;
    }
}

void Game::initGameController()
{
    std::list<GameController::Device> device_list = GameController::enumerate();
    if (device_list.size() > 0) {
        controller.open(device_list.front());
    }
    updateGameControllerMapping();
}

void Game::updateGameControllerMapping()
{
    controller.setDeadzone(config.controller.deadzone);
    controller.mapping.setMappingAxis(GameControllerMapping::Axis::Walk, config.controller.axis_walk);
    controller.mapping.setMappingAxis(GameControllerMapping::Axis::UpDown, config.controller.axis_updown);
    controller.mapping.setMappingButton(GameControllerMapping::Button::MenuUp, config.controller.button_up);
    controller.mapping.setMappingButton(GameControllerMapping::Button::MenuDown, config.controller.button_down);
    controller.mapping.setMappingButton(GameControllerMapping::Button::MenuLeft, config.controller.button_left);
    controller.mapping.setMappingButton(GameControllerMapping::Button::MenuRight, config.controller.button_right);
    controller.mapping.setMappingButton(GameControllerMapping::Button::Menu, config.controller.button_menu);
    controller.mapping.setMappingButton(GameControllerMapping::Button::Action, config.controller.button_action);
    controller.mapping.setMappingButton(GameControllerMapping::Button::Jump, config.controller.button_jump);
    controller.mapping.setMappingButton(GameControllerMapping::Button::Back, config.controller.button_back);
    controller.mapping.setMappingButton(GameControllerMapping::Button::Crouch, config.controller.button_crouch);
    controller.mapping.setMappingButton(GameControllerMapping::Button::Light, config.controller.button_light);
    controller.mapping.setMappingButton(GameControllerMapping::Button::Dash, config.controller.button_dash);
    controller.mapping.setMappingButton(GameControllerMapping::Button::Inventory, config.controller.button_inventory);
    controller.mapping.setMappingButton(GameControllerMapping::Button::Map, config.controller.button_map);
    controller.mapping.setMappingButton(GameControllerMapping::Button::Block, config.controller.button_block);

    controller.mapping.updateMapping();
}

/*
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
    */

void Game::showUi(bool enable)
{
    // const ppl7::grafix::Size& desktop=clientSize();
    showui = enable;
    world_widget->setShowUi(showui);
    // hud->setEditorMode(enable);
    if (showui) {
        viewport.y1 = 32;
        viewport.y2 = 1080 - 32;
        viewport.x1 = 0;
        viewport.x2 = 1920;
        world_widget->setViewport(viewport);
        game_viewport.setViewport(viewport);
        editor.mainmenue->setVisible(true);
        editor.mainmenue->fitMetrics(viewport);
        editor.statusbar->setVisible(true);
    } else {
        editor.closeAll();
        editor.mainmenue->setShowTileTypes(false);
        editor.mainmenue->setWorldFollowsPlayer(true);
        editor.mainmenue->setVisible(false);
        // mainmenue->visibility_hud=true;
        editor.statusbar->setVisible(false);
        viewport.y1 = 0;
        viewport.x1 = 0;
        viewport.y2 = 1080;
        viewport.x2 = 1920;
        editor.mainmenue->fitMetrics(viewport);
        // ppl7::PrintDebug("Game::showUi: Viewport set to x1=%d, y1=%d, x2=%d, y2=%d\n", viewport.x1, viewport.y1, viewport.x2,
        // viewport.y2);
        game_viewport.setViewport(viewport);
        world_widget->setViewport(viewport);
    }
    // hud->setViewport(viewport);
}

void Game::run()
{
    // this->printChildsTree();
    resizeEvent(NULL);
    world_widget->setVisible(true);
    world_widget->setEnabled(true);
    wm->setKeyboardFocus(world_widget);
    wm->setGameControllerFocus(this);
    level.setPlayer(player);

    showUi(true);
    SDL_ShowCursor();
    level.setShowTileGrid(true);
    level.setShowTileTypes(true);
    sdl.setCursor(resources.Cursor.getDrawable(10), resources.Cursor.getPivot(10));
    clock.update();
    double frame_time_accumulator = 0.0f;
    double idle_time_accumulator = 0.0f;
    Metrics last_metrics;
    metrics.clear();

    uint64_t last_second = clock.current_second;
    quitGame = false;
    while (!quitGame) {
        double idle_start_time = ppl7::GetMicrotime();
        if (!renderer.accuireGPUCommandBuffer()) continue;
        clock.update();
        clock.gpu_wait_fsync_time = clock.time - idle_start_time;
        idle_time_accumulator += clock.gpu_wait_fsync_time;
        if (clock.current_second > last_second) {
            // Update Metrics
            last_second = clock.current_second;
            last_metrics = metrics.getAverage();
            metrics.clear();
            editor.mainmenue->updateMetrics(last_metrics);
        }
        metrics.newFrame();
        metrics.time_frame.start();
        metrics.time_total.start();

        metrics.time_events.start();

        // Handle events muss vor level.update ausgeführt werden,damit
        // z.B. im Editor gelöschte Objekte nicht mehr aktualisiert werden
        wm->handleEvents();

        level.update(clock, metrics, WorldCamera, game_viewport.getLogicalSize());

        ppltk::MouseState mouse = wm->getMouseState();
        editor.mouse = mouse;
        updateUi(mouse, last_metrics);
        if (filedialog) checkFileDialog();
        metrics.time_events.stop();

        player->WorldCoords = WorldCamera;
        player->Viewport = game_viewport;
        if (this->controlsEnabled || player->isAutoWalk()) {
            ParallaxLayerId player_layer = player->getParallaxLayer();
            ParallaxLayer& layer = level.layer(player_layer);
            player->update(clock, layer);
        }

        WorldCamera.setFollowPlayer(editor.mainmenue->worldFollowsPlayer());
        WorldCamera.setRenderSize(game_viewport.getLogicalSize());
        WorldCamera.update(clock, player);

        renderer.batcher.clearQueues();

        // Ensure Buffers matches window size
        int w, h;
        SDL_GetWindowSizeInPixels(sdl_window, &w, &h);
        game_viewport.setWindowSize(ppl7::grafix::Size(w, h));
        renderer.resizeRenderBuffer(game_viewport.getRenderSize());

        // World
        drawWorld();

        // HUD
        drawHUD();

        // Ui and Mouse if enabled
        drawUi();

        // Frame done
        renderer.submitGPUCommandBuffer();
        renderer.batcher.resetContextSwitchCount(); // For debugging: Count how many times we switch GPU context (render pass)

        metrics.time_frame.stop();
        metrics.time_total.stop();
        double frame_time = ppl7::GetMicrotime() - clock.time;

        frame_time_accumulator += frame_time;
        if ((clock.frame_count % 60) == 0) {
            // ppl7::PrintDebug("Frametime: %0.3f ms\n", 1000.0 * (frame_time / frame_count));
            // editor.statusbar->setFrameTime(1000.0 * (frame_time_accumulator / 60.0f));
            // editor.statusbar->setLoad(frame_time_accumulator / idle_time_accumulator * 100.0f);
            // editor.statusbar->setFrameTime(1000.0 * (clock.gpu_wait_fsync_time));
            // editor.statusbar->setFps(clock.fps);
            frame_time_accumulator = 0.0f;
            idle_time_accumulator = 0.0f;
        }
        // Framerate-Limitierung (z.B. 60 FPS)
        clock.limit(60);
    }
}

const ppl7::grafix::PointF& Game::getWorldCoords() const
{
    return WorldCamera;
}

const GameViewport& Game::getGameViewport() const
{
    return game_viewport;
}

const Camera& Game::getCamera() const
{
    return WorldCamera;
}

void Game::clearScreen(SDL_GPUCommandBuffer* cmdbuf, SDL_GPUTexture* swapchainTexture)
{
    SDL_GPUColorTargetInfo colorTargetInfo = {0};
    colorTargetInfo.texture = swapchainTexture;
    colorTargetInfo.clear_color = (SDL_FColor){0.3f, 0.0f, 0.0f, 1.0f}; // Black background
    colorTargetInfo.load_op = SDL_GPU_LOADOP_CLEAR;
    colorTargetInfo.store_op = SDL_GPU_STOREOP_STORE;
    colorTargetInfo.cycle = false; // CRITICAL: SDL examples use false!

    SDL_GPURenderPass* renderPass = SDL_BeginGPURenderPass(cmdbuf, &colorTargetInfo, 1, NULL);
    SDL_EndGPURenderPass(renderPass);
}

void Game::updateUi(const ppltk::MouseState& mouse, const Metrics& last_metrics)
{
    // if (mouse.p.inside(game_viewport)) {
    moveWorldOnMouseClick(mouse);
    editor.statusbar->setFps(clock.fps);
    editor.statusbar->setLoad(last_metrics.time_total.get() * 100.0f / last_metrics.time_frame.get());
    editor.statusbar->setFrameTime(last_metrics.time_total.get() * 1000.0f);
    editor.statusbar->setMouse(mouse);
    editor.statusbar->setWorldCoords(WorldCamera);

    ParallaxLayerId current_layer = editor.mainmenue->currentLayer();
    // ppl7::PrintDebug("Current Layer: %d\n", static_cast<int>(current_layer));
    level.setEditLayer(current_layer);
    WorldCamera.setFollowPlayer(editor.mainmenue->worldFollowsPlayer());

    // Update visibility settings
    level.setShowTileGrid(editor.mainmenue->visibility_grid);
    level.setShowTileTypes(editor.mainmenue->visibility_tiletypes);
    level.setShowCollisions(editor.mainmenue->visibility_collision);
    level.setShowTiles(editor.mainmenue->visibility_tiles);
    level.setShowSprites(editor.mainmenue->visibility_sprites);
    level.setShowObjects(editor.mainmenue->visibility_objects);
    level.setShowParticles(editor.mainmenue->visibility_particles);
    level.setLightsEnabled(editor.mainmenue->visibility_lighting);

    for (int i = 0; i < static_cast<int>(ParallaxLayerId::MaxLayerId); i++) {
        auto& layer = level.layer(static_cast<ParallaxLayerId>(i));
        layer.isVisible = editor.mainmenue->layer_visibility[i];
        layer.bBlurEnabled = editor.mainmenue->visibility_blur;
        layer.bLightningEnabled = editor.mainmenue->visibility_lighting;
        layer.bShowSprites = editor.mainmenue->visibility_sprites;
        layer.bShowObjects = editor.mainmenue->visibility_objects;
        layer.bShowParticles = editor.mainmenue->visibility_particles;
    }

    if (player) editor.statusbar->setPlayerCoords(ppl7::grafix::Point(player->x, player->y));
    if (player) editor.statusbar->setPlayerState(player->getState());

    metrics.frame_rate_compensation = clock.frame_rate_compensation;
    metrics.fps += clock.fps;
    metrics.total_sprites += level.countSprites();
    metrics.visible_sprites += level.countVisibleSprites();
    metrics.total_objects += level.countObjects();
    metrics.visible_objects += level.countVisibleObjects();
    metrics.total_particles += level.countParticles();
    metrics.visible_particles += level.countVisibleParticles();
    metrics.total_lights += level.countLights();
    metrics.visible_lights += level.countVisibleLights();

    if (editor.selected_object) {
        editor.statusbar->setSelectedObject(editor.selected_object->id);
    } else {
        editor.statusbar->setSelectedObject(0);
    }
}

void Game::drawUi()
{
    if (!showui) return;
    metrics.time_draw_ui.start();

    // 1. Draw widgets into PPLTK internal texture
    this->drawWidgets();
    wm->updateGPUTexture(*this, renderer.getCommandBuffer());

    SDL_GPUTexture* gpuTex = (SDL_GPUTexture*)wm->getGPUTexture(*this);

    if (gpuTex) {
        renderer.copyTexture(gpuTex, renderer.getSwapchainTexture(), true);
        /*
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

        SDL_BindGPUGraphicsPipeline(renderPass, renderer.uiPipeline);

        SDL_GPUTextureSamplerBinding binding;
        binding.texture = gpuTex;
        binding.sampler = renderPipelines.samplerClamp;
        SDL_BindGPUFragmentSamplers(renderPass, 0, &binding, 1);

        // Draw Fullscreen Quad
        SDL_DrawGPUPrimitives(renderPass, 3, 1, 0, 0);
        SDL_EndGPURenderPass(renderPass);
        */
    } else {
        ppl7::PrintDebug("ERROR: Could not get GPU Texture from PPLTK UI Surface!\n");
    }
    metrics.time_draw_ui.stop();
}

Player* Game::getPlayer()
{
    return player;
}

struct BlurParams
{
    float blurStrength;
    float padding; // WICHTIG: 4 Bytes Füllmaterial für std140 Alignment
    float texelSizeX;
    float texelSizeY;
};

void Game::drawWorld()
{
    metrics.time_draw_world.start();
    level.draw(&renderer, WorldCamera, game_viewport, metrics);
    metrics.time_draw_world.stop();
}

void Game::drawHUD()
{
    metrics.time_draw_ui.start();
    // TODO
    metrics.time_draw_ui.stop();
}

void Game::quitEvent(ppltk::Event* event)
{
    // ppl7::PrintDebug("Quit event received\n");
    quitGame = true;
}

void Game::closeEvent(ppltk::Event* event)
{
    // ppl7::PrintDebug("Close event received\n");
    quitGame = true;
}

void Game::enableControls(bool enable)
{
    controlsEnabled = enable;
}

bool Game::getControlsEnabled() const
{
    return controlsEnabled;
}

void Game::moveWorld(float offset_x, float offset_y)
{
    if (offset_x == 0 && offset_y == 0) return;
    ParallaxLayer& layer = level.editLayer();
    WorldCamera.x += (offset_x / layer.speed_factor / layer.size_factor);
    WorldCamera.y += (offset_y / layer.speed_factor / layer.size_factor);
    if (WorldCamera.x < 0) WorldCamera.x = 0;
    if (WorldCamera.x > 62000) WorldCamera.x = 62000;
    if (WorldCamera.y < 0) WorldCamera.y = 0;
    if (WorldCamera.y > 62000) WorldCamera.y = 62000;
}

void Game::moveWorldOnMouseClick(const ppltk::MouseState& mouse)
{
    ppl7::grafix::PointF translated_mouse = game_viewport.translate(mouse.p);
    const bool* state = SDL_GetKeyboardState(NULL);
    if (worldIsMoving) {
        if (mouse.buttonMask == ppltk::MouseState::Middle ||
            ((mouse.buttonMask == ppltk::MouseState::Left) && state[SDL_SCANCODE_LSHIFT])) {
            // printf("Move\n");
            moveWorld(WorldMoveStart.x - translated_mouse.x, WorldMoveStart.y - translated_mouse.y);
            WorldMoveStart = translated_mouse;
        } else {
            worldIsMoving = false;
            // printf("End\n");
        }
    } else {
        // printf("mouse.buttonMask=%d\n", mouse.button);
        if (mouse.buttonMask == ppltk::MouseState::Middle ||
            ((mouse.buttonMask == ppltk::MouseState::Left) && state[SDL_SCANCODE_LSHIFT])) {
            // printf("Start\n");
            if (showui) {
                worldIsMoving = true;
                WorldMoveStart = translated_mouse;
                editor.mainmenue->setWorldFollowsPlayer(false);
            }
        } else {
            worldIsMoving = false;
            ;
        }
    }
}

void Game::mouseDownEvent(ppltk::MouseEvent* event)
{
    // ppl7::PrintDebugTime("Game::mouseDownEvent\n");
    if (event->widget() == world_widget) {
        // ppl7::PrintDebugTime("Game::mouseDownEvent\n");
        wm->setKeyboardFocus(world_widget);
        game_viewport.translateMouseEvent(event);
        if (editor.sprite_selection != NULL) {
            editor.mouseDownEventOnSprite(event);
        } else if (editor.object_selection != NULL) {
            editor.mouseDownEventOnObject(event);
        } else if ((editor.tiles_selection != NULL || editor.tiletype_selection != NULL)) {
            editor.handleMouseDrawInWorld(*event);
        } else if (editor.waynet_edit != NULL) {
            editor.mouseDownEventOnWayNet(event);
        } else if (editor.lights_selection != NULL) {
            editor.mouseDownEventOnLight(event);
        }
    } else {
        ppl7::PrintDebugTime("Game::mouseDownEvent outside world_widget: %s\n", (const char*)event->widget()->widgetType());
    }
}

void Game::mouseWheelEvent(ppltk::MouseEvent* event)
{
    if (event->widget() == world_widget) {
        game_viewport.translateMouseEvent(event);
        if (editor.sprite_selection != NULL) {
            editor.mouseWheelEventOnSprite(event);
        }
    }
}

void Game::keyDownEvent(ppltk::KeyEvent* event)
{
    if (event->widget() == world_widget) {
        if (editor.sprite_mode == GameEditor::SpriteMode::Edit && editor.sprite_selection != NULL && editor.selected_sprite.id >= 0 &&
            editor.selected_sprite_system != NULL) {
            if (event->key == ppltk::KeyEvent::KEY_DELETE && (event->modifier & ppltk::KeyEvent::KEYMOD_MODIFIER) == 0) {
                // printf ("KeyEvent\n");
                editor.selected_sprite_system->deleteSprite(editor.selected_sprite.id);
                editor.selected_sprite.id = -1;
                editor.selected_sprite_system = NULL;
            }
        } else if (editor.sprite_mode == GameEditor::SpriteMode::Edit && editor.object_selection != NULL &&
                   editor.selected_object != NULL) {
            if (event->key == ppltk::KeyEvent::KEY_DELETE && (event->modifier & ppltk::KeyEvent::KEYMOD_MODIFIER) == 0) {
                ParallaxLayer& layer = level.layer(editor.selected_object->myParallaxLayer);
                layer.objects.deleteObject(editor.selected_object->id);
                editor.selected_object = NULL;
            }
        }
    }
    if (event->key == ppltk::KeyEvent::KEY_F9) {
        showUi(!showui);
    } else if (event->key == ppltk::KeyEvent::KEY_F10) {
        editor.mainmenue->showMetrics();
        editor.mainmenue->fitMetrics(viewport);
    } else if (event->key == ppltk::KeyEvent::KEY_RETURN && (event->modifier & ppltk::KeyEvent::KEYMOD_ALT) > 0) {
        // printf("toggle fullscreen or back\n");
        ppltk::WindowManager_SDL3* sdl3wm = (ppltk::WindowManager_SDL3*)wm;
        Window::WindowMode mode = sdl3wm->getWindowMode(*this);
        if (mode == Window::WindowMode::Window) {
            windowedSize.setSize(width(), height());
            // printf("Aktueller mode ist Window mit %d x %d\n", windowedSize.width, windowedSize.height);
            ppl7::grafix::Size s = sdl.getDisplaySize(config.videoDevice);
            // printf("switche zu FullscreenDesktop %d x %d\n", s.width, s.height);
            sdl3wm->changeWindowMode(*this, Window::WindowMode::FullscreenDesktop);
            ppltk::Window::DisplayMode dmode;
            dmode.format = rgbFormat();
            dmode.width = s.width;
            dmode.height = s.height;
            dmode.refresh_rate = config.ScreenRefreshRate;
            // ppl7::PrintDebug("Set display mode %d x %d @ %d Hz\n", dmode.width, dmode.height, dmode.refresh_rate);
            setWindowDisplayMode(dmode);
            resizeEvent(NULL);
        } else if (mode == Window::WindowMode::FullscreenDesktop) {
            if (windowedSize.width == 0 || windowedSize.height == 0) windowedSize = config.ScreenResolution;
            // printf("Aktueller mode ist FullscreenDesktop, switche zu Fenster %d x %d\n", windowedSize.width, windowedSize.height);
            sdl3wm->changeWindowMode(*this, Window::WindowMode::Window);
            ppltk::Window::DisplayMode dmode;
            dmode.format = rgbFormat();
            dmode.width = windowedSize.width;
            dmode.height = windowedSize.height;
            dmode.refresh_rate = config.ScreenRefreshRate;
            setWindowDisplayMode(dmode);
            resizeEvent(NULL);
        } else {
            // printf("Aktueller mode ist Fullscreen\n");
        }
    }
}

void Game::mouseMoveEvent(ppltk::MouseEvent* event)
{
    if ((editor.tiles_selection != NULL || editor.tiletype_selection != NULL) && event->widget() == world_widget) {
        game_viewport.translateMouseEvent(event);
        editor.handleMouseDrawInWorld(*event);
    }
    if (editor.sprite_selection != NULL) {
        if (event->widget() == world_widget && event->buttonMask == ppltk::MouseState::Left &&
            editor.sprite_mode == GameEditor::SpriteMode::Edit && editor.selected_sprite.id >= 0 && editor.selected_sprite_system != NULL) {
            game_viewport.translateMouseEvent(event);
            ppl7::grafix::Point diff = event->p - editor.sprite_move_start;
            editor.selected_sprite.x += diff.x;
            editor.selected_sprite.y += diff.y;
            editor.selected_sprite_system->modifySprite(editor.selected_sprite);
            // printf("Move: %d, %d\n", diff.x, diff.y);
            editor.sprite_move_start = event->p;
        }
    } else if (editor.object_selection != NULL) {
        if (event->widget() == world_widget && event->buttonMask == ppltk::MouseState::Left &&
            editor.sprite_mode == GameEditor::SpriteMode::Edit && editor.selected_object != NULL) {
            game_viewport.translateMouseEvent(event);
            ppl7::grafix::Point diff = event->p - editor.sprite_move_start;
            editor.selected_object->initial_p.x += diff.x;
            editor.selected_object->initial_p.y += diff.y;
            editor.selected_object->p = editor.selected_object->initial_p;
            editor.selected_object->updateBoundary();
            editor.sprite_move_start = event->p;
        }
    }
}

void Game::resizeEvent(ppltk::ResizeEvent* event)
{
    resizeMenueAndStatusbar();
    if (world_widget == NULL) return;
    const ppl7::grafix::Size& desktop = clientSize();
    viewport.y1 = 0;
    viewport.y2 = desktop.height;
    viewport.x2 = desktop.width;
    if (showui) {
        viewport.y1 = 32;
        viewport.y2 = desktop.height - 32;
    }
    world_widget->setViewport(viewport);
    // const ppl7::grafix::Size& desktop=clientSize();
}

void Game::gameControllerButtonDownEvent(ppltk::GameControllerButtonEvent* event)
{
    /* TODO:
    GameControllerMapping::Button b = controller.mapping.getButton(event);
    // ppl7::PrintDebugTime("gameControllerButtonDownEvent b=%d\n", (int)b);

    if (b == GameControllerMapping::Button::Menu) {
        if (settings_screen) {
            delete settings_screen;
            settings_screen = NULL;
            enableControls(true);
            wm->setKeyboardFocus(world_widget);
            wm->setGameControllerFocus(this);
        } else
            openSettingsScreen();
    }
    */
}

void Game::gameControllerDeviceAdded(ppltk::GameControllerEvent* event)
{
    // ppl7::PrintDebugTime("gameControllerDeviceAdded: %d\n", event->which);
    controller.open(event->which);
}

void Game::gameControllerDeviceRemoved(ppltk::GameControllerEvent* event)
{
    // ppl7::PrintDebugTime("gameControllerDeviceRemoved: %d\n", event->which);
    controller.close();
}
