#include <math.h>
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

void Camera::update(double time, float frame_rate_compensation, const GameViewport& viewport)
{
    if (follow_player) {
        int screen_width = viewport.width();
        int screen_height = viewport.height();
        float target_x = player_position.x - (viewport.width() / 2.0f);
        float target_y = player_position.y - (viewport.height() / 2.0f) - 3 * 64.0f;

        float diff_x = fabs(target_x - x);
        float diff_y = fabs(target_y - y);

        if (diff_x < dead_zone_x) {
            if (speed.x > 0.0f) {
                speed.x -= 0.1f * frame_rate_compensation;
                if (speed.x < 0.0f) speed.x = 0.0f;
            } else if (speed.x < 0.0f) {
                speed.x += 0.1f * frame_rate_compensation;
                if (speed.x > 0.0f) speed.x = 0.0f;
            }
        } else {
            if (target_x > x) {
                speed.x += 0.1f * frame_rate_compensation;
                if (speed.x > 10.0f) speed.x = 10.0f;
            } else if (target_x < x) {
                speed.x -= (0.1f * frame_rate_compensation);
                if (speed.x < -10.0f) speed.x = -10.0f;
            }
        }
        x += speed.x;
        y += speed.y;
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
