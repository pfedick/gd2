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

    // 1. Konstanten für das Feeling (können später in den Header oder eine Config)
    const float ACCEL = 0.1f * frame_rate_compensation;
    const float FRICTION = 0.90f;        // Je kleiner, desto schneller bleibt die Kamera stehen
    const float LOOK_AHEAD_MAX = 300.0f; // Pixel Vorsprung in 4K
    const float LOOK_AHEAD_SPEED = 0.02f * frame_rate_compensation;
    const float VERTICAL_OFFSET = 200.0f; // Spieler etwas unter der Mitte platzieren

    // 2. Aktuelle Mitte der Kamera in Weltkoordinaten
    float cam_center_x = x + (render_size.width / 2.0f);
    float cam_center_y = y + (render_size.height / 2.0f);

    // 3. Ziel-Vorsprung berechnen (Look-Ahead)
    float target_look_ahead = 0;
    if (player->velocity_move.x > 0.5f)
        target_look_ahead = LOOK_AHEAD_MAX;
    else if (player->velocity_move.x < -0.5f)
        target_look_ahead = -LOOK_AHEAD_MAX;
    else {
        // Wenn Spieler steht, entscheidet die Blickrichtung (Orientation)
        if (player->orientation == Physic::Right)
            target_look_ahead = LOOK_AHEAD_MAX * 0.5f;
        else if (player->orientation == Physic::Left)
            target_look_ahead = -LOOK_AHEAD_MAX * 0.5f;
    }
    // Look-Ahead sanft anpassen (Interpolation)
    look_ahead_x += (target_look_ahead - look_ahead_x) * LOOK_AHEAD_SPEED;

    // 4. Distanzen berechnen
    float player_x = player->x;
    float player_y = player->y - 128.0f; // Versatz, da Pivot an den Füßen ist

    float diff_x = (player_x + look_ahead_x) - cam_center_x;
    float diff_y = (player_y - VERTICAL_OFFSET) - cam_center_y;

    // 5. Zustands-Logik (Horizontal)
    bool is_moving = (fabs(player->velocity_move.x) > 0.1f);
    bool outside_deadzone_x = (fabs(player_x - cam_center_x) > dead_zone.x);
    // Kamera ist aktiv, wenn sie bereits rollt ODER wenn der Spieler die Deadzone verlässt.
    bool camera_active_x = (fabs(speed.x) > 0.1f) || outside_deadzone_x;

    if (camera_active_x) {
        // State: Beschleunigen / Folgen / Einschwingen
        float target_speed_x = player->velocity_move.x + (diff_x * 0.05f);
        speed.x += (target_speed_x - speed.x) * ACCEL;
    } else {
        // State: Deadzone aktiv - Kamera steht still
        speed.x *= FRICTION;
        if (fabs(speed.x) < 0.05f) speed.x = 0.0f;
    }

    // 6. Vertikale Bewegung (Analog)
    bool outside_deadzone_y = (fabs(diff_y) > dead_zone.y);
    bool camera_active_y = (fabs(speed.y) > 0.1f) || outside_deadzone_y;

    if (camera_active_y) {
        float target_speed_y = player->velocity_move.y + (diff_y * 0.1f);
        speed.y += (target_speed_y - speed.y) * ACCEL;
    } else {
        speed.y *= FRICTION;
        if (fabs(speed.y) < 0.05f) speed.y = 0.0f;
    }

    // 7. Position aktualisieren
    x += speed.x * frame_rate_compensation;
    y += speed.y * frame_rate_compensation;
}

#ifdef VARIATION
void Camera::update(double time, float frame_rate_compensation, const Player* player)
{
    player_position = player->position();
    if (!follow_player) return;

    // --- DEINE FAVORISIERTEN WERTE ---
    const float ACCEL = 0.9f * frame_rate_compensation; // Feste Kraft (war 0.15, viel zu wenig)
    const float FRICTION = 0.94f;                       // Kräftige Reibung gegen das "Vorbeifliegen"
    const float LOOK_AHEAD_SPEED = 0.07f * frame_rate_compensation;
    const float PLAYER_PIVOT_Y = 150.0f;
    const float TARGET_SCREEN_Y = 200.0f;

    float cam_center_x = x + (render_size.width / 2.0f);
    float cam_center_y = y + (render_size.height / 2.0f);
    float dist_x = player->x - cam_center_x;
    float dist_y = (player->y - PLAYER_PIVOT_Y) - (cam_center_y + TARGET_SCREEN_Y);

    // 1. Sanfter Look-Ahead (wie gehabt)
    float target_look_ahead = 0;
    if (fabs(player->velocity_move.x) > 1.0f) {
        target_look_ahead = (player->velocity_move.x > 0) ? look_ahead_distance : -look_ahead_distance;
    }
    look_ahead_x += (target_look_ahead - look_ahead_x) * LOOK_AHEAD_SPEED;

    // 2. Deadzone-State Check
    // "Kamera steht still und Spieler ist drin" -> Deadzone aktiv
    bool camera_is_moving_x = (fabs(speed.x) > 0.5f);
    bool outside_deadzone_x = (fabs(dist_x) > dead_zone.x);

    if (!camera_is_moving_x && !outside_deadzone_x) {
        speed.x = 0;
    } else {
        // Wir folgen dem Zielpunkt (Spieler + Look-Ahead)
        float target_x_pos = player->x + look_ahead_x - (render_size.width / 2.0f);

        // Feste Beschleunigung in Zielrichtung
        if (target_x_pos > x)
            speed.x += ACCEL;
        else if (target_x_pos < x)
            speed.x -= ACCEL;

        speed.x *= FRICTION;
    }

    // Analog für Y
    bool camera_is_moving_y = (fabs(speed.y) > 0.5f);
    bool outside_deadzone_y = (fabs(dist_y) > dead_zone.y);

    if (!camera_is_moving_y && !outside_deadzone_y) {
        speed.y = 0;
    } else {
        float target_y_pos = (player->y - PLAYER_PIVOT_Y) - (render_size.height / 2.0f) - TARGET_SCREEN_Y;
        if (target_y_pos > y)
            speed.y += ACCEL;
        else if (target_y_pos < y)
            speed.y -= ACCEL;
        speed.y *= FRICTION;
    }

    // 3. Position aktualisieren
    x += speed.x;
    y += speed.y;
}
#endif

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
