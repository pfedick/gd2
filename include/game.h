
#include <ppl7-grafix.h>
#include <ppl7.h>

#include "gpu.h"
#include "renderpipelines.h"
#include "resources.h"
#include "sdl.h"
#include "level.h"
#include "gamecontroller.h"

#define APP_COMPANY "Patrick F.-Productions"
#define APP_NAME "The Magican"

class Config
{
private:
    ppl7::String ConfigFile;

public:
    enum class DifficultyLevel
    {
        easy = 1,
        normal = 2,
        hard = 3
    };
    typedef ppltk::Window::WindowMode WindowMode;
    // Video
    int videoDevice;
    ppl7::grafix::Size ScreenResolution;
    int ScreenRefreshRate;
    ppltk::Window::WindowMode windowMode;

    // Audio
    float volumeTotal;
    float volumeMusic;
    float volumeEffects;
    float volumeAmbience;
    float volumeSpeech;

    // Misc
    ppl7::String CustomLevelPath;
    ppl7::String LastEditorLevel;
    ppl7::String TextLanguage;
    ppl7::String SpeechLanguage;
    bool tutorialPlayed;
    bool skipIntro;
    bool noBlood;
    DifficultyLevel difficulty;

    // Controller
    class Controller
    {
    public:
        int deadzone;
        int axis_walk;
        int axis_jump;
        int button_up;
        int button_down;
        int button_left;
        int button_right;
        int button_menu;
        int button_back;
        int button_action;
        int button_jump;
        int button_crouch;
        int button_flashlight;
        bool use_rumble;
        Controller();
    };
    Controller controller;

    Config();
    ~Config();
    void load();
    void save();
};

class FPS
{
private:
    ppl7::ppl_time_t fps_start_time;
    int fps_frame_count;
    int fps;
    bool debug;

public:
    FPS();
    int getFPS() const;
    void update();
    void enableDebug(bool enable);
};

class MainMenue;
class StatusBar;
class TilesSelection;
class Game;
class Player;
class WorldWidget;

class GameViewport : public ppl7::grafix::Rect
{
private:
    int menu_offset_x;
    ppl7::grafix::Size real_viewport;
    ppl7::grafix::Size render_size;
    bool scaling_enabled;
    bool allow_upscale;
    SDL_FRect render_rect;
    void update();

public:
    GameViewport();
    void setRealViewport(const ppl7::grafix::Size& size);
    void setRenderSize(const ppl7::grafix::Size& size);
    void setMenuOffset(int x);
    void setScalingEnabled(bool enable);
    void setAllowUpscale(bool allow);
    ppl7::grafix::Point translate(const ppl7::grafix::Point& coords) const;

    void translateMouseEvent(ppltk::MouseEvent* event);
    void getRenderRect(SDL_FRect& rect) const;
    const SDL_FRect& getRenderRect() const;
};

class GameEditor
{
    friend class Game;

private:
    class History
    {
    public:
        int lastTileset;
        int lastTile;
        int lastTileColor;
        int lastTileLayer;
        History();
        void clear();
    };

    History history;
    Game* game;

    TilesSelection* tiles_selection;
    MainMenue* mainmenue;
    StatusBar* statusbar;

public:
    GameEditor();
    ~GameEditor();
    void init(Game& game);
    void closeAll();
    void showTilesSelection();
    void showTileTypeSelection();
    void showSpriteSelection();
};

class Game : private ppltk::Window
{
    friend class GameEditor;

private:
    SDL sdl;
    SDL_Window* sdl_window;
    SDL_Renderer* sdl_renderer;
    GPUContext& gpu;
    GPUBatcher gpu_batcher;

    ppltk::WindowManager_SDL3* wm;
    ppltk::WidgetStyle Style;

    ppl7::grafix::Image WidgetDrawbuffer;

    /*
    SDL_GPUTexture* render_target_layer;
    SDL_GPUTexture* render_target_tmp1;
    SDL_GPUTexture* render_target_tmp2;
    SDL_GPUTexture* depthTexture;
    */

    ppl7::grafix::Size render_target_size;
    ppl7::grafix::Rect viewport;
    GameViewport game_viewport;
    ppl7::grafix::PointF WorldCoords;

    void createWindow();
    // void createRenderTargetsIfRequired(const ppl7::grafix::Size& size);

    bool quitGame = false;
    bool showui = true;

    void initUi();
    void initGameController();
    void updateGameControllerMapping();
    void deleteUi();
    void resizeMenueAndStatusbar();
    void updateWorldCoords();

    void clearScreen(SDL_GPUCommandBuffer* cmdbuf, SDL_GPUTexture* swapchainTexture);
    void drawUi(SDL_GPUCommandBuffer* cmdbuf, SDL_GPUTexture* swapchainTexture, const ppltk::MouseState& mouse);
    void drawWorld(SDL_GPUCommandBuffer* cmdbuf, SDL_GPUTexture* swapchainTexture);
    void drawHUD(SDL_GPUCommandBuffer* cmdbuf, SDL_GPUTexture* swapchainTexture);

    uint64_t frame_count = 0;
    double time_accumulator = 0.0f;

    ppl7::grafix::Point WorldMoveStart;
    bool worldIsMoving;

    WorldWidget* world_widget;

    Player* player;

public:
    GameEditor editor;
    GameController controller;
    Level level;
    RenderPipelines renderPipelines;
    Resources resources;
    Config config;
    FPS fps;
    Game(GPUContext& gpu);
    ~Game();

    void init();
    void init_grafix();

    void loadLevel(const ppl7::String& filename);
    void startNewLevel(int width, int height);
    void saveLevel(const ppl7::String& filename);

    void run();
    void updateUi(const ppltk::MouseState& mouse);

    void updateSpriteFromUi();

    void moveWorld(float offset_x, float offset_y);
    void moveWorldOnMouseClick(const ppltk::MouseState& mouse);

    Player* getPlayer();

    // EventHandler
    void quitEvent(ppltk::Event* event);
    void closeEvent(ppltk::Event* event);
    void mouseDownEvent(ppltk::MouseEvent* event);
    void mouseWheelEvent(ppltk::MouseEvent* event);
    void keyDownEvent(ppltk::KeyEvent* event);
    void mouseMoveEvent(ppltk::MouseEvent* event);
};

Game& GetGame();