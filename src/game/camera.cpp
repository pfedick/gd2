#include "game.h"
#include "constants.h"

Camera::Camera()
    : ppl7::grafix::PointF()
{
    zoom = 1.0f;
    target_zoom = 1.0f;
    zoom_speed = 0.0f;
    dead_zone_x = 160.0f;
    dead_zone_y = 128;
    move_speed = 0.0f;
    follow_player = true;
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

void Camera::update(double time, float frame_rate_compensation)
{
    if (zoom < target_zoom) {
        zoom += zoom_speed * frame_rate_compensation;
        if (zoom > target_zoom) zoom = target_zoom;
    } else if (zoom > target_zoom) {
        zoom -= zoom_speed * frame_rate_compensation;
        if (zoom < target_zoom) zoom = target_zoom;
    }
    if (follow_player) {
        float diff_x = player_position.x - x;
        float diff_y = player_position.y - y;

        if (diff_x > dead_zone_x) {
            x += diff_x - dead_zone_x;
        } else if (diff_x < -dead_zone_x) {
            x += diff_x + dead_zone_x;
        }

        if (diff_y > dead_zone_y) {
            y += diff_y - dead_zone_y;
        } else if (diff_y < -dead_zone_y) {
            y += diff_y + dead_zone_y;
        }
    }
}
void Camera::setPlayerPosition(const ppl7::grafix::PointF& pos)
{
    player_position = pos;
}

void Camera::setPosition(const ppl7::grafix::PointF& pos)
{
    x = pos.x;
    y = pos.y;
}

void Camera::setDeadZone(float x, float y)
{
    dead_zone_x = x;
    dead_zone_y = y;
}

void Camera::setFollowPlayer(bool enable)
{
    follow_player = enable;
}

bool Camera::isFollowingPlayer() const
{
    return follow_player;
}
