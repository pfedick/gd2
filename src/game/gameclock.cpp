#include "game.h"

void GameClock::update()
{
    double current_time = ppl7::GetMicrotime();
    delta_time = current_time - time;
    time = current_time;
    frame_rate_compensation = (delta_time > 0.0f) ? (delta_time / (1.0f / 60.0f)) : 1.0f;
    frame_count++;
    fps_frame_count++;
    current_second = static_cast<uint64_t>(time);
    if (current_second > fps_start_time) {
        fps_start_time = current_second;
        fps = fps_frame_count;
        fps_frame_count = 0;
    }
}
