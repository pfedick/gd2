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
    this->time = time;
    left = false;
    right = false;
    up = false;
    down = false;
    action = false;
    block = false;
    dash = false;
    crouch = false;
    jump = false;
    const bool* state = SDL_GetKeyboardState(NULL);
    if (state[SDL_SCANCODE_LEFT] || state[SDL_SCANCODE_A]) left = true;
    if (state[SDL_SCANCODE_RIGHT] || state[SDL_SCANCODE_D]) right = true;
    if (state[SDL_SCANCODE_UP] || state[SDL_SCANCODE_W]) up = true;
    if (state[SDL_SCANCODE_DOWN] || state[SDL_SCANCODE_S]) down = true;
    if (state[SDL_SCANCODE_LSHIFT] || state[SDL_SCANCODE_RSHIFT]) dash = true;
    if (state[SDL_SCANCODE_E]) action = true;
    if (state[SDL_SCANCODE_LCTRL]) crouch = true;
    if (state[SDL_SCANCODE_SPACE]) jump = true;

    if (game->controller.isOpen()) {
        GameController& gc = game->controller;

        int velocity_x = gc.getAxisState(gc.mapping.getSDLAxis(GameControllerMapping::Axis::Walk));
        int velocity_y = gc.getAxisState(gc.mapping.getSDLAxis(GameControllerMapping::Axis::UpDown));

        if (velocity_x > 1000) right = true;
        if (velocity_x < -1000) left = true;

        if (velocity_y > 10000) down = true;
        if (velocity_y < -10000) up = true;

        if (gc.getButtonState(gc.mapping.getSDLButton(GameControllerMapping::Button::Action))) action = true;
        if (gc.getButtonState(gc.mapping.getSDLButton(GameControllerMapping::Button::Crouch))) crouch = true;

        if (gc.getButtonState(gc.mapping.getSDLButton(GameControllerMapping::Button::Jump))) jump = true;
        if (gc.getButtonState(gc.mapping.getSDLButton(GameControllerMapping::Button::Dash))) dash = true;
    }
    for (const auto& [key, timestamp] : key_timestamps) {
        if (timestamp > time - 0.2f) {
            if (key == PlayerKeys::Jump)
                jump = true;
            else if (key == PlayerKeys::Dash)
                dash = true;
        }
    }
}

void KeyState::queueKeyEvent(PlayerKeys key, double time)
{
    key_timestamps[key] = time;
}

bool KeyState::jumpLeft()
{
    return left && jump;
}

bool KeyState::jumpRight()
{
    return right && jump;
}

bool KeyState::jumpUp()
{
    return (!left && !right && jump);
}

bool KeyState::runLeft()
{
    return (left && !jump);
}

bool KeyState::runRight()
{
    return (right && !jump);
}