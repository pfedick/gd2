#ifndef INCLUDE_OBJECTS_GENERIC_H_
#define INCLUDE_OBJECTS_GENERIC_H_

#include "objectsystem.h"
#include "particle.h"
#include <ppltk.h>

namespace Objects
{

class PlayerStartPoint : public Object
{
private:
public:
    PlayerStartPoint();
    static Representation representation();
};

class Coin : public Object
{
private:
    AnimationCycle animation;

public:
    Coin();
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

class ParticleEmitter : public Object
{
private:
    double next_birth;

    void createParticle(ParticleSystem* ps, double time);

public:
    enum class Flags
    {
        useColorGradient = 1,
        useScaleGradient = 2,
        initialStateDisabled = 4,
    };

    Particle::Type particle_type;
    EmitterType emitter_type;
    ppl7::grafix::Color ParticleColor;
    Particle::Layer particle_layer;
    ppl7::grafix::Size emitter_size;
    int flags;
    int min_birth_per_cycle, max_birth_per_cycle;
    float birth_time_min, birth_time_max;
    float min_velocity;
    float max_velocity;
    float scale_min, scale_max;
    float age_min, age_max;
    float direction, variation;
    float weight_min, weight_max;
    ppl7::grafix::PointF gravity;
    std::list<Particle::ScaleGradientItem> scale_gradient;
    std::list<Particle::ColorGradientItem> color_gradient;
    bool current_state;

    ParticleEmitter();
    static Representation representation();
    void update(const GameClock& clock, TileTypePlane& ttplane, Player& player) override;

    size_t save(unsigned char* buffer, size_t size) const override;
    size_t saveSize() const override;
    size_t load(const unsigned char* buffer, size_t size) override;
    void openUi() override;
    void toggle(bool enable, Object* source = NULL) override;
    void trigger(Object* source = NULL);

    ppl7::String generateCode() const;
};

class SpawnPoint : public Object
{
private:
    unsigned char toggle_count;
    double next_touch_time;

public:
    int sample_id;
    int max_distance;
    float volume;
    unsigned char max_toggles;
    Type emitted_object;

    SpawnPoint();
    ~SpawnPoint();
    static Representation representation();
    void init();
    void emmitObject();
    // void handleCollision(Player* player, const Collision& collision) override;
    // void update(double time, TileTypePlane& ttplane, Player& player, float frame_rate_compensation) override;
    void trigger(Object* source = NULL);
    size_t save(unsigned char* buffer, size_t size) const override;
    size_t saveSize() const override;
    size_t load(const unsigned char* buffer, size_t size) override;
    void openUi() override;
    void reset() override;
};

class Speaker : public Object
{
private:
    AudioInstance* audio;

public:
    int sample_id;
    int max_distance;
    float volume;

    bool initial_state;

    enum class SampleType
    {
        AudioLoop = 0,
        Effect
    };
    SampleType sample_type;

    static void fillComboBoxWithEffects(ppltk::ComboBox* combobox, int selected_sample);

    Speaker();
    ~Speaker();
    static Representation representation();
    void setSample(int id, float volume, int max_distance = 1600);
    void update(const GameClock& clock, TileTypePlane& ttplane, Player& player) override;
    size_t save(unsigned char* buffer, size_t size) const override;
    size_t saveSize() const override;
    size_t load(const unsigned char* buffer, size_t size) override;
    void openUi() override;
    void toggle(bool enable, Object* source = NULL) override;
    void trigger(Object* source = NULL) override;
    void test();
};

} // namespace Objects

#endif // INCLUDE_OBJECTS_GENERIC_H_