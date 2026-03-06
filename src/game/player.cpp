#include "game.h"
#include <ppl7-grafix.h>
#include "player.h"
#include "objectsystem.h"
#include "constants.h"
#include "gamecontroller.h"
#include "audio.h"
#include "audiopool.h"
#include "animation.h"

static AnimationDefinition RunCycleLeft(36, 51, true, 51, 0.01666f * 2.0f);
static AnimationDefinition RunCycleRight(16, 31, true, 31, 0.01666f * 2.0f);
static AnimationDefinition JumpUp(53, 58, false, 58, 0.01666f * 2.0f);
static AnimationDefinition JumpRightUp(60, 65, false, 65, 0.01666f * 2.0f);
static AnimationDefinition JumpRightDown(66, 71, false, 71, 0.01666f * 2.0f);
static AnimationDefinition JumpRightLand(72, 77, false, 0, 0.01666f * 2.0f);
static AnimationDefinition JumpLeftUp(78, 84, false, 84, 0.01666f * 2.0f);
static AnimationDefinition JumpLeftDown(85, 90, false, 90, 0.01666f * 2.0f);
static AnimationDefinition JumpLeftLand(91, 96, false, 6, 0.01666f * 2.0f);

static float getMaxAirFromDifficultyLevel(Config::DifficultyLevel level)
{
    switch (level) {
    case Config::DifficultyLevel::easy:
        return 45.0f;
    case Config::DifficultyLevel::normal:
        return 30.0f;
    case Config::DifficultyLevel::hard:
        return 20.0f;
    }
    return 30.0f;
}

Player::Player(Game* game)
    : keys(game)
{
    x = y = 0;
    max_run_speed = 16.0f;
    currentLayer = ParallaxLayerId::Player;
    parallax_scale = 1.0f;
    scale = 1.0f;
    last_animation_sound_played = -1;
    sprite_resource = NULL;
    tiletype_resource = NULL;
    idle_timeout = 0.0f;
    animation.setStaticFrame(3);
    points = 0;
    health = 100;
    lifes = 3;
    godmode = false;
    this->game = game;
    dead = false;
    visible = true;
    controlEnabled = true;
    color_modulation.setColor(255, 255, 255, 255);
    ambient_sound = NULL;
    animation.setDefaultSpeed(0.01666f);
    airStart = 0.0f;
    time = ppl7::GetMicrotime();
    startIdle = time + 1.0f;
    frame_rate_compensation = 0.0f;
}

Player::~Player()
{
    if (ambient_sound) {
        getAudioPool().stopInstace(ambient_sound);
        delete ambient_sound;
        ambient_sound = NULL;
    }
}

void Player::resetState()
{
    animation.resetSpeed();
    last_animation_sound_played = -1;
    points = 0;
    health = 100;
    lifes = 3;
    godmode = false;
    dead = false;
    visible = true;
    last_aircheck = 0;
    stand();
    Inventory.clear();
    object_counter.clear();
    SpecialObjects.clear();
    color_modulation.setColor(255, 255, 255, 255);
    airStart = 0.0f;
    startIdle = ppl7::GetMicrotime() + 5.0f;
    if (ambient_sound) {
        ambient_sound->setAutoDelete(true);
        ambient_sound->fadeout(2.0f);
        ambient_sound = NULL;
    }
}

void Player::resetLevelObjects()
{
    last_animation_sound_played = -1;
    animation.resetSpeed();
    Inventory.clear();
    object_counter.clear();
    last_aircheck = 0;
    SpecialObjects.clear();
    health = 100;
    lifes = 3;
    points = 0;
}

void Player::setVisible(bool flag)
{
    visible = flag;
}

ppl7::grafix::PointF Player::position() const
{
    return ppl7::grafix::PointF(x, y);
}

ParallaxLayerId Player::getParallaxLayer() const
{
    return currentLayer;
}

void Player::setParallaxLayer(ParallaxLayerId layer, float parallax_scale)
{
    currentLayer = layer;
    this->parallax_scale = parallax_scale;
}

void Player::setZeroVelocity()
{
    velocity_move.x = 0;
    velocity_move.y = 0;
}

void Player::setSpriteResource(const SpriteTexture& resource)
{
    sprite_resource = &resource;
}

void Player::setTileTypeResource(const SpriteTexture& resource)
{
    tiletype_resource = &resource;
}

void Player::setGodMode(bool enabled)
{
    godmode = enabled;
}

bool Player::godModeEnabled() const
{
    return godmode;
}

void Player::move(int x, int y)
{
    this->x = x;
    this->y = y;
}

void Player::draw(GameRenderer& renderer, const GameViewport& viewport, const ppl7::grafix::Point& worldcoords, float size) const
{
    if (!visible) return;
    ppl7::grafix::PointF p(x - worldcoords.x, y - worldcoords.y);
    int frame = animation.getFrame();

    renderer.addSprite(*sprite_resource, frame, p.x, p.y + 1, scale * size, scale * size, 0.0f, color_modulation);
}

void Player::drawCollision(GameRenderer& renderer, const GameViewport& viewport, const ppl7::grafix::Point& worldcoords) const
{
    ppl7::grafix::PointF p(x - worldcoords.x, y - worldcoords.y);
    ppl7::grafix::Color nocol(255, 255, 255, 64);
    ppl7::grafix::Color white(255, 266, 255, 255);

    int frame = animation.getFrame();
    renderer.addBoundingBox(*sprite_resource, frame, p.x, p.y + 1, scale * parallax_scale, scale * parallax_scale, 0.0f,
                            ppl7::grafix::Color(255, 192, 0, 255));

    ppl7::grafix::Rect box = sprite_resource->spriteBoundary(frame, scale * parallax_scale, scale * parallax_scale, 0.0f, p.x, p.y);
    if (world_collision.left) {
        renderer.addLine(box.x1, box.y1, box.x1, box.y2, ppl7::grafix::Color(255, 0, 0, 255), 8);
    }
    if (world_collision.right) {
        renderer.addLine(box.x2, box.y1, box.x2, box.y2, ppl7::grafix::Color(255, 0, 0, 255), 8);
    }
    if (world_collision.top) {
        renderer.addLine(box.x1, box.y1, box.x2, box.y1, ppl7::grafix::Color(255, 0, 0, 255), 8);
    }
    if (world_collision.bottom) {
        renderer.addLine(box.x1, box.y2, box.x2, box.y2, ppl7::grafix::Color(255, 0, 0, 255), 8);
    }
    int half_tile_width = TILE_WIDTH / 2;

    renderer.addSprite(*tiletype_resource, world_collision.leftPivotTile, p.x - TILE_WIDTH, p.y);
    renderer.addSprite(*tiletype_resource, world_collision.rightPivotTile, p.x, p.y);

    renderer.addSprite(*tiletype_resource, world_collision.leftGroundTile, p.x - TILE_WIDTH, p.y + TILE_HEIGHT);
    renderer.addSprite(*tiletype_resource, world_collision.rightGroundTile, p.x, p.y + TILE_HEIGHT);

    // renderer.addSprite(*tiletype_resource, world_collision.middlePivotTile, p.x - half_tile_width, p.y);
    // renderer.addSprite(*tiletype_resource, world_collision.middleGroundTile, p.x - half_tile_width, p.y + TILE_HEIGHT);
}

void Player::turn(PlayerOrientation target)
{
    movement = Turn;
    turnTarget = target;
    if (orientation == Front) {
        if (target == Left) {
            animation.startSequence(4, 5, false, 6);
        } else {
            animation.startSequence(2, 1, false, 0);
        }
    } else if (orientation == Left) {
        if (target == Right) {
            animation.startSequence(5, 1, false, 0);
        } else if (target == Front) {
            animation.startSequence(5, 4, false, 3);
        }
    } else if (orientation == Right) {
        if (target == Left) {
            animation.startSequence(1, 5, false, 6);
        } else if (target == Front) {
            animation.startSequence(1, 2, false, 3);
        }
    }
}

void Player::stand()
{
    movement = Stand;
    if (orientation == Left)
        animation.setStaticFrame(6);
    else if (orientation == Right)
        animation.setStaticFrame(0);
    else if (orientation == Front)
        animation.setStaticFrame(3);
    else if (orientation == Back)
        animation.setStaticFrame(9);
    idle_timeout = time + 4.0;
    startIdle = idle_timeout;
}

void Player::addPoints(int points)
{
    if (movement == Dead) return;
    this->points += points;
}

void Player::addHealth(int points)
{
    if (movement == Dead) return;
    this->health += points;
    if (health > 100) health = 100;
}

void Player::addLife(int lifes)
{
    if (movement == Dead) return;
    this->lifes += lifes;
}

void Player::countObject(Objects::Type type)
{
    object_counter[static_cast<int>(type)]++;
}

size_t Player::getObjectCount(int type) const
{
    std::map<int, size_t>::const_iterator it;
    it = object_counter.find(type);
    if (it != object_counter.end()) return (*it).second;
    return 0;
}

void Player::dropHealth(float points, HealthDropReason reason)
{

    if (movement == Dead) return;
    if (points == 0.0f) return;
    if (godmode) return;
    if (reason == HealthDropReason::FallingDeep && game->config.difficulty < Config::DifficultyLevel::normal) return;
    if (game->config.difficulty == Config::DifficultyLevel::easy)
        points *= 0.5f;
    else if (game->config.difficulty == Config::DifficultyLevel::hard)
        points *= 2.0f;

    if (orientation == Front && movement == Stand && points > 0.0f) {
        if (animation.getFrame() != 297) animation.setStaticFrame(297);
    }

    // game->controller.rumbleTrigger(0xffff, 0xffff, 16);
    if (game->config.controller.use_rumble) game->controller.rumble(0xffff, 0xffff, 100);
    health -= (points * frame_rate_compensation);
    if (health > 100.0f) health = 100.0f;
    if (health <= 0.0f && movement != Dead) {
        health = 0;
        movement = Dead;
        fallstart = 0.0f;
        // we can play different animations for different reasons
        if (reason == FallingDeep) {
            // TODO: animation.start(death_by_falling, sizeof(death_by_falling) / sizeof(int), false, 106);
        } else if (reason == Drowned) {
            animation.startSequence(260, 281, false, 281);
        } else if (reason == Smashed) {
            animation.startSequence(403, 408, false, 408);
            ppl7::grafix::Point p(x, y);
            getAudioPool().playOnce(AudioEffect::stamper_squish, p, 1600, 1.0f);
            // startEmittingParticles(time + 1.0f, ParticleReason::Smashed);
        } else if (reason == SmashedSideways) {
            animation.startSequence(409, 414, false, 414);
            ppl7::grafix::Point p(x, y);
            getAudioPool().playOnce(AudioEffect::stamper_squish, p, 1600, 1.0f);
            // startEmittingParticles(time + 1.0f, ParticleReason::Smashed);

        } else if (reason == Burned) {
            animation.startSequence(208, 216, false, 216);
            // startEmittingParticles(time + 1.0f, ParticleReason::Burning);

        } else {
            // TODO: animation.start(death_animation, sizeof(death_animation) / sizeof(int), false, 106);
        }
    } else if (health > 0.0f && movement != Dead && points > 0.0f) {
        // TODO: Voice
    }
}

void Player::addInventory(int object_id, const Objects::Representation& repr)
{
    Inventory.insert(std::pair<int, Objects::Representation>(object_id, repr));
}

bool Player::isInInventory(int object_id) const
{
    if (object_id > 0) {
        std::map<int, Objects::Representation>::const_iterator it;
        it = Inventory.find(object_id);
        if (it != Inventory.end()) return true;
    }
    return false;
}

bool Player::isDead() const
{
    if (movement == Dead) return true;
    return dead;
}

void Player::setSavePoint(const ppl7::grafix::Point& p)
{
    lastSavePoint = p;
}

void Player::setStandingOnObject(Objects::Object* object)
{
    player_stands_on_object = object;
}

void Player::dropLifeAndResetToLastSavePoint()
{
    dead = false;
    lifes--;
    health = 100.0f;
    x = lastSavePoint.x;
    y = lastSavePoint.y;
    color_modulation.setColor(255, 255, 255, 255);
    stand();
}

static void play_step(AudioPool& ap)
{
    int r = ppl7::rand(1, 5);
    switch (r) {
    case 1:
        ap.playOnce(AudioEffect::player_step1, 0.5f);
        break;
    case 2:
        ap.playOnce(AudioEffect::player_step2, 0.5f);
        break;
    case 3:
        ap.playOnce(AudioEffect::player_step3, 0.5f);
        break;
    case 4:
        ap.playOnce(AudioEffect::player_step4, 0.5f);
        break;
    case 5:
        ap.playOnce(AudioEffect::player_step5, 0.5f);
        break;
    default:
        ap.playOnce(AudioEffect::player_step1, 0.5f);
        break;
    }
}

static void play_ladder(AudioPool& ap)
{
    int r = ppl7::rand(1, 7);
    switch (r) {
    case 1:
        ap.playOnce(AudioEffect::ladder_step1, 0.5f);
        break;
    case 2:
        ap.playOnce(AudioEffect::ladder_step2, 0.5f);
        break;
    case 3:
        ap.playOnce(AudioEffect::ladder_step3, 0.5f);
        break;
    case 4:
        ap.playOnce(AudioEffect::ladder_step4, 0.5f);
        break;
    case 5:
        ap.playOnce(AudioEffect::ladder_step5, 0.5f);
        break;
    case 6:
        ap.playOnce(AudioEffect::ladder_step6, 0.5f);
        break;
    default:
        ap.playOnce(AudioEffect::ladder_step7, 0.5f);
        break;
    }
}

void Player::playSoundOnAnimationSprite()
{
    int sprite = animation.getFrame();
    if (sprite == last_animation_sound_played) return;
    last_animation_sound_played = sprite;
    AudioPool& ap = getAudioPool();

    // TODO
    if (sprite == 3 || sprite == 7 || sprite == 12 || sprite == 16 || sprite == 64 || sprite == 68 || sprite == 73 || sprite == 77)
        play_step(ap);
    if (movement == ClimbUp && (sprite == 91 || sprite == 96)) play_ladder(ap);
    if (movement == ClimbDown && (sprite == 101 || sprite == 96)) play_ladder(ap);
}

void Player::update(const GameClock& clock, ParallaxLayer& layer)
{
    time = clock.time;
    frame_rate_compensation = clock.frame_rate_compensation;
    if (animation.update(clock.time)) {
        // Gibt's hier was zu tun?
    }
    playSoundOnAnimationSprite();
    keys.update(clock.time);

    if (movement == Dead) {
        if (animation.isFinished()) {
            dead = true;
        }
        return;
    }
    if (dead) return;

    AudioPool& ap = getAudioPool();
    if (movement == Jump || movement == Falling || movement == Slide) {
        if (airStart == 0) {
            airStart = time;
            // if (ppl7::rand(0, 2) == 0) speak(static_cast<VoiceGeorge::Id>(ppl7::rand(0, 4) + static_cast<int>(VoiceGeorge::hepp1)),
            // 0.1f);
        }
    } else if (airStart > 0.0f) {
        double volume = (time - airStart);
        if (volume > 1.0f) volume = 1.0f;
        airStart = 0.0f;
        ap.playOnce(AudioEffect::player_jump_land, volume);
    }

    player_stands_on_object = NULL;
    checkCollisionWithObjects(clock, layer.objects);

    if (movement == Hacking) return;
    if (movement == Dead) return;

    int frame = animation.getFrame();

    world_collision =
        GetWorldCollision(clock, layer.TileTypeMatrix, x, y, sprite_resource, animation.getFrame(), scale * parallax_scale, 0.0f, false, 0);
    Physic::PlayerMovement new_movement = checkCollisionWithWorld(clock, layer.TileTypeMatrix);
    if (new_movement == Stand) stand();

    // PlayerMovement last_movement=movement;
    if (updatePhysics(clock)) {
    }

    x += (velocity_move.x * clock.frame_rate_compensation);
    y += (velocity_move.y * clock.frame_rate_compensation);

    if (!controlEnabled) return;

    if (movement == Turn) {
        if (!animation.isFinished()) return;
        // printf ("debug 2\n");
        movement = Stand;
        orientation = turnTarget;
        startIdle = time;
        velocity_move.clear();
    }
    if (movement == Slide || movement == Dead) {
        return;
    }

    if (movement == Jump || movement == Falling) {
        handleKeyboardWhileJumpOrFalling(clock, layer.TileTypeMatrix, layer.objects);
        return;
    } else {
        handleKeyboard(clock, layer.TileTypeMatrix, layer.objects);
    }

    if (keys.action) {
        checkActivationOfObjectsInRange(layer.objects);
        if (movement == Hacking) return;
    }
}
void Player::handleKeyboard(const GameClock& clock, const TileTypePlane& world, ObjectSystem& objects)
{
    if (keys.jump) {
        if (movement != Jump) {
            movement = Jump;
            if (orientation == Left) {
                animation.start(JumpLeftUp);
            } else if (orientation == Right) {
                animation.start(JumpRightUp);
            } else if (orientation == Front) {
                animation.start(JumpUp);
            } else if (orientation == Back) {
                animation.start(JumpUp);
            }
        }
    } else if (keys.left) {
        if (orientation == Right) {
            turn(Left);
        } else {
            velocity_move.x = -max_run_speed;
            orientation = Left;
            if (movement != Run) {
                animation.start(RunCycleLeft);
                movement = Run;
            }
        }
    } else if (keys.right) {
        if (orientation == Left) {
            turn(Right);
        } else {
            velocity_move.x = max_run_speed;
            orientation = Right;
            if (movement != Run) {
                animation.start(RunCycleRight);
                movement = Run;
            }
        }

    } else {
        velocity_move.x = 0;
        if (movement == Run) stand();
    }
}

void Player::handleKeyboardWhileJumpOrFalling(const GameClock& clock, const TileTypePlane& world, ObjectSystem& objects)
{
    /* TODO:
    if (movement == Jump) {
        if (!(keys.jump)) {
            movement = Falling;
            return;
        }
        if ((keys.left) && velocity_move.x == 0) {
            if (acceleration_jump_sideways > -6.0f) acceleration_jump_sideways = -6.0f;
            velocity_move.x = acceleration_jump_sideways * frame_rate_compensation;
            if (acceleration_jump_sideways > -9.0f) acceleration_jump_sideways -= (0.2f * frame_rate_compensation);
            orientation = Left;
            animation.setStaticFrame(84);

        } else if ((keys.right) && velocity_move.x == 0) {
            if (acceleration_jump_sideways < 6.0f) acceleration_jump_sideways = 6.0f;
            velocity_move.x = acceleration_jump_sideways * frame_rate_compensation;
            if (acceleration_jump_sideways < 9.0f) acceleration_jump_sideways += (0.2f * frame_rate_compensation);
            orientation = Right;
            animation.setStaticFrame(65);
        }
    } else {
        if (keys.left) {
            if (acceleration_jump_sideways > -6.0f) acceleration_jump_sideways = -6.0f;
            velocity_move.x = acceleration_jump_sideways * frame_rate_compensation;
            if (acceleration_jump_sideways > -9.0f) acceleration_jump_sideways -= (0.2f * frame_rate_compensation);
            orientation = Left;
            animation.setStaticFrame(84);
        } else if (keys.right) {
            if (acceleration_jump_sideways < 6.0f) acceleration_jump_sideways = 6.0f;
            velocity_move.x = acceleration_jump_sideways * frame_rate_compensation;
            if (acceleration_jump_sideways < 9.0f) acceleration_jump_sideways += (0.2f * frame_rate_compensation);
            orientation = Right;
            animation.setStaticFrame(65);
        }
    }

    // ppl7::PrintDebugTime("acceleration_jump_sideways=%0.3f\n", acceleration_jump_sideways);
    */
}

Physic::PlayerMovement Player::checkCollisionWithWorld(const GameClock& clock, const TileTypePlane& world)
{
    Physic::PlayerMovement new_movement = Unchanged;

    // Physic::PlayerMovement new_movement = Physic::checkCollisionWithWorld(world, x, y);
    if (movement == Dead) return new_movement;
checkagain:
    if (world_collision.leftPivotTile == TileType::Type::Blocking || world_collision.rightPivotTile == TileType::Type::Blocking) {
        velocity_move.y = 0;
        y = (((int)y / TILE_HEIGHT) * TILE_HEIGHT) - 1;
        world_collision.update(x, y);
        // while (world_collision.leftPivotTile == TileType::Type::Blocking || world_collision.rightPivotTile == TileType::Type::Blocking) {
        //     y--;
        //     world_collision.update(x, y);
        // }
    }
    if (world_collision.leftPivotTile == TileType::Type::TwoThirdBlockLower ||
        world_collision.rightPivotTile == TileType::Type::TwoThirdBlockLower) {
        velocity_move.y = 0;
        y = (((int)y / TILE_HEIGHT) * TILE_HEIGHT) + (TILE_HEIGHT / 3) - 1;
        world_collision.update(x, y);
    } else if (world_collision.leftPivotTile == TileType::Type::ThirdBlockLower ||
               world_collision.rightPivotTile == TileType::Type::ThirdBlockLower) {
        velocity_move.y = 0;
        y = (((int)y / TILE_HEIGHT) * TILE_HEIGHT) + 2 * (TILE_HEIGHT / 3) - 1;
        world_collision.update(x, y);
    }

    return new_movement;
    /*
    if (collision_type_count[TileType::Type::Speer] > 0) {
        if (checkCollisionMatrixBody(TileType::Type::Speer)) this->dropHealth(10);
    }
    if (collision_type_count[TileType::Type::Fire] > 0) {
        if (checkCollisionMatrixBody(TileType::Type::Fire)) this->dropHealth(10, Burned);
    }
        */
}

bool Player::updatePhysics(const GameClock& clock)
{
    bool movement_changed = false;
    if (velocity_move.y < gravity) {
        if (!world_collision.bottom) {
            velocity_move.y += gravity * clock.frame_rate_compensation;
            if (velocity_move.y > gravity) velocity_move.y = gravity;
            if (velocity_move.y > 0) {
                if (movement != Jump && movement != Falling) {
                    movement = Falling;
                    return true;
                }
            }
        } else {
            velocity_move.y = 0;
            if (movement == Jump || movement == Falling) {
                movement = Stand;
                return true;
            }
        }
    }
    return movement_changed;
}

void Player::checkCollisionWithObjects(const GameClock& clock, ObjectSystem& objects)
{
    // we try to find existing pixels inside the player boundary
    // to build a list with points we want to check against the
    // objects
    if (movement == Dead) return;
    collision_checkpoints.clear();
    collision_checkpoints.push_back(ppl7::grafix::Point(x, y));

    const ppl7::grafix::Drawable& draw = sprite_resource->getDrawable(animation.getFrame());
    ppl7::grafix::Rect boundary = sprite_resource->spriteBoundary(animation.getFrame(), 1.0f, x, y);

    if (draw.width()) {
        // ppl7::PrintDebugTime("boundary= %d:%d - %d:%d\n", boundary.x1, boundary.y1, boundary.x2, boundary.y2);
        int stepx = boundary.width() / 16;
        int stepy = boundary.height() / 16;
        for (int py = boundary.y1; py < boundary.y2; py += stepx) {
            for (int px = boundary.x1; px < boundary.x2; px += stepy) {
                ppl7::grafix::Color c = draw.getPixel(px - boundary.x1, py - boundary.y1);
                if (c.alpha() > 92) {
                    collision_checkpoints.push_back(ppl7::grafix::Point(px, py));
                }
            }
        }
    }
    // printf ("check collision against %zd points:\n", checkpoints.size());

    // Decker::Objects::Object* object=objects->detectCollision(checkpoints);
    std::list<Objects::Object*> object_list;
    objects.detectCollision(collision_checkpoints, object_list);
    if (object_list.empty()) return;
    std::list<Objects::Object*>::iterator it;
    for (it = object_list.begin(); it != object_list.end(); ++it) {
        Objects::Collision col(clock, this, (*it));
        col.detect((*it), collision_checkpoints, *this);
        (*it)->handleCollision(this, col);
    }

    // printf ("Detected Collision with Object: %s, ID: %d\n",
    //		(const char*)object->typeName(), object->id);
    // const ppl7::grafix::Rect &bbi=col.bounding_box_intersection;
    // printf ("BoundingBox Player: %d:%d - %d:%d\n", bbp.x1, bbp.y1, bbp.x2, bbp.y2);
    // printf ("BoundingBox Object: %d:%d - %d:%d\n", bbo.x1, bbo.y1, bbo.x2, bbo.y2);
    // printf ("Intersection:       %d:%d - %d:%d\n", bbi.x1, bbi.y1, bbi.x2, bbi.y2);
}

const std::list<ppl7::grafix::Point>& Player::getCollisionCheckpoints() const
{
    return collision_checkpoints;
}

ppl7::grafix::Rect Player::getBoundingBox() const
{
    return sprite_resource->spriteBoundary(animation.getFrame(), scale * parallax_scale, x, y);
}

void Player::addSpecialObject(int type)
{
    SpecialObjects.insert(type);
}

bool Player::hasSpecialObject(int type) const
{
    std::set<int>::const_iterator it;
    it = SpecialObjects.find(type);
    if (it != SpecialObjects.end()) return true;
    return false;
}

void Player::checkActivationOfObjectsInRange(ObjectSystem& objectsystem)
{
    std::list<Objects::Object*> object_list;

    if (objectsystem.findObjectsInRange(position(), 200, object_list)) {
        // ppl7::PrintDebugTime("Found %zd objects in range\n", object_list.size());
        std::list<Objects::Object*>::iterator it;
        for (it = object_list.begin(); it != object_list.end(); ++it) {
            /*
            if ((*it)->type() == Objects::Type::BreakingWall) {
                double dist = ppl7::grafix::Distance((*it)->p, position());
                if (dist < 100) startHacking((*it));
            }
                */
        }
    }
}

void Player::takeAllItems(Objects::Type type)
{
    if (type == Objects::Type::ExtraLife) {
        lifes = 1;
        //} else if (type == Objects::Type::PowerCell) {
        //    powercells = 0;
    } else {
        SpecialObjects.erase(static_cast<int>(type));
    }
}

void Player::enableControl()
{
    controlEnabled = true;
    airStart = 0.0f;
    animation.resetSpeed();
    stand();
}

void Player::disableControl()
{
    stand();
    controlEnabled = false;
}
