#include <math.h>
#include <ppl7-grafix.h>
#include "game.h"
#include "gamerenderer.h"
#include "player.h"
#include "constants.h"

Camera::Camera()
    : ppl7::grafix::PointF()
{
    zoom = 1.0f;
    target_zoom = 1.0f;
    zoom_speed = 0.0f;
    dead_zone.x = 400.0f;
    dead_zone.y = 400.0f;
    follow_player = true;
    logical_render_size.setSize(1920 * 2, 1080 * 2);
    look_ahead_x = 0.0f;
    look_ahead_distance = 300.0f;
    acceleration = 0.3f;
    decceleration = 0.8f;
    player_offset_y = 180.0f;
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

void Camera::setLogicalRenderSize(const ppl7::grafix::Size& size)
{
    logical_render_size = size;
}

ppl7::grafix::Size Camera::getLogicalRenderSize() const
{
    return logical_render_size;
}

void Camera::draw(GameRenderer& renderer, const GameViewport& viewport) const
{
    ppl7::grafix::Color color(255, 255, 255, 128);

    renderer.addRect(200, 200, logical_render_size.width - 400, logical_render_size.height - 400, color, 4);
    renderer.addLine((logical_render_size.width / 2.0f), 0, (logical_render_size.width / 2.0f), logical_render_size.height, color, 4);
    renderer.addLine(0, (logical_render_size.height / 2.0f), logical_render_size.width, (logical_render_size.height / 2.0f), color, 4);

    color.set(255, 0, 0, 128);
    renderer.addRect((logical_render_size.width / 2.0f) - dead_zone.x, (logical_render_size.height / 2.0f) - dead_zone.y,
                     dead_zone.x * 2.0f, dead_zone.y * 2.0f, color, 4);

    // ppl7::PrintDebug("Deadzone: (%.2f, %.2f)\n", dead_zone.x, dead_zone.y);
}

void Camera::stopMovement(float frame_rate_compensation)
{
    float decel = decceleration * frame_rate_compensation;
    if (speed.x > 0.0f) {
        speed.x -= decel;
        if (speed.x < 0.0f) speed.x = 0.0f;
    } else if (speed.x < 0.0f) {
        speed.x += decel;
        if (speed.x > 0.0f) speed.x = 0.0f;
    }
    if (speed.y > 0.0f) {
        speed.y -= decel;
        if (speed.y < 0.0f) speed.y = 0.0f;
    } else if (speed.y < 0.0f) {
        speed.y += decel;
        if (speed.y > 0.0f) speed.y = 0.0f;
    }
}

bool Camera::isPlayerInDeadZone() const
{
    float cam_center_x = x + (logical_render_size.width / 2.0f);
    float cam_center_y = y + (logical_render_size.height / 2.0f);
    float diff_x = player_position.x - cam_center_x;
    float diff_y = player_position.y - cam_center_y - player_offset_y;
    return (fabs(diff_x) <= dead_zone.x && fabs(diff_y) <= dead_zone.y);
}

void Camera::aimTarget(const ppl7::grafix::PointF& target, float frame_rate_compensation, const Player* player)
{
    ppl7::grafix::PointF cam_center(x + (logical_render_size.width / 2.0f), y + (logical_render_size.height / 2.0f));
    ppl7::PrintDebug("Player:=(%.2f, %.2f), Camera::aimTarget: target=(%.2f, %.2f), camera=(%.2f, %.2f), speed=(%.2f, %.2f)\n", player->x,
                     player->y, target.x, target.y, cam_center.x, cam_center.y, speed.x, speed.y);
    const float ACCEL = acceleration * frame_rate_compensation;
    float decel = decceleration * frame_rate_compensation;
    float distance = ppl7::grafix::Distance(*this, target);
    float max_speed = 20.0f;
    if (distance < 100.0f) max_speed = 10.0f;
    if (target.x > cam_center.x) {
        if (speed.x < max_speed)
            speed.x += ACCEL;
        else if (speed.x > max_speed) {
            speed.x -= decel;
            if (speed.x < max_speed) speed.x = max_speed;
        }

    } else if (target.x < cam_center.x) {
        if (speed.x > -max_speed)
            speed.x -= ACCEL;
        else if (speed.x < -max_speed) {
            speed.x += decel;
            if (speed.x > -max_speed) speed.x = -max_speed;
        }
    }
    if (target.y > cam_center.y) {
        speed.y += ACCEL;
    } else if (target.y < cam_center.y) {
        speed.y -= ACCEL;
    }
}

ppl7::grafix::PointF Camera::getTarget(const ppl7::grafix::PointF& movement, const Player* player)
{
    float target_look_ahead = 0;
    if (movement.x > 0.1f)
        target_look_ahead = look_ahead_distance;
    else if (movement.x < -0.1f)
        target_look_ahead = -look_ahead_distance;
    else {
        if (player->orientation == Physic::Right)
            target_look_ahead = look_ahead_distance * 0.5f;
        else if (player->orientation == Physic::Left)
            target_look_ahead = -look_ahead_distance * 0.5f;
    }
    return ppl7::grafix::PointF(player_position.x + target_look_ahead, player_position.y);
}

void Camera::update(const GameClock& clock, const Player* player)
{
    ppl7::grafix::PointF movement = player->position() - player_position;
    player_position = player->position();
    if (!follow_player) return;
    /*
    x = player_position.x - (render_size.width / 2.0f);
    y = player_position.y - (render_size.height / 2.0f) - player_offset_y; // Spieler etwas unter der Mitte platzieren
    return;
    */

    float deltaTime = clock.delta_time;

    ppl7::grafix::PointF cam_center(x + (logical_render_size.width / 2.0f), y + (logical_render_size.height / 2.0f));

    if (target_position.x == 0.0f && target_position.y == 0.0f) {
        target_position = player->position();
    }
    float diffX = player->x - cam_center.x;
    if (std::abs(diffX) > dead_zone.x) {
        target_position.x = (diffX > 0) ? player->x - dead_zone.x : player->x + dead_zone.x;
    }

    float diffY = player->y - cam_center.y;
    if (std::abs(diffY) > dead_zone.y) {
        target_position.y = (diffY > 0) ? player->y - dead_zone.y : player->y + dead_zone.y;
    }
    float smoothing = 2.0f;  // Wie schnell sie folgt
    float lookahead = 20.0f; // Wie weit sie vorausplant

    float lookaheadX = movement.x * lookahead;
    float finalTargetX = target_position.x + lookaheadX;

    float interpolation = 1.0f - std::exp(-smoothing * deltaTime);
    x += (finalTargetX - cam_center.x) * interpolation;
    y += (target_position.y - cam_center.y) * interpolation;

    return;

    float cam_center_x = x + (logical_render_size.width / 2.0f);
    float cam_center_y = y + (logical_render_size.height / 2.0f);

    if (speed.x == 0.0f && speed.y == 0.0f) {
        dead_zone.x = 200.0f;
        if (isPlayerInDeadZone()) {
            y = player->y - (logical_render_size.height / 2.0f) - player_offset_y; // Spieler etwas unter der Mitte platzieren
            return;
        }
    }
    // Falls sich der Spieler bewegt, berechnen wir die Target-Position
    if (movement.x != 0.0f) {
        ppl7::grafix::PointF target = getTarget(movement, player);
        aimTarget(target, clock.frame_rate_compensation, player);

    } else {
        // Spieler steht, Kamera abbremsen
        if (isPlayerInDeadZone())
            stopMovement(clock.frame_rate_compensation);
        else {
            // stopMovement(frame_rate_compensation);
            aimTarget(player_position, clock.frame_rate_compensation, player);
        }
    }
    x += speed.x * clock.frame_rate_compensation;
    // y += speed.y * frame_rate_compensation;
    y = player->y - (logical_render_size.height / 2.0f) - player_offset_y; // Spieler etwas unter der Mitte platzieren
}

#ifdef OLDCODE

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
        ppl7::grafix::Size halfsize = logical_render_size / 2;
        setPoint(player_position - ppl7::grafix::PointF(halfsize.width, halfsize.height));
        speed.setPoint(0.0f, 0.0f);
    }
    follow_player = enable;
}

bool Camera::isFollowingPlayer() const
{
    return follow_player;
}
