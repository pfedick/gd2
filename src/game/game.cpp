#include "game.h"
#include <ppltk.h>
#include "player.h"
#include "ui/menue.h"
#include "ui/statusbar.h"
#include "ui/worldwidget.h"
#include "constants.h"
#include "translate.h"

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

ppltk::Window* GetGameWindow()
{
    if (!game) throw ppl7::Exception("Game not initialized!");
    return &game->window();
}

Game::Game(GPUContext& gpu)
    : ppltk::Window(),
      gpu(gpu)
{
    game = this;
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
    last_frame_time = 0.0f;
    frame_rate_compensation = 1.0f;
    game_viewport.setRenderSize(ppl7::grafix::Size(3840, 2160));
    game_viewport.setAspectRatio(16.0f / 9.0f);
    player = new Player(this);
    player->setSavePoint(ppl7::grafix::PointF(1920.0f, 1080.0f));
    player->move(1920.0f, 1080.0f);
}

Game::~Game()
{
    if (world_widget) {
        this->removeChild(world_widget);
        delete world_widget;
        world_widget = NULL;
    }
    if (player) {
        delete player;
        player = NULL;
    }
    deleteUi();
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
    gpu.initializeWindow((SDL_Window*)getSDLWindow());
    sdl.setGPUDevice(gpu.gpu);
    gpu_batcher.init(&gpu); // Now using Storage Buffers instead of vertex buffer instancing

    renderPipelines.init(gpu.gpu, (SDL_Window*)getSDLWindow());
    // Initialize projection/view matrices for rendering
    gpu_batcher.updateMatrices(1920, 1080);
    level.initialize(gpu, renderPipelines, gpu_batcher);
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
    controller.mapping.setMappingAxis(GameControllerMapping::Axis::Jump, config.controller.axis_jump);
    controller.mapping.setMappingButton(GameControllerMapping::Button::MenuUp, config.controller.button_up);
    controller.mapping.setMappingButton(GameControllerMapping::Button::MenuDown, config.controller.button_down);
    controller.mapping.setMappingButton(GameControllerMapping::Button::MenuLeft, config.controller.button_left);
    controller.mapping.setMappingButton(GameControllerMapping::Button::MenuRight, config.controller.button_right);
    controller.mapping.setMappingButton(GameControllerMapping::Button::Menu, config.controller.button_menu);
    controller.mapping.setMappingButton(GameControllerMapping::Button::Action, config.controller.button_action);
    controller.mapping.setMappingButton(GameControllerMapping::Button::Jump, config.controller.button_jump);
    controller.mapping.setMappingButton(GameControllerMapping::Button::Back, config.controller.button_back);
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
        // editor.mainmenue->fitMetrics(viewport);
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
        // editor.mainmenue->fitMetrics(viewport);
        world_widget->setViewport(viewport);
        game_viewport.setViewport(viewport);
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

    showUi(true);
    SDL_ShowCursor();
    level.setShowTileGrid(true);
    level.setShowTileTypes(true);
    sdl.setCursor(resources.Cursor.getDrawable(10), resources.Cursor.getPivot(10));
    ppl7::ppl_time_t last_second = ppl7::GetTime();
    quitGame = false;
    while (!quitGame) {
        double start_time = ppl7::GetMicrotime();

        ppl7::ppl_time_t current_second = ppl7::GetTime();
        if (current_second > last_second) {
            last_second = current_second;
            // TODO: Update Metrics
        }
        frame_rate_compensation = 1.0f;
        if (last_frame_time > 0.0f) {
            float frametime = start_time - last_frame_time;
            frame_rate_compensation = frametime / (1.0f / 60.0f);
        }
        last_frame_time = start_time;
        wm->handleEvents();
        ppltk::MouseState mouse = wm->getMouseState();
        updateUi(mouse);
        if (filedialog) checkFileDialog();

        player->WorldCoords = WorldCoords;
        player->Viewport = game_viewport;
        if (this->controlsEnabled || player->isAutoWalk()) {
            ParallaxLayerId player_layer = player->getParallaxLayer();
            ParallaxLayer& layer = level.layer(player_layer);
            player->update(start_time, layer, frame_rate_compensation);
        }
        WorldCoords.setFollowPlayer(editor.mainmenue->worldFollowsPlayer());
        WorldCoords.setRenderSize(game_viewport.getRenderSize());
        WorldCoords.update(start_time, frame_rate_compensation, player);

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
        // clearScreen(cmdbuf, swapchainTexture);

        // Ensure Depth Buffer matches window size
        int w, h;
        SDL_GetWindowSizeInPixels(sdl_window, &w, &h);
        game_viewport.setWindowSize(ppl7::grafix::Size(w, h));
        level.resizeRenderBuffer(game_viewport.getRenderSize());
        gpu_batcher.updateMatrices(game_viewport.getRenderSize());
        // createRenderTargetsIfRequired(ppl7::grafix::Size(w, h));

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
            // ppl7::PrintDebug("Frametime: %0.3f ms\n", 1000.0 * (frame_time / frame_count));
            editor.statusbar->setFrameTime(1000.0 * (frame_time / frame_count));
            frame_count = 0;
            time_accumulator = 0.0f;
        }
    }
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

void Game::updateUi(const ppltk::MouseState& mouse)
{
    // if (mouse.p.inside(game_viewport)) {
    moveWorldOnMouseClick(mouse);
    editor.statusbar->setWorldCoords(WorldCoords);
    ParallaxLayerId current_layer = editor.mainmenue->currentLayer();
    // ppl7::PrintDebug("Current Layer: %d\n", static_cast<int>(current_layer));
    level.setEditLayer(current_layer);
    WorldCoords.setFollowPlayer(editor.mainmenue->worldFollowsPlayer());

    // Update visibility settings
    level.setShowTileGrid(editor.mainmenue->visibility_grid);
    level.setShowTileTypes(editor.mainmenue->visibility_tiletypes);
    level.setShowSprites(editor.mainmenue->visibility_sprites);
    level.setShowObjects(editor.mainmenue->visibility_objects);
    level.setShowParticles(editor.mainmenue->visibility_particles);
    level.setLightingEnabled(editor.mainmenue->visibility_lighting);
    for (int i = 0; i < static_cast<int>(ParallaxLayerId::MaxLayerId); i++) {
        level.layer(static_cast<ParallaxLayerId>(i)).setVisible(editor.mainmenue->layer_visibility[i]);
    }

    if (player) editor.statusbar->setPlayerCoords(ppl7::grafix::Point(player->x, player->y));
    if (player) editor.statusbar->setPlayerState(player->getState());
}

void Game::drawUi(SDL_GPUCommandBuffer* cmdbuf, SDL_GPUTexture* swapchainTexture, const ppltk::MouseState& mouse)
{
    if (!showui) return;
    editor.statusbar->setFps(fps.getFPS());

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

void Game::drawWorld(SDL_GPUCommandBuffer* cmdbuf, SDL_GPUTexture* swapchainTexture)
{
    level.updateVisibleObjects(WorldCoords, game_viewport.getWindowSize());
    level.draw(cmdbuf, swapchainTexture, WorldCoords, game_viewport, player);

#ifdef OLDCODE
    //  Start render pass (resets z-order counter)
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
    SDL_GPUViewport sviewport = {0.0f, 0.0f, 1920.0f, 1080.0f, 0.0f, 1.0f};
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
    SDL_SetGPUViewport(renderPass, &sviewport);
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
    SDL_SetGPUViewport(renderPass, &sviewport);
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
#endif

    // #endif
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
    WorldCoords.x += (offset_x / layer.speed_factor / layer.size_factor);
    WorldCoords.y += (offset_y / layer.speed_factor / layer.size_factor);
    if (WorldCoords.x < 0) WorldCoords.x = 0;
    if (WorldCoords.x > 62000) WorldCoords.x = 62000;
    if (WorldCoords.y < 0) WorldCoords.y = 0;
    if (WorldCoords.y > 62000) WorldCoords.y = 62000;
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
            // mouseDownEventOnSprite(event);
        } else if (editor.object_selection != NULL) {
            // mouseDownEventOnObject(event);
        } else if ((editor.tiles_selection != NULL || editor.tiletype_selection != NULL)) {
            editor.handleMouseDrawInWorld(*event);
        } else if (editor.waynet_edit != NULL) {
            // mouseDownEventOnWayNet(event);
        } else if (editor.lights_selection != NULL) {
            // mouseDownEventOnLight(event);
        }
    } else {
        ppl7::PrintDebugTime("Game::mouseDownEvent outside world_widget: %s\n", (const char*)event->widget()->widgetType());
    }
}

void Game::mouseWheelEvent(ppltk::MouseEvent* event)
{
}

void Game::keyDownEvent(ppltk::KeyEvent* event)
{
    if (event->key == ppltk::KeyEvent::KEY_F9) {
        showUi(!showui);
    }
}

void Game::mouseMoveEvent(ppltk::MouseEvent* event)
{
    if ((editor.tiles_selection != NULL || editor.tiletype_selection != NULL) && event->widget() == world_widget) {
        game_viewport.translateMouseEvent(event);
        editor.handleMouseDrawInWorld(*event);
    }
}

void Game::resizeEvent(ppltk::ResizeEvent* event)
{
    resizeMenueAndStatusbar();
    if (world_widget == NULL) return;
    const ppl7::grafix::Size& desktop = clientSize();
    viewport.y1 = 33;
    viewport.y2 = desktop.height - 33;
    viewport.x2 = desktop.width;
    world_widget->setViewport(viewport);
    // const ppl7::grafix::Size& desktop=clientSize();
}