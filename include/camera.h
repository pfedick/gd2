#ifndef INCLUDE_CAMERA_H_
#define INCLUDE_CAMERA_H_

#include <ppl7-grafix.h>

class Player;
class GameRenderer;
class GameClock;

class Camera : public ppl7::grafix::PointF
{
private:
    float zoom;
    float target_zoom;
    float zoom_speed;
    float look_ahead_x;        // Der aktuelle gleitende Vorsprung
    float look_ahead_distance; // Maximaler Vorsprung
    float acceleration;
    float decceleration;
    float player_offset_y;
    ppl7::grafix::PointF dead_zone;
    ppl7::grafix::PointF speed;
    ppl7::grafix::PointF player_position;
    ppl7::grafix::PointF target_position;
    ppl7::grafix::Size render_size;
    bool follow_player;

    void stopMovement(float frame_rate_compensation);
    bool isPlayerInDeadZone() const;
    void aimTarget(const ppl7::grafix::PointF& target, float frame_rate_compensation, const Player* player);
    ppl7::grafix::PointF getTarget(const ppl7::grafix::PointF& movement, const Player* player);

public:
    Camera();
    void setZoom(float zoom);
    float getZoom() const;
    void setTargetZoom(float zoom, float speed);
    void setRenderSize(const ppl7::grafix::Size& size);
    void update(const GameClock& clock, const Player* player);
    void setPosition(const ppl7::grafix::PointF& pos);
    void setDeadZone(float x, float y);
    void setFollowPlayer(bool enable);
    bool isFollowingPlayer() const;
    void draw(GameRenderer& batcher, const GameViewport& viewport) const;
};

#endif // INCLUDE_CAMERA_H_