#ifndef INCLUDE_AUDIOPOOL_H_
#define INCLUDE_AUDIOPOOL_H_
#include "stdlib.h"
#include "stdio.h"
#include <ppl7.h>
#include <ppl7-grafix.h>
#include <ppl7-audio.h>

#include "audio.h"
#include <map>

enum class AudioEffect : uint32_t
{
    // Achtung: ID darf nicht > 65535 sein, da sie in Game-Objecten in einem uint16_t gespeichert wird
    none = 0,
    coin1 = 1,
    coin2,
    coin3,
    coin4,
    coin5,
    coin6,
    player_step1,
    player_step2,
    player_step3,
    player_step4,
    player_step5,
    player_jump_land,
    ladder_step1,
    ladder_step2,
    ladder_step3,
    ladder_step4,
    ladder_step5,
    ladder_step6,
    ladder_step7,
    watersplash1,
    watersplash2,
    watersplash3,
    watersplash4,
    water_pouring1,
    water_pouring2,
    impact1,
    crystal1,
    crystal2,
    crystal3,
    trap1,
    trap2,
    arrow_swoosh,
    arrow_hit_wall,
    bat,
    break1,
    bullet_hits_player,
    bullet_hits_wall,
    stamper_down,
    stamper_up,
    stamper_echo,
    stamper_squish,
    crunch,
    explosion1,
    fabric,
    fall,
    fanfare1,
    fanfare2,
    fanfare3,
    fanfare4,
    fireball_impact,
    squash1,
};

enum class AudioLoop : uint32_t
{
    none = 0,
    birds1 = 100000,
    birds2,
    birds3,
    birds_in_rain,
    cave1,
    cave2,
    cave3,
    cave4,
    desert_at_night,
    electric,
    fire1,
    fire2,
    fire3,
    fire4,
    fireworks_loop,
    fireball_fly,
    jungle1,
    jungle2,
    lavaloop1,
    lavaloop2,
    lavabubbles,
    night1,
    night2,
    night3,
    night4,
    nightowl1,
    nightowl2,
    nightowl3,
    rain1,
    rain2,
    rain3,
    soft_rain,
    waterflow1,
    waterflow2,
    waterflow3,
    waterdrips1,
    waterdrips2,
    waterdrips3,
    water_bubble1,
    water_bubble2,
    water_bubble3,
    water_bubble4,
    water_bubble5,
    underwaterloop1,
    waves1,
    waves2,
    waves3,
    waves4,
    gas1,
    gas2,
    gas3,
    wind1,
    wind2,
    wind3,
    wind4,
    wind_strong,
    earthquake,
    rumble,
    waterpuddle,
    crate_loop,
    thruster,
    wind_crickets,
};

enum class SpeechClip : uint32_t
{
    none = 0,
    aua1 = 200000,
};

struct AudioClipId
{
    uint32_t id;

    constexpr AudioClipId()
        : id(0)
    {
    }

    // Konstruktoren erlauben die implizite Umwandlung von den Enums in diesen Typ
    constexpr AudioClipId(AudioEffect effect)
        : id(static_cast<uint32_t>(effect))
    {
    }
    constexpr AudioClipId(AudioLoop loop)
        : id(static_cast<uint32_t>(loop))
    {
    }
    constexpr AudioClipId(SpeechClip speech)
        : id(static_cast<uint32_t>(speech))
    {
    }

    // Erlaubt die Verwendung als Key in der Map oder für Vergleiche
    constexpr operator uint32_t() const
    {
        return id;
    }
    auto operator<=>(const AudioClipId&) const = default;
};

class MusicTrack
{
public:
    ppl7::String Name;
    ppl7::String Filename;

    MusicTrack(const ppl7::String& Name, const ppl7::String& Filename)
    {
        this->Name = Name;
        this->Filename = Filename;
    }
};

class AudioPool
{
private:
    AudioSystem* audio;

    void loadClip(AudioClipId id, const ppl7::String& filename);

    std::map<uint32_t, AudioSample> samples;

public:
    std::list<MusicTrack> musictracks;

    AudioPool();
    ~AudioPool();
    void load();
    void load_speech(const ppl7::String& lang);
    void loadLoops();
    void loadEffects();
    void setAudioSystem(AudioSystem* audio);
    size_t size() const;
    AudioInstance* getInstance(AudioClipId id, AudioClass a = AudioClass::Effect);
    void playOnce(AudioClipId id, float volume = 1.0f, AudioClass a = AudioClass::Effect);
    void playOnce(
        AudioClipId id, const ppl7::grafix::Point& p, int max_distance = 1600, float volume = 1.0f, AudioClass a = AudioClass::Effect);
    void playInstance(AudioInstance* instance);
    void stopInstace(AudioInstance* instance);
};

AudioPool& getAudioPool();

#endif // INCLUDE_AUDIOPOOL_H_
