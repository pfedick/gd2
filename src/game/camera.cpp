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
    look_ahead_distance = 400.0f;
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
    const float ACCEL = 0.4f * frame_rate_compensation;
    const float FRICTION = 0.92f;        // Je kleiner, desto schneller bleibt die Kamera stehen
    const float LOOK_AHEAD_MAX = 500.0f; // Pixel Vorsprung in 4K
    const float LOOK_AHEAD_SPEED = 0.05f * frame_rate_compensation;
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
    bool outside_deadzone = (fabs(player_x - cam_center_x) > dead_zone.x);

    if (outside_deadzone || is_moving) {
        // State: Beschleunigen / Folgen
        // Die Kamera strebt eine Geschwindigkeit an, die den Abstand verringert
        float target_speed_x = player->velocity_move.x + (diff_x * 0.05f);
        speed.x += (target_speed_x - speed.x) * ACCEL;
    } else {
        // State: Spieler steht / in Deadzone -> Abbremsen und zentrieren
        speed.x *= FRICTION;
        // Sanftes "Einschwingen" in die Mitte, wenn fast stillstand
        if (!is_moving) {
            speed.x += diff_x * 0.01f * frame_rate_compensation;
        }
    }

    // 6. Vertikale Bewegung (meistens direkter oder mit eigener Deadzone)
    if (fabs(diff_y) > dead_zone.y || fabs(player->velocity_move.y) > 0.1f) {
        speed.y += (diff_y * 0.1f - speed.y) * ACCEL;
    } else {
        speed.y *= FRICTION;
    }

    // 7. Position aktualisieren
    x += speed.x * frame_rate_compensation;
    y += speed.y * frame_rate_compensation;
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
