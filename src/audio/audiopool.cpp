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
    }
    catch (const ppl7::Exception& exp) {
        exp.print();
        throw;
    }
    loadEffects();
    loadLoops();
}

void AudioPool::loadEffects()
{
    try {
        sample[AudioClip::arrow_swoosh].load("res/audio/effect/arrow_swoosh.mp3");
        sample[AudioClip::bat].load("res/audio/effect/bat.mp3");
        sample[AudioClip::break1].load("res/audio/effect/break1.mp3");
        sample[AudioClip::bullet_hits_player].load("res/audio/effect/bullet_hits_player.mp3");
        sample[AudioClip::bullet_hits_wall].load("res/audio/effect/bullet_hits_wall.mp3");
        sample[AudioClip::stamper_down].load("res/audio/effect/stamper_down.mp3");
        sample[AudioClip::stamper_up].load("res/audio/effect/stamper_up.mp3");
        sample[AudioClip::stamper_echo].load("res/audio/effect/stamper_echo.mp3");
        sample[AudioClip::stamper_squish].load("res/audio/effect/stamper_squish.mp3");

        sample[AudioClip::coin1].load("res/audio/effect/coin1.mp3");
        sample[AudioClip::coin2].load("res/audio/effect/coin2.mp3");
        sample[AudioClip::coin3].load("res/audio/effect/coin3.mp3");
        sample[AudioClip::coin4].load("res/audio/effect/coin4.mp3");
        sample[AudioClip::coin5].load("res/audio/effect/coin5.mp3");
        sample[AudioClip::coin6].load("res/audio/effect/coin6.mp3");

        sample[AudioClip::crunch].load("res/audio/effect/crunch.mp3");

        sample[AudioClip::crystal1].load("res/audio/effect/crystal1.mp3");
        sample[AudioClip::crystal2].load("res/audio/effect/crystal2.mp3");
        sample[AudioClip::crystal3].load("res/audio/effect/crystal3.mp3");
        sample[AudioClip::explosion1].load("res/audio/effect/explosion1.mp3");
        sample[AudioClip::fabric].load("res/audio/effect/fabric.mp3");
        sample[AudioClip::fall].load("res/audio/effect/fall.mp3");
        sample[AudioClip::fanfare1].load("res/audio/effect/fanfare1.mp3");
        sample[AudioClip::fanfare2].load("res/audio/effect/fanfare2.mp3");
        sample[AudioClip::fanfare3].load("res/audio/effect/fanfare3.mp3");
        sample[AudioClip::fanfare4].load("res/audio/effect/fanfare4.mp3");
        sample[AudioClip::fireball_impact].load("res/audio/effect/fireball_impact.mp3");

        sample[AudioClip::impact1].load("res/audio/effect/impact1.mp3");

        sample[AudioClip::player_step1].load("res/audio/effect/player_step1.mp3");
        sample[AudioClip::player_step2].load("res/audio/effect/player_step2.mp3");
        sample[AudioClip::player_step3].load("res/audio/effect/player_step3.mp3");
        sample[AudioClip::player_step4].load("res/audio/effect/player_step4.mp3");
        sample[AudioClip::player_step5].load("res/audio/effect/player_step5.mp3");
        sample[AudioClip::player_jump_land].load("res/audio/effect/player_jump_land.mp3");
        sample[AudioClip::ladder_step1].load("res/audio/effect/ladder_step1.mp3");
        sample[AudioClip::ladder_step2].load("res/audio/effect/ladder_step2.mp3");
        sample[AudioClip::ladder_step3].load("res/audio/effect/ladder_step3.mp3");
        sample[AudioClip::ladder_step4].load("res/audio/effect/ladder_step4.mp3");
        sample[AudioClip::ladder_step5].load("res/audio/effect/ladder_step5.mp3");
        sample[AudioClip::ladder_step6].load("res/audio/effect/ladder_step6.mp3");
        sample[AudioClip::ladder_step7].load("res/audio/effect/ladder_step7.mp3");

        sample[AudioClip::squash1].load("res/audio/effect/squash1.mp3");
        sample[AudioClip::trap1].load("res/audio/effect/trap1.mp3");
        sample[AudioClip::trap2].load("res/audio/effect/trap2.mp3");
    }
    catch (const ppl7::Exception& exp) {
        exp.print();
        throw;
    }
}

void AudioPool::loadLoops()
{
    try {
        sample[AudioClip::birds_in_rain].load("res/audio/loop/birds_in_the_rain.ogg");
        sample[AudioClip::birds1].load("res/audio/loop/birds1.ogg");
        sample[AudioClip::birds2].load("res/audio/loop/birds2.ogg");
        sample[AudioClip::birds3].load("res/audio/loop/birds2.ogg");
        sample[AudioClip::cave1].load("res/audio/loop/cave1.ogg");
        sample[AudioClip::cave2].load("res/audio/loop/cave2.ogg");
        sample[AudioClip::cave3].load("res/audio/loop/cave3.ogg");
        sample[AudioClip::cave4].load("res/audio/loop/cave4.ogg");
        sample[AudioClip::crate_loop].load("res/audio/loop/crate_loop.ogg");
        sample[AudioClip::desert_at_night].load("res/audio/loop/desert_at_night.mp3");
        sample[AudioClip::earthquake].load("res/audio/loop/earthquake.ogg");
        sample[AudioClip::electric].load("res/audio/loop/electric.mp3");
        sample[AudioClip::fire1].load("res/audio/loop/fire1.ogg");
        sample[AudioClip::fire2].load("res/audio/loop/fire2.ogg");
        sample[AudioClip::fire3].load("res/audio/loop/fire3.ogg");
        sample[AudioClip::fire4].load("res/audio/loop/fire4.ogg");
        sample[AudioClip::fireball_fly].load("res/audio/loop/fireball_fly.ogg");
        sample[AudioClip::fireworks_loop].load("res/audio/loop/fireworks_loop.ogg");
        sample[AudioClip::gas1].load("res/audio/loop/gas1.ogg");
        sample[AudioClip::gas2].load("res/audio/loop/gas2.ogg");
        sample[AudioClip::gas3].load("res/audio/loop/gas3.ogg");
        sample[AudioClip::jungle1].load("res/audio/loop/jungle1.ogg");
        sample[AudioClip::jungle2].load("res/audio/loop/jungle2.ogg");
        sample[AudioClip::lavaloop1].load("res/audio/loop/lavaloop1.ogg");
        sample[AudioClip::lavaloop2].load("res/audio/loop/lavaloop2.ogg");
        sample[AudioClip::lavabubbles].load("res/audio/loop/lavabubbles.ogg");
        sample[AudioClip::night1].load("res/audio/loop/night1.mp3");
        sample[AudioClip::night2].load("res/audio/loop/night2.mp3");
        sample[AudioClip::night3].load("res/audio/loop/night3.mp3");
        sample[AudioClip::night4].load("res/audio/loop/night4.mp3");
        sample[AudioClip::nightowl1].load("res/audio/loop/nightowl1.mp3");
        sample[AudioClip::nightowl2].load("res/audio/loop/nightowl2.mp3");
        sample[AudioClip::nightowl3].load("res/audio/loop/nightowl3.mp3");
        sample[AudioClip::rain1].load("res/audio/loop/rain1.mp3");
        sample[AudioClip::rain2].load("res/audio/loop/rain2.mp3");
        sample[AudioClip::rain3].load("res/audio/loop/rain3.mp3");
        sample[AudioClip::rumble].load("res/audio/loop/rumble.ogg");
        sample[AudioClip::soft_rain].load("res/audio/loop/soft_rain.mp3");
        sample[AudioClip::thruster].load("res/audio/loop/thruster.ogg");
        sample[AudioClip::underwaterloop1].load("res/audio/loop/underwaterloop1.ogg");
        sample[AudioClip::water_bubble1].load("res/audio/loop/water_bubble1.mp3");
        sample[AudioClip::water_bubble2].load("res/audio/loop/water_bubble2.ogg");
        sample[AudioClip::water_bubble3].load("res/audio/loop/water_bubble3.ogg");
        sample[AudioClip::water_bubble4].load("res/audio/loop/water_bubble4.ogg");
        sample[AudioClip::water_bubble5].load("res/audio/loop/water_bubble5.ogg");
        sample[AudioClip::waterdrips1].load("res/audio/loop/waterdrips.mp3");
        sample[AudioClip::waterdrips2].load("res/audio/loop/waterdrips2.mp3");
        sample[AudioClip::waterdrips3].load("res/audio/loop/waterdrips3.mp3");
        sample[AudioClip::waterflow1].load("res/audio/loop/waterflow1.ogg");
        sample[AudioClip::waterflow2].load("res/audio/loop/waterflow2.ogg");
        sample[AudioClip::waterflow3].load("res/audio/loop/waterflow3.ogg");
        sample[AudioClip::waterpuddle].load("res/audio/loop/waterpuddle.ogg");
        sample[AudioClip::waves1].load("res/audio/loop/waves1.ogg");
        sample[AudioClip::waves2].load("res/audio/loop/waves2.ogg");
        sample[AudioClip::waves3].load("res/audio/loop/waves3.ogg");
        sample[AudioClip::waves4].load("res/audio/loop/waves4.ogg");
        sample[AudioClip::wind_grillen].load("res/audio/loop/wind_grillen.ogg");
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
