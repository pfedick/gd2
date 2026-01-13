#include <stdio.h>
#include <ppl7-grafix.h>
#include <ppltk.h>
#include <SDL3/SDL.h>
#include <locale.h>

#include "game.h"

#define DEBUGTIME

void help()
{
    printf("Usage:\n\n");
    printf("  -l FILE    load a level without presenting the start screen\n");
    fflush(stdout);
}

void start(int argc, char** argv)
{
#ifdef WIN32
    ppl7::String::setGlobalEncoding("UTF-8");
#endif

    if (setlocale(LC_CTYPE, "") == NULL) {
        printf("setlocale fehlgeschlagen\n");
        throw std::exception();
    }
    if (setlocale(LC_NUMERIC, "C") == NULL) {
        printf("setlocale fuer LC_NUMERIC fehlgeschlagen\n");
        throw std::exception();
    }

    if (ppl7::HaveArgv(argc, argv, "-h") || ppl7::HaveArgv(argc, argv, "--help")) {
        help();
        return;
    }


    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS | SDL_INIT_GAMEPAD);

    ppl7::grafix::Grafix gfx;
    ppltk::WindowManager_SDL3 wm;

    /*
    SDL_GPUDevice* gpu = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV, true, "vulkan");
    if (!gpu) {
        printf("Failed to create GPU device: %s", SDL_GetError());
        return;
    }
    wm.enableGPURenderer(gpu);
    */

    GPUContext gpu;
    gpu.initializeGPUDevice();
    Game game(gpu);
    try {
        game.init();
        game.init_grafix();
        if (ppl7::HaveArgv(argc, argv, "-l")) {
            ppl7::String level = ppl7::GetArgv(argc, argv, "-l");
            //game.loadLevel(level);
        }
        game.run();

    }
    catch (const ppl7::Exception& ex) {
        ex.print();
        throw;
    }



    //game.audiosystem.shutdown();
}

#ifdef WIN32
int WinMain()
{
    start(__argc, __argv);
    return 0;
    try {
        start(__argc, __argv);
        return 0;
    }
    catch (const ppl7::Exception& ex) {
        ex.print();
        throw;
        return 1;
    }
    return 0;
}
#endif

int main(int argc, char** argv)
{

    start(argc, argv);
    return 0;
}
