
#include "sdl3_if.h"

class Game : private ppltk::Window
{
private:
    SDL3 sdl;
    ppltk::WindowManager* wm;
    ppltk::WidgetStyle Style;

    void createWindow();

public:
    Game();

    void init();
    void init_grafix();

    void loadLevel(const ppl7::String& filename);

    void run();

    // EventHandler
    void quitEvent(ppltk::Event* event);
    void closeEvent(ppltk::Event* event);

};