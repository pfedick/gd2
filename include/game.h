
#include <ppl7-grafix.h>
#include <ppl7.h>

#include "gpu.h"
#include "renderpipelines.h"
#include "resources.h"
#include "sdl.h"
#include "metrics.h"
#include "level.h"
#include "gamecontroller.h"
#include "gameviewport.h"
#include "background.h"
#include "translate.h"
#include "audio.h"
#include "audiopool.h"
#include "camera.h"

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
        int axis_updown;
        int button_up;
        int button_down;
        int button_left;
        int button_right;
        int button_menu;
        int button_back;
        int button_action;
        int button_jump;
        int button_crouch;
        int button_light;
        int button_dash;
        int button_inventory;
        int button_map;
        int button_block;
        bool use_rumble;
        Controller();
    };
    Controller controller;

    Config();
    ~Config();
    void load();
    void save();
};

class GameClock
{
private:
    uint64_t fps_frame_count = 0;
    uint64_t fps_start_time = 0;

public:
    void update();
    uint64_t frame_count = 0;
    uint64_t current_second = 0;
    double time = 0.0f;
    float frame_rate_compensation = 0.0f;
    float delta_time = 0.0f;
    float gpu_wait_fsync_time = 0.0f;
    int fps = 0;
};

class MainMenue;
class StatusBar;
class TilesSelection;
class TileTypeSelection;
class ObjectSelection;
class SpriteSelection;
class Game;
class Player;
class WorldWidget;
class FileDialog;

class Soundtrack
{
private:
    AudioStream* playing_song;
    size_t song_index;
    AudioSystem& audiosystem;
    const LevelParameter& params;
    ppl7::String currentSong;

public:
    Soundtrack(AudioSystem& audio, const LevelParameter& level_params);
    ~Soundtrack();
    void update();
    void playInitialSong();
    void playSong(const ppl7::String& filename);
    void fadeout(float seconds);
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
        int lastTileType = 1;
        int ObjectType = 0;
        int ObjectDifficulty = 0xff;
        int ObjectLayer = 0;
        History();
        void clear();
    };

    History history;
    Game* game;

    Objects::Object* selected_object;

    TilesSelection* tiles_selection;
    TileTypeSelection* tiletype_selection;
    SpriteSelection* sprite_selection;
    ObjectSelection* object_selection;
    void* lights_selection;
    void* waynet_edit;

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
    void showObjectSelection();

    void handleMouseDrawInWorld(const ppltk::MouseState& mouse);

    void updateDifficultyForSelectedObject(uint8_t dificulty);
    void updateObjectLayerForSelectedObject(int layer);
    void setSpriteModeToDraw();
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
    Translator translator;

    ppltk::WindowManager_SDL3* wm;
    ppltk::WidgetStyle Style;

    ppl7::grafix::Image WidgetDrawbuffer;

    FileDialog* filedialog;

    ppl7::grafix::Size render_target_size;
    ppl7::grafix::Size windowedSize;
    ppl7::grafix::Rect viewport;
    GameViewport game_viewport;
    Camera WorldCamera;

    void createWindow();
    // void createRenderTargetsIfRequired(const ppl7::grafix::Size& size);

    bool quitGame = false;
    bool showui = true;
    bool controlsEnabled = true;

    void initUi();
    void initAudio();
    void initGameController();
    void updateGameControllerMapping();
    void deleteUi();
    void resizeMenueAndStatusbar();
    void checkFileDialog();

    void clearScreen(SDL_GPUCommandBuffer* cmdbuf, SDL_GPUTexture* swapchainTexture);
    void drawUi(SDL_GPUCommandBuffer* cmdbuf, SDL_GPUTexture* swapchainTexture, const ppltk::MouseState& mouse);
    void drawWorld(SDL_GPUCommandBuffer* cmdbuf, SDL_GPUTexture* swapchainTexture);
    void drawHUD(SDL_GPUCommandBuffer* cmdbuf, SDL_GPUTexture* swapchainTexture);

    ppl7::grafix::PointF WorldMoveStart;
    bool worldIsMoving;

    WorldWidget* world_widget;

    Player* player;

    ppl7::String LevelFile;
    ppl7::String nextLevelFile;
    Background background;

public:
    GameEditor editor;
    GameController controller;
    Level level;
    RenderPipelines renderPipelines;
    Resources resources;
    AudioPool audiopool;
    AudioSystem audiosystem;
    Config config;
    GameClock clock;
    Metrics metrics;

private:
    Soundtrack soundtrack = Soundtrack(audiosystem, level.params);

public:
    Game(GPUContext& gpu);
    ~Game();

    ppltk::Window& window();

    void init();
    void init_grafix();

    void run();
    void updateUi(const ppltk::MouseState& mouse, const Metrics& metrics);

    void showUi(bool enable);
    void updateSpriteFromUi();

    void moveWorld(float offset_x, float offset_y);
    void moveWorldOnMouseClick(const ppltk::MouseState& mouse);

    const ppl7::grafix::Rect& getViewport() const;
    const ppl7::grafix::PointF& getWorldCoords() const;

    void enableControls(bool enable);
    bool getControlsEnabled() const;

    Player* getPlayer();

    // Level Management
    void startLevel(const ppl7::String& filename);
    void unloadLevel();
    bool nextLevel(const ppl7::String& filename);
    void save(const ppl7::String& filename);
    void load();
    void createNewLevel(const LevelParameter& params);
    void updateFromLevelParameters();
    void openSaveAsDialog();
    void openLoadDialog();
    void openNewLevelDialog();
    const ppl7::String& getLevelFilename() const;

    // EventHandler
    void quitEvent(ppltk::Event* event);
    void closeEvent(ppltk::Event* event);
    void mouseDownEvent(ppltk::MouseEvent* event);
    void mouseWheelEvent(ppltk::MouseEvent* event);
    void keyDownEvent(ppltk::KeyEvent* event);
    void mouseMoveEvent(ppltk::MouseEvent* event);
    void resizeEvent(ppltk::ResizeEvent* event);
    void gameControllerButtonDownEvent(ppltk::GameControllerButtonEvent* event);
    void gameControllerDeviceAdded(ppltk::GameControllerEvent* event);
    void gameControllerDeviceRemoved(ppltk::GameControllerEvent* event);
};

Game& GetGame();
ppltk::Window* GetGameWindow();