#ifndef INCLUDE_PHYSIC_H_
#define INCLUDE_PHYSIC_H_

#include "tiletypes.h"
#include "game.h"
#include "collision.h"

class TileTypePlane;

class Vector2
{
public:
    float x = 0.0f;
    float y = 0.0f;

    Vector2(float x = 0.0f, float y = 0.0f)
    {
        this->x = x;
        this->y = y;
    }
    void clear()
    {
        x = 0.0f;
        y = 0.0f;
    }
};

class KeyboardKeys
{
public:
    enum
    {
        Left = 1,
        Right = 2,
        Up = 4,
        Down = 8,
        Shift = 16,
        Action = 32,
        Flashlight = 64,
        Crouch = 128,
        JumpLeft = 5,
        JumpRight = 6
    };
};

class Physic
{
public:
    enum PlayerMovement
    {
        Unchanged = 0,
        Stand,
        Turn,
        Run,
        Pickup,
        ClimbUp,
        ClimbDown,
        Jump,
        Falling,
        Slide,
        Floating,
        Dead,
        Swim,
        SwimStraight,
        SwimUp,
        SwimDown,
        Hacking,
        Crouch,
        Crawling,
        CrawlTurn,
        Petrified
    };
    enum PlayerOrientation
    {
        Left,
        Right,
        Front,
        Back
    };

    enum HealthDropReason
    {
        Unknown,
        FallingDeep,
        Smashed,
        Drowned,
        Burned,
        SmashedSideways,
        Etched
    };

    PlayerMovement movement = Stand;
    PlayerOrientation orientation = Front;
    PlayerOrientation turnTarget;

    Vector2 velocity_move; // current velocity of object
    Vector2 wind;
    float gravity;
    float friction;
    float max_run_speed;
    float max_slide_speed;
    float max_falling_speed;
    float coyote_time;

    double time;
    double fallstart_time;
    double last_grounded_time;

    void* player_stands_on_object;

    bool isEnemy;

    Physic();

    bool updatePhysics(const GameClock& clock, WorldCollision& collision); // returns true if movement has changed
    PlayerMovement checkCollisionWithWorld(WorldCollision& collision,
                                           float& x,
                                           float& y); // returns new movement if movement has changed, otherwise Unchanged

    void setWind(float strength, float direction);

    ppl7::String getState() const;
};

#endif // INCLUDE_PHYSIC_H_
