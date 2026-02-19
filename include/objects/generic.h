#ifndef INCLUDE_OBJECTS_GENERIC_H_
#define INCLUDE_OBJECTS_GENERIC_H_

#include "objectsystem.h"

namespace Objects
{

class PlayerStartPoint : public Object
{
private:
public:
    PlayerStartPoint();
    static Representation representation();
};

class CoinReward : public Object
{
private:
    AnimationCycle animation;

public:
    CoinReward();
    static Representation representation();

    void update(const GameClock& clock, TileTypePlane& ttplane, Player& player) override;
    void handleCollision(Player* player, const Collision& collision) override;
};

class ExtraLife : public Object
{
private:
    AnimationCycle animation;

public:
    ExtraLife();
    static Representation representation();

    void update(const GameClock& clock, TileTypePlane& ttplane, Player& player) override;
    void handleCollision(Player* player, const Collision& collision) override;
};

class CrystalReward : public Object
{
private:
    AnimationCycle animation;
    // LightObject light_glow;

public:
    CrystalReward();
    static Representation representation();
    void update(const GameClock& clock, TileTypePlane& ttplane, Player& player) override;
    void handleCollision(Player* player, const Collision& collision) override;
};

class Medikit : public Object
{
private:
public:
    Medikit();
    static Representation representation();
    void handleCollision(Player* player, const Collision& collision) override;
};

class SavePoint : public Object
{
private:
    double next_animation;
    AnimationCycle animation;

public:
    SavePoint();
    static Representation representation();
    void update(const GameClock& clock, TileTypePlane& ttplane, Player& player) override;
    void handleCollision(Player* player, const Collision& collision) override;
};

} // namespace Objects

#endif // INCLUDE_OBJECTS_GENERIC_H_