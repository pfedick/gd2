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
    spriteset = NULL;
    nextid = 1;
    active_map = 0;
}

ParticleSystem::~ParticleSystem()
{
    clear();
    spriteset = NULL;
}

void ParticleSystem::clear()
{
    visible_particle_map[0].clear();
    visible_particle_map[1].clear();
    std::map<uint64_t, Particle*>::iterator it;
    for (it = particle_map.begin(); it != particle_map.end(); ++it) {
        delete it->second;
    }
    new_particles.clear();
    particles_to_delete.clear();
    particle_map.clear();
    nextid = 1;
}

void ParticleSystem::setParticleSpriteset(SpriteTexture* texture)
{
    spriteset = texture;
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
                            const ppl7::grafix::PointF& worldcoords,
                            const ppl7::grafix::Rect& viewport,
                            float frame_rate_compensation)
{
    // TODO:
    // Wir müssen eventuell prüfen, ob der globale Update-Thread bereits Daten bereitgestellt hat.
    // Oder das hier wird erst aufgerufen, wenn der Thread fertig ist.

    cleanupParticles();
    active_map = (active_map + 1) & 1;
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

void ParticleSystem::draw(GPUBatcher& batcher, const ppl7::grafix::Rect& viewport, const ppl7::grafix::Point& worldcoords) const
{
#ifdef DEBUGOUT
    ppl7::PrintDebugTime("[%llu] ParticleSystem::draw => DONE\n", ppl7::ThreadID());
#endif
    std::map<uint32_t, Particle*>::const_iterator it;
    ppl7::grafix::Point coords(viewport.x1 - worldcoords.x, viewport.y1 - worldcoords.y);

    for (it = visible_particle_map[active_map].begin(); it != visible_particle_map[active_map].end(); ++it) {
        const Particle* particle = it->second;
        if (particle->sprite_set <= 2) {
            batcher.addSprite(*spriteset, particle->sprite_no, particle->p.x + coords.x, particle->p.y + coords.y, particle->scale,
                              particle->scale, 0.0f, particle->color_mod);

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
    return visible_particle_map[active_map].size();
}
