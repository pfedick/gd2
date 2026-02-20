#include "audiopool.h"

static AudioPool* audiopool = NULL;

AudioPool& getAudioPool()
{
    if (!audiopool) throw AudioException("Audiopool is not initialized");
    return *audiopool;
}

AudioPool::AudioPool()
{
    if (!audiopool) audiopool = this;
    audio = NULL;
}

AudioPool::~AudioPool()
{
    if (audiopool == this) audiopool = NULL;
}

size_t AudioPool::size() const
{
    size_t s = 0;
    for (int i = 1; i < AudioClip::maxClips; i++)
        s += sample[i].bufferSize();
    return s;
}

void AudioPool::setAudioSystem(AudioSystem* audio)
{
    this->audio = audio;
}

void AudioPool::playOnce(AudioClip::Id id, float volume, AudioClass a)
{
    AudioInstance* instance = new AudioInstance(sample[id], a);
    instance->setVolume(volume);
    instance->setAutoDelete(true);
    audio->play(instance);
}

void AudioPool::playOnce(AudioClip::Id id, const ppl7::grafix::Point& p, int max_distance, float volume, AudioClass a)
{
    if (id < AudioClip::maxClips) {
        AudioInstance* instance = new AudioInstance(sample[id], a);
        instance->setVolume(volume);
        instance->setAutoDelete(true);
        instance->setPositional(p, max_distance);
        audio->play(instance);
    }
}

AudioInstance* AudioPool::getInstance(AudioClip::Id id, AudioClass a)
{
    if (id < AudioClip::maxClips) return new AudioInstance(sample[id], a);
    return NULL;
}

void AudioPool::playInstance(AudioInstance* instance)
{
    audio->play(instance);
}

void AudioPool::stopInstace(AudioInstance* instance)
{
    audio->stop(instance);
}

void AudioPool::load()
{
    try {
        musictracks.push_back(
            MusicTrack("Patrick F. - In The Hall Of The Mountain King", "res/audio/PatrickF-In_The_Hall_Of_The_Mountain_King.mp3"));
        musictracks.push_back(MusicTrack("Patrick F. - ID", "res/audio/PatrickF-ID.mp3"));
        musictracks.push_back(MusicTrack("Patrick F. - Sonic Waves", "res/audio/PatrickF-Sonic_Waves.mp3"));
        musictracks.push_back(MusicTrack("Patrick F. - George Decker Theme", "res/audio/PatrickF-George_Decker_Theme.mp3"));
        musictracks.push_back(MusicTrack("Patrick F. - Heaven", "res/audio/PatrickF-Heaven.mp3"));
        musictracks.push_back(MusicTrack("Patrick F. - Spring", "res/audio/PatrickF-Spring.mp3"));

        sample[AudioClip::coin1].load("res/audio/effect/coin1.mp3");
        sample[AudioClip::coin2].load("res/audio/effect/coin2.mp3");
        sample[AudioClip::coin3].load("res/audio/effect/coin3.mp3");
        sample[AudioClip::coin4].load("res/audio/effect/coin4.mp3");
        sample[AudioClip::coin5].load("res/audio/effect/coin5.mp3");
        sample[AudioClip::coin6].load("res/audio/effect/coin6.mp3");
        sample[AudioClip::crystal1].load("res/audio/effect/crystal1.mp3");
        sample[AudioClip::crystal2].load("res/audio/effect/crystal2.mp3");
        sample[AudioClip::crystal3].load("res/audio/effect/crystal3.mp3");

        sample[AudioClip::wind1].load("res/audio/loop/wind1.ogg");
        sample[AudioClip::wind2].load("res/audio/loop/wind2.ogg");
        sample[AudioClip::wind3].load("res/audio/loop/wind3.ogg");
        sample[AudioClip::wind4].load("res/audio/loop/wind4.ogg");
        sample[AudioClip::wind_strong].load("res/audio/loop/wind_strong.ogg");
    }
    catch (const ppl7::Exception& exp) {
        exp.print();
        throw;
    }
}

void AudioPool::load_speech(const ppl7::String& lang)
{

    try {
        // voice_george[SpeechClip::aua1].load("res/audio/george/common/aua1.mp3");

        if (lang == "en") {
        }
    }
    catch (const ppl7::Exception& exp) {
        exp.print();
        throw;
    }
}
