#ifndef INCLUDE_PLAYER_H_
#define INCLUDE_PLAYER_H_
#include <ppl7-grafix.h>

class SpriteTexture;
#include "gpu.h"
#include "animation.h"
#include "physic.h"
#include "audio.h"
#include "audiopool.h"
#include "level.h"
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

    double next_keycheck;
    double last_fullspeed;
    double idle_timeout;
    double last_aircheck;
    Game* game;

    AnimationCycle animation;
    ppl7::grafix::Color color_modulation;

    ppl7::grafix::Point lastSavePoint;

    std::map<int, Objects::Representation> Inventory;
    std::map<int, size_t> object_counter;
    std::set<int> SpecialObjects;
    std::set<uint16_t> spokenText;
    bool godmode;
    bool dead;
    bool visible;
    bool autoWalk;
    bool waterSplashPlayed;
    bool expressionJump;
    bool talkie;
    double airStart;
    double voiceDamageCooldown;
    double startIdle;
    double nextIdleSpeech;
    double nextPhonetic;
    float frame_rate_compensation;
    float battery_drain_rate;
    bool greetingPlayed;
    bool flashlightOn;
    bool petrified;
    bool controlEnabled;
    double actionToggleCooldown;
    double petrifiedTimeout;
    int last_animation_sound_played;

    AudioInstance* ambient_sound;
    AudioInstance* voice;
    AudioSample voice_sample;
    AudioClip::Id ambient_playing;
    // LightObject flashlight1, flashlight2, flashlight3, flashlight2_ladder;

    class AutoWalk
    {
    private:
        Player* player;
        ppl7::grafix::PointF target;
        bool isEnabled;
        bool use_waynet;

    public:
        AutoWalk();
        void setPlayer(Player* player);
        bool enabled() const;
        void getKeyboardMatrix(Player::Keys& keys, const ppl7::grafix::PointF& player_p);
        void setTarget(const ppl7::grafix::PointF& p, bool use_waynet = false);
        void stop();
    };
    AutoWalk player_autowalk;
    enum class ParticleReason
    {
        None = 0,
        Drowned,
        Burning,
        Smashed,
        SmashedSideways
    };
    double particle_end_time, next_particle_birth;
    ParticleReason particle_reason;

    Objects::Object* hackingObject;
    int hackingState;
    double hacking_end;
    ppl7::String phonetics;
    std::list<ppl7::grafix::Point> collision_checkpoints;

    ParallaxLayerId currentLayer;

    class FlashLightPivot
    {
    public:
        FlashLightPivot(int x, int y, float angle);
        int x;
        int y;
        float angle;
    };
    std::map<int, FlashLightPivot> flashlight_pivots;
    void initFlashLightPivots();

    void turn(PlayerOrientation target);
    void crawlTurn(PlayerOrientation target);
    void splashIntoWater(float gravity);
    void emmitParticles(double time);

    void moveOutOfWater(float angel, float speed);
    void checkCollisionWithObjects(ObjectSystem* objects, float frame_rate_compensation);
    void checkCollisionWithWorld(const TileTypePlane& world);

    void handleDiving(double time, const TileTypePlane& world, ObjectSystem* objects, float frame_rate_compensation);
    bool hackingInProgress();

    void handleKeyboardWhileJumpOrFalling(double time, const TileTypePlane& world, ObjectSystem* objects, float frame_rate_compensation);
    void handleKeyboardWhileSwimming(double time, const TileTypePlane& world, ObjectSystem* objects, float frame_rate_compensation);
    void handleKeyboardWhileCrawling(double time, const TileTypePlane& world, ObjectSystem* objects, float frame_rate_compensation);

    void playSoundOnAnimationSprite();
    void checkActivationOfObjectsInRange(ObjectSystem* objectsystem);
    void toggleFlashlight();
    void idleJokes(double time);
    void playPhonetics();
    void initFlashLight();
    void drainBattery();

public:
    float x, y;
    int points, lifes;
    int powercells;
    float health;
    float air;
    float maxair;
    float energylevel;
    float scale;

    // is updated every frame
    ppl7::grafix::PointF WorldCoords;
    GameViewport Viewport;
    KeyState keys;

    explicit Player(Game* game);
    ~Player();
    ppl7::grafix::PointF position() const;
    void setParallaxLayer(ParallaxLayerId layer);
    ParallaxLayerId getParallaxLayer() const;
    void stand();
    void jumpExpression();
    void resetState();
    void resetLevelObjects();
    void setZeroVelocity();
    void setVisible(bool flag);
    void enableTalkie(bool flag);
    void addPoints(int points);
    void addHealth(int points);
    void addAir(float seconds);
    void addLife(int lifes);
    void addSpecialObject(int type);
    bool hasSpecialObject(int type) const;
    void countObject(int type);
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
    void draw(GPUBatcher& batcher, const GameViewport& viewport, const ppl7::grafix::Point& worldcoords, float size) const;
    // void addFlashlightToLightSystem(LightSystem& lights);
    void drawCollision(GPUBatcher& batcher, const GameViewport& viewport, const ppl7::grafix::Point& worldcoords) const;
    void move(int x, int y);
    ppl7::grafix::Rect getBoundingBox() const;
    void setStandingOnObject(Objects::Object* object);
    void setAutoWalk(bool enabled);
    bool isAutoWalk() const;
    void startEmittingParticles(double endtime, ParticleReason reason);
    void startHacking(Objects::Object* object);
    void update(const GameClock& clock, ParallaxLayer& layer);
    void update(const GameClock& clock, const TileTypePlane& world, ObjectSystem* objects);

    /*
    void speak(VoiceGeorge::Id id,
               float volume = 0.7f,
               const ppl7::String& text = ppl7::String(),
               const ppl7::String& phonetics = ppl7::String());
    bool speak(uint16_t id, float volume = 0.7f);
    bool isSpeaking() const;
    bool hasSpoken(uint16_t id) const;
    */
    bool isFlashlightOn() const;
    void enableFlashlight(bool enable);
    void hitBySpiderWeb();
    void setPetrified(bool petrified, float timeout = 86400.0f);
    bool isPetrified() const;
    void addPowerCell();
    void setBatteryDrainRate(float rate);
    void takeAllItems(int type);
    void drainBatteryCompletely();
    void enableControl();
    void disableControl();
    void walkToNode(const ppl7::grafix::PointF& target, bool useWaynet = false);
    void stop();

    const std::list<ppl7::grafix::Point>& getCollisionCheckpoints() const;
};
#endif /* INCLUDE_PLAYER_H_ */
