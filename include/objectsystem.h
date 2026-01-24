#ifndef INCLUDE_OBJECTS_H_
#define INCLUDE_OBJECTS_H_

#include <map>
#include <list>
#include <ppl7.h>
#include <ppl7-grafix.h>
#include "gpu.h"

#include "animation.h"
// #include "particle.h"
//  #include "light.h"

class SpriteTexture;
// class LightObject;

class Player;
class TileTypePlane;
class Waynet;

namespace ppl7::tk
{
class Widget;
}

class AudioInstance;
class Glimmer;

namespace Objects
{

class Type
{
public:
    // ID must be <256
    enum ObjectType
    {
        PlayerStartpoint = 1,
        Savepoint = 2,
        Medikit = 3,
        Crystal = 4,
        Diamond = 5,
        Coin = 6,
        Speaker = 7,
        ExtraLife = 8,
        ParticleEmitter = 9,
        Projectile = 10,
        SpawnPoint = 11,
    };
    static ppl7::String name(Type::ObjectType type);
};

class Spriteset
{
public:
    enum SpritesetIds
    {
        GenericObjects = 0,
        ThreeSpeers,
        Skeleton,
        Mummy,
        Vent,
        Mushroom,
        TreasureChest,
        Doors,
        Scarabeus,
        Laser,
        Wallenstein,
        Helena,
        Bat,
        Scorpion,
        Bird,
        LevelEnd,
        Yeti,
        George,
        Ostrich,
        ScorpionMetalic,
        Piranha,
        BreakingWall,
        Rat,
        Ghost,
        Zombie,
        StamperV2,
        Skull,
        SkullMaster,
        Switches,
        Crates,
        Spider,
        MagicGround,
        GreatElevator,
        MaxSpritesets
    };
};

class Representation
{
public:
    Representation(int sprite_set, int sprite_no);
    int sprite_set;
    int sprite_no;
};

Representation getRepresentation(int object_type);

class Object;
class Collision
{
private:
    Object* object;
    std::list<ppl7::grafix::Point> collision_points;

public:
    ppl7::grafix::Rect bounding_box_object;
    ppl7::grafix::Rect bounding_box_player;
    ppl7::grafix::Rect bounding_box_intersection;
    float frame_rate_compensation;
    Collision();
    Collision(const Collision& other);
    Collision(const Player* player, const Object* object, float frame_rate_compensation);
    void detect(Object* object, const std::list<ppl7::grafix::Point>& checkpoints, const Player& player);
    const std::list<ppl7::grafix::Point>& getCollisionPoints() const;
    Object* getObject() const;
    bool onFoot() const;

    bool objectTop() const;
    bool objectBottom() const;
    bool objectLeft() const;
    bool objectRight() const;

    bool objectTop(int t) const;
    bool objectBottom(int t) const;
    bool objectLeft(int t) const;
    bool objectRight(int t) const;

    bool playerTop(int t) const;
    bool playerBottom(int t) const;
    bool playerLeft(int t) const;
    bool playerRight(int t) const;
};

class ObjectCollision
{
private:
    const Object* this_object;
    const Object* other_object;

public:
    ppl7::grafix::Rect bounding_box_this_object;
    ppl7::grafix::Rect bounding_box_other_object;
    ppl7::grafix::Rect bounding_box_intersection;

    ObjectCollision(const Object* this_object, const Object* other_object);
    void update();
    bool objectTop(int tolerance = 1) const;
    bool objectBottom(int tolerance = 1) const;
    bool objectLeft(int tolerance = 1) const;
    bool objectRight(int tolerance = 1) const;
};

enum class ParallaxLayerId;

class Object
{
    friend class ObjectSystem;

public:
    enum class Layer
    {
        BehindBricks = 0,
        BeforeBricks = 1,
        BeforePlayer = 2,
        BehindPlayer = 1,
    };

private:
    Type::ObjectType myType;

public:
    Layer myLayer;
    ParallaxLayerId myPlane;

    ppl7::grafix::PointF p;
    ppl7::grafix::PointF initial_p;
    SpriteTexture* texture;
    ppl7::grafix::Rect boundary, initial_boundary;
    uint32_t id;
    ppl7::grafix::Color color_mod;
    int sprite_set;
    int sprite_no;
    int sprite_no_representation;
    uint8_t difficulty_matrix;
    float scale;
    float rotation;
    bool collisionDetection;
    bool visibleAtPlaytime;
    bool enabled;
    bool pixelExactCollision;
    bool spawned; // not saved, deleted on collection
    bool deleteDefered;
    bool alwaysUpdate;
    bool isInViewport; // not saved, indicates, if the object gets drawn in the current frame

    explicit Object(Type::ObjectType type);
    virtual ~Object();
    Type::ObjectType type() const;
    ppl7::String typeName() const;
    void updateBoundary();
    void updateSpriteset(int spriteset);
    virtual void update(double time, TileTypePlane& ttplane, Player& player, float frame_rate_compensation);
    virtual size_t save(unsigned char* buffer, size_t size) const;
    virtual size_t load(const unsigned char* buffer, size_t size);
    virtual size_t saveSize() const;
    virtual void handleCollision(Player* player, const Collision& collision);
    // virtual void draw(SDL_Renderer* renderer, const ppl7::grafix::Point& coords) const;
    // virtual void drawEditMode(SDL_Renderer* renderer, const ppl7::grafix::Point& coords) const;
    virtual void openUi();
    virtual void reset();
    virtual void toggle(bool enable, Object* source = NULL);
    virtual void trigger(Object* source = NULL);
    virtual bool isEnabled() const;
    static Representation representation();
};

class ObjectSystem
{
private:
    uint32_t nextid;
    uint32_t next_spawn_id;
    int player_start;
    std::map<uint32_t, Object*> object_list;
    std::map<uint64_t, Object*> visible_object_map;
    SpriteTexture* spriteset[Spriteset::MaxSpritesets];
    SpriteTexture* light_objects;
    Waynet* waynet;

    // void updateVisibleObjectsForPlane(PlaneId plane, const ppl7::grafix::Point& worldcoords, const ppl7::grafix::Rect& viewport);
    // Object* findMatchingObjectOnPlane(PlaneId plane, const ppl7::grafix::Point& p) const;

public:
    ObjectSystem();
    ~ObjectSystem();
    void clear();
    void setWaynet(Waynet* waynet);
    void setSpritesetResources(); // TODO
    // void loadSpritesets(SDL& sdl);   // TODO: Wir brauchen globale Spritesets, die jedem ObjectSystem zur Verfügung stehen
    void addObject(Object* object);
    Object* getInstance(int object_type) const;
    void update(double time, TileTypePlane& ttplane, Player& player, float frame_rate_compensation);
    void updateVisibleObjectList(const ppl7::grafix::Point& worldcoords, const ppl7::grafix::Rect& viewport);
    void draw(GPUBatcher& batcher, const ppl7::grafix::Rect& viewport, const ppl7::grafix::Point& worldcoords, Object::Layer layer) const;
    void drawEditMode(GPUBatcher& batcher,
                      const ppl7::grafix::Rect& viewport,
                      const ppl7::grafix::Point& worldcoords,
                      Object::Layer layer) const;
    void save(ppl7::FileObject& file, unsigned char chunkid, unsigned char layer) const;
    void load(const ppl7::ByteArrayPtr& ba);
    Object* getObject(uint32_t object_id);
    Object* findMatchingObject(const ppl7::grafix::Point& worldcoords, const ppl7::grafix::Point& p) const;
    void detectCollision(const std::list<ppl7::grafix::Point>& checkpoints, std::list<Object*>& object_list);
    static bool checkCollisionWithObject(const std::list<ppl7::grafix::Point>& checkpoints, const Object* object);
    void detectObjectCollision(const Object* object, std::list<Object*>& collision_object_list);
    void detectObjectCollision(const ppl7::grafix::Rect& boundary, std::list<Object*>& collision_object_list);

    void drawSelectedSpriteOutline(GPUBatcher& batcher, const ppl7::grafix::Rect& viewport, const ppl7::grafix::Point& worldcoords, int id);
    void drawPlaceSelection(GPUBatcher& batcher, const ppl7::grafix::Point& p, int object_type);
    void deleteObject(int id);
    bool findObjectsInRange(const ppl7::grafix::PointF& p, double range, std::list<Object*>& objects);
    ppl7::grafix::Point findPlayerStart() const;
    ppl7::grafix::Point nextPlayerStart();
    SpriteTexture* getTexture(int sprite_set) const;
    void resetPlayerStart();
    size_t count() const;
    size_t countVisible() const;
    Waynet& getWaynet();
    void getObjectCounter(std::map<int, size_t>& object_counter) const;
};

ObjectSystem* GetObjectSystem();

} // namespace Objects

#endif // INCLUDE_OBJECTS_H_
