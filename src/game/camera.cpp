#include <math.h>
#include "game.h"
#include "player.h"
#include "constants.h"

Camera::Camera()
    : ppl7::grafix::PointF()
{
    zoom = 1.0f;
    target_zoom = 1.0f;
    zoom_speed = 0.0f;
    dead_zone.x = 160.0f;
    dead_zone.y = 128;
    follow_player = true;
    render_size.setSize(1920 * 2, 1080 * 2);
    look_ahead_x = 0.0f;
    look_ahead_distance = 300.0f;
}
void Camera::setZoom(float zoom)
{
    this->zoom = zoom;
}

float Camera::getZoom() const
{
    return zoom;
}

void Camera::setTargetZoom(float zoom, float speed)
{
    target_zoom = zoom;
    zoom_speed = speed;
}

void Camera::setRenderSize(const ppl7::grafix::Size& size)
{
    render_size = size;
}

void Camera::update(double time, float frame_rate_compensation, const Player* player)
{
    player_position = player->position();
    if (!follow_player) return;

    // 1. Parameter für das Feeling
    const float SMOOTHING = 0.04f * frame_rate_compensation;
    const float LOOK_AHEAD_SPEED = 0.02f * frame_rate_compensation;
    const float PLAYER_PIVOT_ADJUST = 150.0f;    // Fokuspunkt von Füßen Richtung Körpermitte
    const float VERTICAL_SCREEN_OFFSET = 200.0f; // Spieler im unteren Drittel

    // 2. Aktuelle Mitte der Kamera in Weltkoordinaten
    float cam_center_x = x + (render_size.width / 2.0f);
    float cam_center_y = y + (render_size.height / 2.0f);

    // 3. Ziel-Vorsprung (Look-Ahead) berechnen
    float target_look_ahead = 0;
    if (player->velocity_move.x > 1.0f)
        target_look_ahead = look_ahead_distance;
    else if (player->velocity_move.x < -1.0f)
        target_look_ahead = -look_ahead_distance;
    else {
        // Im Stehen nur halber Look-Ahead basierend auf Blickrichtung
        if (player->orientation == Physic::Right)
            target_look_ahead = look_ahead_distance * 0.4f;
        else if (player->orientation == Physic::Left)
            target_look_ahead = -look_ahead_distance * 0.4f;
    }

    // Look-Ahead sanft gleiten lassen (verhindert Ruckeln bei Richtungswechsel)
    look_ahead_x += (target_look_ahead - look_ahead_x) * LOOK_AHEAD_SPEED;

    // 4. Deadzone als "Fenster" (Lock-Box)
    // Wenn der Spieler sich innerhalb der Deadzone bewegt, "schiebt" er das Fenster nicht.
    float target_base_x = cam_center_x;
    float dist_x = player->x - cam_center_x;
    if (dist_x > dead_zone.x)
        target_base_x = player->x - dead_zone.x;
    else if (dist_x < -dead_zone.x)
        target_base_x = player->x + dead_zone.x;

    float player_focus_y = player->y - PLAYER_PIVOT_ADJUST;
    float target_base_y = cam_center_y + VERTICAL_SCREEN_OFFSET;
    float dist_y = player_focus_y - (cam_center_y + VERTICAL_SCREEN_OFFSET);
    if (dist_y > dead_zone.y)
        target_base_y = player_focus_y - dead_zone.y;
    else if (dist_y < -dead_zone.y)
        target_base_y = player_focus_y + dead_zone.y;

    // 5. Finales Ziel (Basis + Look-Ahead)
    float final_target_x = target_base_x + look_ahead_x;
    float final_target_y = target_base_y - VERTICAL_SCREEN_OFFSET;

    // 6. Sanfte Annäherung (Interpolation)
    float ideal_speed_x = (final_target_x - cam_center_x) * SMOOTHING;
    float ideal_speed_y = (final_target_y - cam_center_y) * SMOOTHING;

    float accel_limit = 0.1f * frame_rate_compensation;
    speed.x += (ideal_speed_x - speed.x) * accel_limit;
    speed.y += (ideal_speed_y - speed.y) * accel_limit;

    // 7. Position aktualisieren
    x += speed.x;
    y += speed.y;
}

void Camera::setPosition(const ppl7::grafix::PointF& pos)
{
    x = pos.x;
    y = pos.y;
}

void Camera::setDeadZone(float x, float y)
{
    dead_zone.x = x;
    dead_zone.y = y;
}

void Camera::setFollowPlayer(bool enable)
{
    if (enable && !follow_player) {
        x = player_position.x;
        y = player_position.y;
        speed.setPoint(0.0f, 0.0f);
    }
    follow_player = enable;
}

bool Camera::isFollowingPlayer() const
{
    return follow_player;
}
