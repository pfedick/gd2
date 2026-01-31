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

        sample[AudioClip::coin1].load("res/audio/coin1.mp3");
        sample[AudioClip::coin2].load("res/audio/coin2.mp3");
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
