#include "tiletypes.h"
#include "physic.h"
#include "objectsystem.h"
#include "constants.h"

static const char* movement_string[Physic::MaxMovementId + 1] = {
    "Unchanged", "Stand",  "Turn",     "Run",       "Pickup",    "ClimbUp",      "ClimbDown", "Jump",
    "Falling",   "Slide",  "Floating", "Dead",      "Swim",      "SwimStraight", "SwimUp",    "SwimDown",
    "Hacking",   "Crouch", "Crawling", "CrawlTurn", "Petrified", "JumpStart",    "JumpUp"};
static const char* orientation_string[4] = {"Left", "Right", "Front", "Back"};

Physic::Physic()
{
    movement = Stand;
    orientation = Front;
    turnTarget = Front;
    gravity = 9.81f;
    friction = 0.5f;
    max_run_speed = 16.0f;
    max_slide_speed = 8.0f;
    max_falling_speed = 50.0f;
    max_jump_speed = 30.0f;
    max_jump_time = 0.5f;
    min_jump_time = 0.05f;
    coyote_time = 1.3f;
    time = 0.0f;
    fallstart_time = 0.0f;
    last_grounded_time = 0.0f;
    jump_climax_time = 0.0f;
    jump_min_time = 0.0f;
    player_stands_on_object = nullptr;
    isEnemy = false;
}

ppl7::String Physic::getState() const
{
    ppl7::String s;
    s.setf("%s:%s, velocity: %0.3f:%0.3f", movement_string[movement], orientation_string[orientation], velocity_move.x, velocity_move.y);
    return s;
}

bool Physic::updatePhysics(const GameClock& clock, WorldCollision& collision)
{

    return false;
}

Physic::PlayerMovement Physic::checkCollisionWithWorld(WorldCollision& collision, float& x, float& y)
{
    return Unchanged;
}
