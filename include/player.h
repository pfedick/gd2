#ifndef INCLUDE_PLAYER_H_
#define INCLUDE_PLAYER_H_
#include <ppl7-grafix.h>

class SpriteTexture;
#include "gpu.h"
#include "gamerenderer.h"
#include "animation.h"
#include "physic.h"
#include "audio.h"
#include "audiopool.h"
#include "level.h"
#include "collision.h"
#include <map>

class TileTypePlane;
class Game;
class GameClock;
namespace Objects
{
class Object;
class Representation;
} // namespace Objects
class ObjectSystem;

enum class PlayerKeys
{
    Left = 1,
    Right,
    Up,
    Down,
    Action,
    Crouch,
    Jump,
    Dash,
    Block,
    Inventory,
    Map,
    Light
};

class KeyState
{
private:
    std::map<PlayerKeys, double> key_timestamps;
    Game* game;
    double time;

public:
    KeyState(Game* game);
    bool left = false;
    bool right = false;
    bool up = false;
    bool down = false;
    bool action = false;
    bool block = false;
    bool dash = false;
    bool crouch = false;
    bool jump = false;
    bool light = false;
    bool inventory = false;
    bool map = false;

    void update(double time);
    void queueKeyEvent(PlayerKeys key, double time); // TODO: mapping von SDL-Events zu PlayerKeys in GameController

    bool jumpLeft();
    bool jumpRight();
    bool jumpUp();
    bool runLeft();
    bool runRight();
};

class Player : public Physic
{
private:
    const SpriteTexture* sprite_resource;
    const SpriteTexture* tiletype_resource;

    double idle_timeout;
    double last_aircheck;
    Game* game;

    AnimationCycle animation;
    ppl7::grafix::Color color_modulation;

    ppl7::grafix::Point lastSavePoint;

    std::map<int, Objects::Representation> Inventory;
    std::map<int, size_t> object_counter;
    std::set<int> SpecialObjects;
    bool godmode;
    bool dead;
    bool visible;
    double airStart;
    double startIdle;
    float frame_rate_compensation;
    bool controlEnabled;
    int last_animation_sound_played;

    AudioInstance* ambient_sound;
    AudioLoop ambient_playing;
    // LightObject flashlight1, flashlight2, flashlight3, flashlight2_ladder;

    enum class ParticleReason
    {
        None = 0,
        Drowned,
        Burning,
        Smashed,
        SmashedSideways
    };

    std::list<ppl7::grafix::Point> collision_checkpoints;
    WorldCollision world_collision;

    ParallaxLayerId currentLayer;
    float parallax_scale;

    void turn(PlayerOrientation target);
    void checkCollisionWithObjects(const GameClock& clock, ObjectSystem& objects);
    bool updatePhysics(const GameClock& clock);
    Physic::PlayerMovement checkCollisionWithWorld(const GameClock& clock, const TileTypePlane& world);

    void handleKeyboardWhileJumpOrFalling(const GameClock& clock, const TileTypePlane& world, ObjectSystem& objects);
    void handleKeyboard(const GameClock& clock, const TileTypePlane& world, ObjectSystem& objects);

    void playSoundOnAnimationSprite();
    void checkActivationOfObjectsInRange(ObjectSystem& objectsystem);

public:
    float x, y;
    int points, lifes;
    float health;
    float scale;

    // is updated every frame
    ppl7::grafix::PointF WorldCoords;
    GameViewport Viewport;
    KeyState keys;

    explicit Player(Game* game);
    ~Player();
    ppl7::grafix::PointF position() const;
    void setParallaxLayer(ParallaxLayerId layer, float parallax_scale = 1.0f);
    ParallaxLayerId getParallaxLayer() const;
    void stand();

    void resetState();
    void resetLevelObjects();
    void setZeroVelocity();
    void setVisible(bool flag);
    void addPoints(int points);
    void addHealth(int points);
    void addLife(int lifes);
    void addSpecialObject(int type);
    bool hasSpecialObject(int type) const;
    void countObject(Objects::Type type);
    size_t getObjectCount(int type) const;
    void dropHealth(float points, HealthDropReason reason = HealthDropReason::Unknown);
    void addInventory(int object_id, const Objects::Representation& repr);
    bool isInInventory(int object_id) const;
    bool isDead() const;
    void dropLifeAndResetToLastSavePoint();
    void setGodMode(bool enabled);
    bool godModeEnabled() const;
    void setSavePoint(const ppl7::grafix::Point& p);
    void setSpriteResource(const SpriteTexture& resource);
    void setTileTypeResource(const SpriteTexture& resource);
    void draw(GameRenderer& renderer, const GameViewport& viewport, const ppl7::grafix::Point& worldcoords, float size) const;
    void drawCollision(GameRenderer& renderer, const GameViewport& viewport, const ppl7::grafix::Point& worldcoords) const;
    void move(int x, int y);
    ppl7::grafix::Rect getBoundingBox() const;
    void setStandingOnObject(Objects::Object* object);
    void update(const GameClock& clock, ParallaxLayer& layer);

    void takeAllItems(Objects::Type type);
    void enableControl();
    void disableControl();

    const std::list<ppl7::grafix::Point>& getCollisionCheckpoints() const;
};
#endif /* INCLUDE_PLAYER_H_ */
