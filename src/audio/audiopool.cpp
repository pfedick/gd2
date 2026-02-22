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

void AudioPool::loadClip(AudioClipId id, const ppl7::String& filename)
{
    AudioSample sample;
    sample.load(filename);
    samples[id] = sample;
}

size_t AudioPool::size() const
{
    size_t s = 0;
    for (const auto& pair : samples) {
        s += pair.second.bufferSize();
    }
    return s;
}

void AudioPool::setAudioSystem(AudioSystem* audio)
{
    this->audio = audio;
}

void AudioPool::playOnce(AudioClipId id, float volume, AudioClass a)
{
    auto it = samples.find(id);
    if (it == samples.end()) {
        ppl7::PrintDebug("WARNING: AudioPool::playOnce - AudioClipId %u not found in AudioPool\n", static_cast<uint32_t>(id));
        return;
    }
    AudioInstance* instance = new AudioInstance(it->second, a);
    instance->setVolume(volume);
    instance->setAutoDelete(true);
    audio->play(instance);
}

void AudioPool::playOnce(AudioClipId id, const ppl7::grafix::Point& p, int max_distance, float volume, AudioClass a)
{
    auto it = samples.find(id);
    if (it == samples.end()) {
        ppl7::PrintDebug("WARNING: AudioPool::playOnce - AudioClipId %u not found in AudioPool\n", static_cast<uint32_t>(id));
        return;
    }
    AudioInstance* instance = new AudioInstance(it->second, a);
    instance->setVolume(volume);
    instance->setAutoDelete(true);
    instance->setPositional(p, max_distance);
    audio->play(instance);
}

AudioInstance* AudioPool::getInstance(AudioClipId id, AudioClass a)
{
    auto it = samples.find(id);
    if (it == samples.end()) {
        ppl7::PrintDebug("WARNING: AudioPool::playOnce - AudioClipId %u not found in AudioPool\n", static_cast<uint32_t>(id));
        return NULL;
    }
    return new AudioInstance(it->second, a);
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
        loadClip(AudioEffect::arrow_swoosh, "res/audio/effect/arrow_swoosh.mp3");
        loadClip(AudioEffect::bat, "res/audio/effect/bat.mp3");
        loadClip(AudioEffect::break1, "res/audio/effect/break1.mp3");
        loadClip(AudioEffect::bullet_hits_player, "res/audio/effect/bullet_hits_player.mp3");
        loadClip(AudioEffect::bullet_hits_wall, "res/audio/effect/bullet_hits_wall.mp3");
        loadClip(AudioEffect::stamper_down, "res/audio/effect/stamper_down.mp3");
        loadClip(AudioEffect::stamper_up, "res/audio/effect/stamper_up.mp3");
        loadClip(AudioEffect::stamper_echo, "res/audio/effect/stamper_echo.mp3");
        loadClip(AudioEffect::stamper_squish, "res/audio/effect/stamper_squish.mp3");
        loadClip(AudioEffect::coin1, "res/audio/effect/coin1.mp3");
        loadClip(AudioEffect::coin2, "res/audio/effect/coin2.mp3");
        loadClip(AudioEffect::coin3, "res/audio/effect/coin3.mp3");
        loadClip(AudioEffect::coin4, "res/audio/effect/coin4.mp3");
        loadClip(AudioEffect::coin5, "res/audio/effect/coin5.mp3");
        loadClip(AudioEffect::coin6, "res/audio/effect/coin6.mp3");
        loadClip(AudioEffect::crunch, "res/audio/effect/crunch.mp3");
        loadClip(AudioEffect::crystal1, "res/audio/effect/crystal1.mp3");
        loadClip(AudioEffect::crystal2, "res/audio/effect/crystal2.mp3");
        loadClip(AudioEffect::crystal3, "res/audio/effect/crystal3.mp3");
        loadClip(AudioEffect::explosion1, "res/audio/effect/explosion1.mp3");
        loadClip(AudioEffect::fabric, "res/audio/effect/fabric.mp3");
        loadClip(AudioEffect::fall, "res/audio/effect/fall.mp3");
        loadClip(AudioEffect::fanfare1, "res/audio/effect/fanfare1.mp3");
        loadClip(AudioEffect::fanfare2, "res/audio/effect/fanfare2.mp3");
        loadClip(AudioEffect::fanfare3, "res/audio/effect/fanfare3.mp3");
        loadClip(AudioEffect::fanfare4, "res/audio/effect/fanfare4.mp3");
        loadClip(AudioEffect::fireball_impact, "res/audio/effect/fireball_impact.mp3");
        loadClip(AudioEffect::impact1, "res/audio/effect/impact1.mp3");
        loadClip(AudioEffect::player_step1, "res/audio/effect/player_step1.mp3");
        loadClip(AudioEffect::player_step2, "res/audio/effect/player_step2.mp3");
        loadClip(AudioEffect::player_step3, "res/audio/effect/player_step3.mp3");
        loadClip(AudioEffect::player_step4, "res/audio/effect/player_step4.mp3");
        loadClip(AudioEffect::player_step5, "res/audio/effect/player_step5.mp3");
        loadClip(AudioEffect::player_jump_land, "res/audio/effect/player_jump_land.mp3");
        loadClip(AudioEffect::ladder_step1, "res/audio/effect/ladder_step1.mp3");
        loadClip(AudioEffect::ladder_step2, "res/audio/effect/ladder_step2.mp3");
        loadClip(AudioEffect::ladder_step3, "res/audio/effect/ladder_step3.mp3");
        loadClip(AudioEffect::ladder_step4, "res/audio/effect/ladder_step4.mp3");
        loadClip(AudioEffect::ladder_step5, "res/audio/effect/ladder_step5.mp3");
        loadClip(AudioEffect::ladder_step6, "res/audio/effect/ladder_step6.mp3");
        loadClip(AudioEffect::ladder_step7, "res/audio/effect/ladder_step7.mp3");
        loadClip(AudioEffect::squash1, "res/audio/effect/squash1.mp3");
        loadClip(AudioEffect::trap1, "res/audio/effect/trap1.mp3");
        loadClip(AudioEffect::trap2, "res/audio/effect/trap2.mp3");
    }
    catch (const ppl7::Exception& exp) {
        exp.print();
        throw;
    }
}

void AudioPool::loadLoops()
{
    try {
        loadClip(AudioLoop::birds_in_rain, "res/audio/loop/birds_in_the_rain.ogg");
        loadClip(AudioLoop::birds1, "res/audio/loop/birds1.ogg");
        loadClip(AudioLoop::birds2, "res/audio/loop/birds2.ogg");
        loadClip(AudioLoop::birds3, "res/audio/loop/birds2.ogg");
        loadClip(AudioLoop::cave1, "res/audio/loop/cave1.ogg");
        loadClip(AudioLoop::cave2, "res/audio/loop/cave2.ogg");
        loadClip(AudioLoop::cave3, "res/audio/loop/cave3.ogg");
        loadClip(AudioLoop::cave4, "res/audio/loop/cave4.ogg");
        loadClip(AudioLoop::crate_loop, "res/audio/loop/crate_loop.ogg");
        loadClip(AudioLoop::desert_at_night, "res/audio/loop/desert_at_night.mp3");
        loadClip(AudioLoop::earthquake, "res/audio/loop/earthquake.ogg");
        loadClip(AudioLoop::electric, "res/audio/loop/electric.mp3");
        loadClip(AudioLoop::fire1, "res/audio/loop/fire1.ogg");
        loadClip(AudioLoop::fire2, "res/audio/loop/fire2.ogg");
        loadClip(AudioLoop::fire3, "res/audio/loop/fire3.ogg");
        loadClip(AudioLoop::fire4, "res/audio/loop/fire4.ogg");
        loadClip(AudioLoop::fireball_fly, "res/audio/loop/fireball_fly.ogg");
        loadClip(AudioLoop::fireworks_loop, "res/audio/loop/fireworks_loop.ogg");
        loadClip(AudioLoop::gas1, "res/audio/loop/gas1.ogg");
        loadClip(AudioLoop::gas2, "res/audio/loop/gas2.ogg");
        loadClip(AudioLoop::gas3, "res/audio/loop/gas3.ogg");
        loadClip(AudioLoop::jungle1, "res/audio/loop/jungle1.ogg");
        loadClip(AudioLoop::jungle2, "res/audio/loop/jungle2.ogg");
        loadClip(AudioLoop::lavaloop1, "res/audio/loop/lavaloop1.ogg");
        loadClip(AudioLoop::lavaloop2, "res/audio/loop/lavaloop2.ogg");
        loadClip(AudioLoop::lavabubbles, "res/audio/loop/lavabubbles.ogg");
        loadClip(AudioLoop::night1, "res/audio/loop/night1.mp3");
        loadClip(AudioLoop::night2, "res/audio/loop/night2.mp3");
        loadClip(AudioLoop::night3, "res/audio/loop/night3.mp3");
        loadClip(AudioLoop::night4, "res/audio/loop/night4.mp3");
        loadClip(AudioLoop::nightowl1, "res/audio/loop/nightowl1.mp3");
        loadClip(AudioLoop::nightowl2, "res/audio/loop/nightowl2.mp3");
        loadClip(AudioLoop::nightowl3, "res/audio/loop/nightowl3.mp3");
        loadClip(AudioLoop::rain1, "res/audio/loop/rain1.mp3");
        loadClip(AudioLoop::rain2, "res/audio/loop/rain2.mp3");
        loadClip(AudioLoop::rain3, "res/audio/loop/rain3.mp3");
        loadClip(AudioLoop::rumble, "res/audio/loop/rumble.ogg");
        loadClip(AudioLoop::soft_rain, "res/audio/loop/soft_rain.mp3");
        loadClip(AudioLoop::thruster, "res/audio/loop/thruster.ogg");
        loadClip(AudioLoop::underwaterloop1, "res/audio/loop/underwaterloop1.ogg");
        loadClip(AudioLoop::water_bubble1, "res/audio/loop/water_bubble1.mp3");
        loadClip(AudioLoop::water_bubble2, "res/audio/loop/water_bubble2.ogg");
        loadClip(AudioLoop::water_bubble3, "res/audio/loop/water_bubble3.ogg");
        loadClip(AudioLoop::water_bubble4, "res/audio/loop/water_bubble4.ogg");
        loadClip(AudioLoop::water_bubble5, "res/audio/loop/water_bubble5.ogg");
        loadClip(AudioLoop::waterdrips1, "res/audio/loop/waterdrips.mp3");
        loadClip(AudioLoop::waterdrips2, "res/audio/loop/waterdrips2.mp3");
        loadClip(AudioLoop::waterdrips3, "res/audio/loop/waterdrips3.mp3");
        loadClip(AudioLoop::waterflow1, "res/audio/loop/waterflow1.ogg");
        loadClip(AudioLoop::waterflow2, "res/audio/loop/waterflow2.ogg");
        loadClip(AudioLoop::waterflow3, "res/audio/loop/waterflow3.ogg");
        loadClip(AudioLoop::waterpuddle, "res/audio/loop/waterpuddle.ogg");
        loadClip(AudioLoop::waves1, "res/audio/loop/waves1.ogg");
        loadClip(AudioLoop::waves2, "res/audio/loop/waves2.ogg");
        loadClip(AudioLoop::waves3, "res/audio/loop/waves3.ogg");
        loadClip(AudioLoop::waves4, "res/audio/loop/waves4.ogg");
        loadClip(AudioLoop::wind_crickets, "res/audio/loop/wind_grillen.ogg");
        loadClip(AudioLoop::wind1, "res/audio/loop/wind1.ogg");
        loadClip(AudioLoop::wind2, "res/audio/loop/wind2.ogg");
        loadClip(AudioLoop::wind3, "res/audio/loop/wind3.ogg");
        loadClip(AudioLoop::wind4, "res/audio/loop/wind4.ogg");
        loadClip(AudioLoop::wind_strong, "res/audio/loop/wind_strong.ogg");
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
