#ifndef INCLUDE_AUDIOPOOL_H_
#define INCLUDE_AUDIOPOOL_H_
#include "stdlib.h"
#include "stdio.h"
#include <ppl7.h>
#include <ppl7-grafix.h>
#include <ppl7-audio.h>

#include "audio.h"

class AudioClip
{
public:
    // DO NOT CHANGE THE ORDER OF THE IDs!!!
    // They are stored in the level files
    enum Id
    {
        none = 0,
        coin1 = 1,
        coin2,
        stamper_squish,
        underwaterloop1,
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
        powercell_change,
        powercells_depleted,

        // this must always be the last entry!
        maxClips
    };
};

class SpeechClip
{
public:
    enum Id
    {
        none = 0,
        aua1,

        maxClips
    };
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

public:
    AudioSample sample[AudioClip::Id::maxClips + 1];
    AudioSample voice_george[SpeechClip::maxClips + 1];

    std::list<MusicTrack> musictracks;

    AudioPool();
    ~AudioPool();
    void load();
    void load_speech(const ppl7::String& lang);
    void setAudioSystem(AudioSystem* audio);
    size_t size() const;
    AudioInstance* getInstance(AudioClip::Id id, AudioClass a = AudioClass::Effect);
    void playOnce(AudioClip::Id id, float volume = 1.0f, AudioClass a = AudioClass::Effect);
    void playOnce(
        AudioClip::Id id, const ppl7::grafix::Point& p, int max_distance = 1600, float volume = 1.0f, AudioClass a = AudioClass::Effect);
    void playInstance(AudioInstance* instance);
    void stopInstace(AudioInstance* instance);
};

AudioPool& getAudioPool();

#endif // INCLUDE_AUDIOPOOL_H_
