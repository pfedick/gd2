#include <ppl7.h>
#include <ppl7-grafix.h>
#include <SDL3/SDL.h>
#include "game.h"
#include "particle.h"

// #define DEBUGOUT

static ParticleSystem* particle_system = NULL;

ParticleSystem* GetParticleSystem()
{
    return particle_system;
}

ParticleSystem::ParticleSystem()
{
    if (!particle_system) particle_system = this;
    for (int i = 0; i < ParticleSpriteset::MaxSpritesets; i++) {
        spriteset[i] = new SpriteTexture();
    }
    nextid = 1;
    active_map = 0;
    update_thread.ps = this;
    update_thread.threadStart();
}

ParticleSystem::~ParticleSystem()
{
    update_thread.mutex.signal();
    update_thread.threadStop();
    clear();
    for (int i = 0; i < ParticleSpriteset::MaxSpritesets; i++) {
        delete spriteset[i];
    }

    if (particle_system == this) particle_system = NULL;
}

void ParticleSystem::clear()
{
    waitForUpdateThreadFinished();
    for (int i = 0; i < static_cast<int>(Particle::Layer::maxLayer); i++) {
        visible_particle_map[0][i].clear();
        visible_particle_map[1][i].clear();
    }
    std::map<uint64_t, Particle*>::iterator it;
    for (it = particle_map.begin(); it != particle_map.end(); ++it) {
        delete it->second;
    }
    new_particles.clear();
    particles_to_delete.clear();
    particle_map.clear();
    nextid = 1;
}

void ParticleSystem::loadSpritesets(GPUContext& gpu)
{
    // SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
    spriteset[ParticleSpriteset::GenericParticles]->enableOutlines(false);
    spriteset[ParticleSpriteset::GenericParticles]->enableMemoryBuffer(false);
    spriteset[ParticleSpriteset::GenericParticles]->load(gpu, "res/particles.tex");
}

void ParticleSystem::addParticle(Particle* particle)
{
    if (!particle) return;
    if (particle->birth_time == 0.0f || particle->birth_time > particle->death_time) return;
    particle->life_time = particle->death_time - particle->birth_time;
    new_particles.insert(std::pair<uint64_t, Particle*>(nextid, particle));
    nextid++;
}

void ParticleSystem::deleteParticle(uint64_t id)
{
    std::map<uint64_t, Particle*>::const_iterator it;
    it = particle_map.find(id);
    if (it != particle_map.end()) {
        Particle* particle = it->second;
        particle_map.erase(it);
        particle->sprite_set = 1234567;
        delete particle;
    }
}

void ParticleSystem::update(double time,
                            TileTypePlane& ttplane,
                            Player& player,
                            const ppl7::grafix::Point& worldcoords,
                            const ppl7::grafix::Rect& viewport,
                            float frame_rate_compensation)
{
    if (update_thread.isRunning()) {
        ppl7::PrintDebugTime("Particle Update Thread too slow!\n");
        while (update_thread.isRunning())
            ppl7::MSleep(1);
    }
#ifdef DEBUGOUT
    ppl7::PrintDebugTime("[%llu] ParticleSystem::update: started, map=%d\n", ppl7::ThreadID(), active_map);
#endif
    cleanupParticles();
    update_thread.frame_rate_compensation = frame_rate_compensation;
    update_thread.time = time;
    update_thread.ttplane = &ttplane;
    update_thread.player = &player;
    update_thread.worldcoords.x = worldcoords.x;
    update_thread.worldcoords.y = worldcoords.y;
    update_thread.viewport = viewport;
#ifdef DEBUGOUT
    ppl7::PrintDebugTime("[%llu] ParticleSystem::update: set visible map to %d\n", ppl7::ThreadID(), active_map);
#endif
    update_thread.setVisibleParticleMapAndContinue(visible_particle_map[active_map]);
    active_map = (active_map + 1) & 1;
#ifdef DEBUGOUT
    ppl7::PrintDebugTime("[%llu] ParticleSystem::update:  ended, draw on map: %d, send signal to ParticleUpdateThread\n", ppl7::ThreadID(),
                         active_map);
#endif
}

double ParticleSystem::waitForUpdateThreadFinished()
{
#ifdef DEBUGOUT
    ppl7::PrintDebugTime("[%llu] ParticleSystem::waitForUpdateThreadFinished\n", ppl7::ThreadID());
#endif
    while (update_thread.isRunning())
        ppl7::MSleep(1);
#ifdef DEBUGOUT
    ppl7::PrintDebugTime("[%llu] ParticleSystem::waitForUpdateThreadFinished => OK\n", ppl7::ThreadID());
#endif

    return update_thread.getThreadDuration();
}

void ParticleSystem::cleanupParticles()
{
#ifdef DEBUGOUT
    ppl7::PrintDebugTime("[%llu] ParticleSystem::cleanupParticles, particles to delete: %zd, insert: %zd\n", ppl7::ThreadID(),
                         particles_to_delete.size(), new_particles.size());
#endif
    if (particles_to_delete.size() > 0) {
        // ppl7::PrintDebugTime("deleting %zd particles\n", particles_to_delete.size());
        std::list<uint64_t>::iterator dit;
        for (dit = particles_to_delete.begin(); dit != particles_to_delete.end(); ++dit) {
            deleteParticle(*dit);
        }
        particles_to_delete.clear();
    }
    // Insert new Particles
    std::map<uint64_t, Particle*>::const_iterator it;
    for (it = new_particles.begin(); it != new_particles.end(); ++it) {
        particle_map.insert(std::pair<uint64_t, Particle*>(it->first, it->second));
    }
    new_particles.clear();
#ifdef DEBUGOUT
    ppl7::PrintDebugTime("[%llu] ParticleSystem::cleanupParticles => DONE\n", ppl7::ThreadID());
#endif
}

void ParticleSystem::draw(GPUBatcher& batcher,
                          const ppl7::grafix::Rect& viewport,
                          const ppl7::grafix::Point& worldcoords,
                          Particle::Layer layer) const
{
#ifdef DEBUGOUT
    ppl7::PrintDebugTime("[%llu] ParticleSystem::draw => DONE\n", ppl7::ThreadID());
#endif
    std::map<uint32_t, Particle*>::const_iterator it;
    ppl7::grafix::Point coords(viewport.x1 - worldcoords.x, viewport.y1 - worldcoords.y);
    int l = static_cast<int>(layer);
    // ppl7::PrintDebugTime("   draw with active_map=%d\n", active_map);

    for (it = visible_particle_map[active_map][l].begin(); it != visible_particle_map[active_map][l].end(); ++it) {
        const Particle* particle = it->second;
        if (particle->sprite_set <= 2) {
            spriteset[particle->sprite_set]->drawScaled(renderer, particle->p.x + coords.x, particle->p.y + coords.y, particle->sprite_no,
                                                        particle->scale, particle->color_mod);

        } else {
            ppl7::PrintDebugTime("[%llu] Found invalid "
                                 "particle!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n",
                                 ppl7::ThreadID());
        }
    }
#ifdef DEBUGOUT
    ppl7::PrintDebugTime("[%llu] ParticleSystem::draw => DONE\n", ppl7::ThreadID());
#endif
}

size_t ParticleSystem::count() const
{
    return particle_map.size();
}

size_t ParticleSystem::countVisible() const
{
    size_t c = 0;
    for (int i = 0; i < static_cast<int>(Particle::Layer::maxLayer); i++) {
        c += visible_particle_map[active_map][i].size();
    }
    return c;
}

/*
ppl7::String ParticleSystem::layerName(Particle::Layer layer)
{
    switch (layer) {
    case Particle::Layer::BehindBricks: return "BehindBricks";
    case Particle::Layer::BeforePlayer: return "BeforePlayer";
    case Particle::Layer::BehindPlayer: return "BehindPlayer";
    case Particle::Layer::BackplaneFront: return "BackplaneFront";
    case Particle::Layer::BackplaneBack: return "BackplaneBack";
    case Particle::Layer::FrontplaneFront: return "FrontplaneFront";
    case Particle::Layer::FrontplaneBack: return "FrontplaneBack";
    default: return "unknown";
    }
    return "unknown";
}
*/
