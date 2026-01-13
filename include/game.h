
#include <ppl7.h>
#include <ppl7-grafix.h>
#include "sdl.h"
#include "gpu.h"
#include "resources.h"
#include "renderpipelines.h"

#define APP_COMPANY "Patrick F.-Productions"
#define APP_NAME "The Magican"


class Config
{
private:
    ppl7::String		ConfigFile;
public:
    enum class DifficultyLevel
    {
        easy = 1,
        normal = 2,
        hard = 3
    };
    typedef ppltk::Window::WindowMode WindowMode;
    // Video
    int					videoDevice;
    ppl7::grafix::Size	ScreenResolution;
    int 				ScreenRefreshRate;
    ppltk::Window::WindowMode windowMode;

    // Audio
    float				volumeTotal;
    float				volumeMusic;
    float				volumeEffects;
    float				volumeAmbience;
    float				volumeSpeech;

    // Misc
    ppl7::String		CustomLevelPath;
    ppl7::String		LastEditorLevel;
    ppl7::String		TextLanguage;
    ppl7::String		SpeechLanguage;
    bool				tutorialPlayed;
    bool				skipIntro;
    bool				noBlood;
    DifficultyLevel		difficulty;

    // Controller
    class Controller {
    public:
        int					deadzone;
        int					axis_walk;
        int					axis_jump;
        int					button_up;
        int					button_down;
        int					button_left;
        int					button_right;
        int					button_menu;
        int					button_back;
        int					button_action;
        int					button_jump;
        int					button_crouch;
        int					button_flashlight;
        bool				use_rumble;
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



class Game : private ppltk::Window
{
private:
    SDL sdl;
    SDL_Window* sdl_window;
    GPUContext& gpu;
    GPUBatcher gpu_batcher;

    ppltk::WindowManager_SDL3* wm;
    ppltk::WidgetStyle Style;

    ppl7::grafix::Image WidgetDrawbuffer;

    void createWindow();

    bool quitGame = false;
    bool showui = true;

    void drawUi(SDL_GPUCommandBuffer* cmdbuf, SDL_GPUTexture* swapchainTexture, const ppltk::MouseState& mouse);
    void drawWorld(SDL_GPUCommandBuffer* cmdbuf, SDL_GPUTexture* swapchainTexture);
    void drawHUD(SDL_GPUCommandBuffer* cmdbuf, SDL_GPUTexture* swapchainTexture);

    uint64_t frame_count = 0;
    double time_accumulator = 0.0f;

public:
    RenderPipelines renderPipelines;
    Resources resources;
    Config config;
    FPS fps;
    Game(GPUContext& gpu);

    void init();
    void init_grafix();

    void loadLevel(const ppl7::String& filename);

    void run();
    void updateUi(const ppltk::MouseState& mouse);

    // EventHandler
    void quitEvent(ppltk::Event* event);
    void closeEvent(ppltk::Event* event);

};