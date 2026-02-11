#include <SDL3/SDL.h>
#include "player.h"
#include "game.h"
#include "gamecontroller.h"

KeyState::KeyState(Game* game)
{
    this->game = game;
}

void KeyState::update(double time)
{
    const bool* state = SDL_GetKeyboardState(NULL);
    left = false;
    right = false;
    up = false;
    down = false;
    action = false;
    block = false;
    dash = false;
    crouch = false;
    jump = false;
    if (state[SDL_SCANCODE_LEFT] || state[SDL_SCANCODE_A]) left = true;
    if (state[SDL_SCANCODE_RIGHT] || state[SDL_SCANCODE_D]) right = true;
    if (state[SDL_SCANCODE_UP] || state[SDL_SCANCODE_W]) up = true;
    if (state[SDL_SCANCODE_DOWN] || state[SDL_SCANCODE_S]) down = true;
    if (state[SDL_SCANCODE_LSHIFT] || state[SDL_SCANCODE_RSHIFT]) dash = true;
    if (state[SDL_SCANCODE_E]) action = true;
    if (state[SDL_SCANCODE_LCTRL]) crouch = true;

    // ppl7::PrintDebugTime("keys: %4d, velocity x: %5d, velocity y: %5d --- ", k.matrix, k.velocity_x, k.velocity_y);

    if (game->controller.isOpen()) {
        GameController& gc = game->controller;

        int velocity_x = gc.getAxisState(gc.mapping.getSDLAxis(GameControllerMapping::Axis::Walk));
        int velocity_y = gc.getAxisState(gc.mapping.getSDLAxis(GameControllerMapping::Axis::UpDown));

        if (velocity_x > 1000) right = true;
        if (velocity_x < 1000) left = true;

        if (velocity_y > 10000) down = true;
        if (velocity_y < -10000) up = true;

        if (gc.getButtonState(gc.mapping.getSDLButton(GameControllerMapping::Button::Action))) action = true;
        if (gc.getButtonState(gc.mapping.getSDLButton(GameControllerMapping::Button::Crouch))) crouch = true;

        if (gc.getButtonState(gc.mapping.getSDLButton(GameControllerMapping::Button::Jump))) jump = true;
        if (gc.getButtonState(gc.mapping.getSDLButton(GameControllerMapping::Button::Dash))) dash = true;
    }
}