#include "game.h"

void GameClock::update()
{
    double current_time = ppl7::GetMicrotime();
    delta_time = current_time - time;
    time = current_time;
    frame_rate_compensation = (delta_time > 0.0f) ? (delta_time / (1.0f / 60.0f)) : 1.0f;
    if (frame_rate_compensation > 2.0f) frame_rate_compensation = 2.0f;
    frame_count++;
    fps_frame_count++;
    current_second = static_cast<uint64_t>(time);
    if (current_second > fps_start_time) {
        fps_start_time = current_second;
        fps = fps_frame_count;
        fps_frame_count = 0;
    }
}

void GameClock::limit(int target_fps)
{
    if (target_fps <= 0) return;
    double target_frametime = 1.0 / (double)target_fps;
    double current_time = ppl7::GetMicrotime();
    double elapsed = current_time - time;

    if (elapsed < target_frametime) {
        // Wir berechnen die verbleibende Zeit in Nanosekunden
        double remaining_seconds = target_frametime - elapsed;
        Uint64 remaining_ns = (Uint64)(remaining_seconds * 1000000000.0);

        // SDL_DelayPrecise nutzt die bestmögliche Methode des Betriebssystems
        SDL_DelayPrecise(remaining_ns);
    }
}
