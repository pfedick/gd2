#include <ppl7.h>
#include <ppl7-grafix.h>
#include "particle.h"
#include "level.h"
#include "game.h"

ParticleUpdateThread::ParticleUpdateThread()
{
    thread_duration = 0.0f;
    thread_running = false;
}

double ParticleUpdateThread::getThreadDuration() const
{
    return thread_duration;
}

bool ParticleUpdateThread::isRunning() const
{
    return thread_running;
}

void ParticleUpdateThread::wakeUp()
{
    mutex.signal();
}

void ParticleUpdateThread::run()
{
    thread_running = false;
    Level& level = GetGame().level;

    while (!threadShouldStop()) {
        mutex.wait();
        if (threadShouldStop()) break;
        thread_running = true;
        double thread_start_time = ppl7::GetMicrotime();
        // Hier sollte eigentlich nur eine Funktion im ParticleSystem aufgerufen werden,
        // die dann die Partikel updatet.
        level.updateParticles(thread_start_time);
        thread_duration = ppl7::GetMicrotime() - thread_start_time;
        thread_running = false;
    }
}

void ParticleUpdateThread::waitForFinished() const
{
    while (thread_running) {
        ppl7::MSleep(1);
    }
}

#ifdef OLDCODE
// Der ganze Code hier sollte in eine Update-Funktion im ParticleSystem wandern. An dieser Stelle hier
// itteriert der Thread nur über die Parallax-Layer und ruft dort die Update-Funktion auf.

class PlaneCoords
{
public:
    void init(const ppl7::grafix::PointF& worldcoords, const ppl7::grafix::Rect& viewport, float factor);
    float left;
    float top;
    float right;
    float bottom;
};

void PlaneCoords::init(const ppl7::grafix::PointF& worldcoords, const ppl7::grafix::Rect& viewport, float factor)
{
    left = worldcoords.x * factor - 128;
    top = worldcoords.y * factor - 128;
    right = worldcoords.x * factor + viewport.width() + 128;
    bottom = worldcoords.y * factor + viewport.height() + 128;
}

void ParticleUpdateThread::run()
{
#ifdef DEBUGOUT
    ppl7::PrintDebugTime("[%llu] ParticleUpdateThread STARTED\n", ppl7::ThreadID());
#endif
    thread_running = false;
    while (!threadShouldStop()) {
#ifdef DEBUGOUT
        ppl7::PrintDebugTime("[%llu] ParticleUpdateThread waiting for signal\n", ppl7::ThreadID());
#endif

        mutex.wait();
        datamutex.lock();
        thread_running = true;
#ifdef DEBUGOUT
        ppl7::PrintDebugTime("[%llu] ParticleUpdateThread running\n", ppl7::ThreadID());
#endif

        double thread_start_time = ppl7::GetMicrotime();
        if (visible_particle_map) {
            for (int i = 0; i < static_cast<int>(Particle::Layer::maxLayer); i++)
                planec[i].init(worldcoords, viewport, ParticlePlaneFactor[i]);
            /*
            float left=worldcoords.x - 128;
            float top=worldcoords.y - 128;
            float right=worldcoords.x + viewport.width() + 128;
            float bottom=worldcoords.y + viewport.height() + 128;
            */

            std::map<uint64_t, Particle*>::iterator it;
            for (it = ps->particle_map.begin(); it != ps->particle_map.end(); ++it) {
                Particle* particle = it->second;
                if (time <= particle->death_time) {
                    particle->age = (time - particle->birth_time) / particle->life_time; // Rises from 0.0f to 1.0f
                    particle->update(time, frame_rate_compensation);
                    const PlaneCoords& pc = planec[static_cast<int>(particle->layer)];
                    if (particle->p.x > pc.left && particle->p.y > pc.top && particle->p.x < pc.right && particle->p.y < pc.bottom) {
                        uint32_t id = (uint32_t)(((uint32_t)particle->p.y & 0xffff) << 16) | (uint32_t)((uint32_t)particle->p.x & 0xffff);
                        visible_particle_map[static_cast<int>(particle->layer)].insert(std::pair<uint32_t, Particle*>(id, particle));
                        particle->visible = true;
                    } else {
                        particle->visible = false;
                    }
                } else {
                    ps->particles_to_delete.push_back(it->first);
                }
            }
        }
        thread_duration = ppl7::GetMicrotime() - thread_start_time;
        thread_running = false;
        datamutex.unlock();
        // ppl7::PrintDebugTime("    ParticleUpdateThread: sleep\n");

        // printf("ParticleUpdateThread signaled\n");
    }
#ifdef DEBUGOUT
    ppl7::PrintDebugTime("[%llu] ParticleUpdateThread ENDED\n", ppl7::ThreadID());
#endif
}

#endif