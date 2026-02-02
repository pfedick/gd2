
#include <ppl7-grafix.h>
#include <ppl7.h>

#include "gpu.h"
#include "renderpipelines.h"
#include "resources.h"
#include "sdl.h"
#include "level.h"
#include "gamecontroller.h"
#include "gameviewport.h"
#include "background.h"
#include "translate.h"
#include "audio.h"
#include "audiopool.h"

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
class TileTypeSelection;
class Game;
class Player;
class WorldWidget;
class FileDialog;

class Camera : public ppl7::grafix::PointF
{
private:
    float zoom;
    float target_zoom;
    float zoom_speed;
    float look_ahead_x;        // Der aktuelle gleitende Vorsprung
    float look_ahead_distance; // Maximaler Vorsprung
    ppl7::grafix::PointF dead_zone;
    ppl7::grafix::PointF speed;
    ppl7::grafix::PointF player_position;
    ppl7::grafix::Size render_size;
    bool follow_player;

public:
    Camera();
    void setZoom(float zoom);
    float getZoom() const;
    void setTargetZoom(float zoom, float speed);
    void setRenderSize(const ppl7::grafix::Size& size);
    void update(double time, float frame_rate_compensation, const Player* player);
    void setPosition(const ppl7::grafix::PointF& pos);
    void setDeadZone(float x, float y);
    void setFollowPlayer(bool enable);
    bool isFollowingPlayer() const;
};

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
        History();
        void clear();
    };

    History history;
    Game* game;

    void* selected_object;

    TilesSelection* tiles_selection;
    TileTypeSelection* tiletype_selection;
    void* sprite_selection;
    void* object_selection;
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

    void handleMouseDrawInWorld(const ppltk::MouseState& mouse);
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
    ppl7::grafix::Rect viewport;
    GameViewport game_viewport;
    Camera WorldCoords;
    double last_frame_time;
    float frame_rate_compensation;

    void createWindow();
    // void createRenderTargetsIfRequired(const ppl7::grafix::Size& size);

    bool quitGame = false;
    bool showui = true;
    bool controlsEnabled = true;

    Soundtrack soundtrack = Soundtrack(audiosystem, level.params);

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

    uint64_t frame_count = 0;
    double time_accumulator = 0.0f;

    ppl7::grafix::PointF WorldMoveStart;
    bool worldIsMoving;

    WorldWidget* world_widget;

    Player* player;

    ppl7::String LevelFile;
    ppl7::String nextLevelFile;
    Background background = Background(gpu);

public:
    GameEditor editor;
    GameController controller;
    Level level;
    RenderPipelines renderPipelines;
    Resources resources;
    AudioPool audiopool;
    AudioSystem audiosystem;
    Config config;
    FPS fps;
    Game(GPUContext& gpu);
    ~Game();

    ppltk::Window& window();

    void init();
    void init_grafix();

    void run();
    void updateUi(const ppltk::MouseState& mouse);

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
};

Game& GetGame();
ppltk::Window* GetGameWindow();