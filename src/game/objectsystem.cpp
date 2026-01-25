#include <ppl7.h>
#include <ppl7-grafix.h>
#include "objectsystem.h"
#include "player.h"
#include "game.h"
#include "sprite.h"

static uint8_t getDifficultyMatrix()
{
    switch (GetGame().config.difficulty) {
    case Config::DifficultyLevel::easy:
        return 1;
    case Config::DifficultyLevel::normal:
        return 2;
    case Config::DifficultyLevel::hard:
        return 4;
    }
    return 2;
}

ObjectSystem::ObjectSystem()
{
    nextid = 1;
    next_spawn_id = 1000000;
    player_start = 0;
    spritesets = NULL;
}

ObjectSystem::~ObjectSystem()
{
    clear();
}

void ObjectSystem::setSpritesetResources(ObjectSpritesets& spritesets)
{
    this->spritesets = &spritesets;
}

void ObjectSystem::clear()
{
    visible_object_map.clear();

    for (auto it = object_list.begin(); it != object_list.end(); ++it) {
        delete it->second;
    }
    object_list.clear();
    nextid = 1;
    next_spawn_id = 1000000;
    player_start = 0;
}

void ObjectSystem::addObject(Objects::Object* object)
{
    if (!object) return;
    if (object->spawned) {
        object->id = next_spawn_id;
        next_spawn_id++;
    } else {
        object->id = nextid;
        nextid++;
    }
    if (spritesets) {
        object->texture = spritesets->getSpriteset(object->sprite_set);
        object->updateBoundary();
    }
    object_list.insert(std::pair<uint32_t, Objects::Object*>(object->id, object));
}

void ObjectSystem::deleteObject(int id)
{
    std::map<uint32_t, Objects::Object*>::const_iterator it;
    it = object_list.find(id);
    if (it != object_list.end()) {
        Objects::Object* object = it->second;
        object_list.erase(it);
        delete object;
    }
}

void ObjectSystem::updateVisibleObjectList(const ppl7::grafix::Point& worldcoords, const ppl7::grafix::Rect& viewport)
{
    visible_object_map.clear();
    std::map<uint32_t, Objects::Object*>::iterator it;
    std::list<uint32_t> deleteme;
    int width = viewport.width();
    int height = viewport.height();

    for (it = object_list.begin(); it != object_list.end(); ++it) {
        Objects::Object* object = it->second;
        object->isInViewport = false;
        if (object->deleteDefered) {
            deleteme.push_back(it->first);
        } else if (object->texture) {
            int x = object->p.x - worldcoords.x;
            int y = object->p.y - worldcoords.y;
            bool isVisible = false;
            if (x + object->boundary.width() > 0 && y + object->boundary.height() > 0 && x - object->boundary.width() < width &&
                y - object->boundary.height() < height)
                isVisible = true;
            if (object->p != object->initial_p) {
                x = object->initial_p.x - worldcoords.x;
                y = object->initial_p.y - worldcoords.y;
                if (x > 0 && y > 0 && x < width && y < height) isVisible = true;
            }
            if (isVisible) {
                object->isInViewport = true;
                uint64_t id =
                    (((uint64_t)object->p.y & 0xffff) << 48) | (uint64_t)(((uint64_t)object->p.x & 0xffff) << 32) | (uint64_t)object->id;
                visible_object_map.insert(std::pair<uint32_t, Objects::Object*>(id, object));
            }
        }
    }
    if (deleteme.size() > 0) {
        std::list<uint32_t>::iterator it;
        for (it = deleteme.begin(); it != deleteme.end(); ++it) {
            deleteObject(*it);
        }
    }
}

void ObjectSystem::update(double time, TileTypePlane& ttplane, Player& player, float frame_rate_compensation)
{
    std::map<uint32_t, Objects::Object*>::iterator it;
    uint8_t dm = getDifficultyMatrix();
    for (it = object_list.begin(); it != object_list.end(); ++it) {
        Objects::Object* object = it->second;
        if (object->difficulty_matrix & dm) {
            if (object->alwaysUpdate || object->isInViewport) object->update(time, ttplane, player, frame_rate_compensation);
        }
    }
}

void ObjectSystem::draw(GPUBatcher& batcher,
                        const ppl7::grafix::Rect& viewport,
                        const ppl7::grafix::Point& worldcoords,
                        Objects::Object::Layer layer) const
{
    std::map<uint64_t, Objects::Object*>::const_iterator it;
    ppl7::grafix::Point coords(viewport.x1 - worldcoords.x, viewport.y1 - worldcoords.y);
    uint8_t dm = getDifficultyMatrix();
    for (it = visible_object_map.begin(); it != visible_object_map.end(); ++it) {
        const Objects::Object* object = it->second;
        if (object->texture != NULL && object->enabled == true && object->visibleAtPlaytime == true && object->myLayer == layer &&
            (object->difficulty_matrix & dm)) {
            object->draw(batcher, coords);
        }
    }
}

static void drawId(GPUBatcher& batcher, SpriteTexture* spriteset, int x, int y, uint32_t as)
{
    ppl7::WideString s;
    s.setf("%d", as);
    int w = (int)s.size() * 12;
    x -= w / 2;
    for (size_t p = 0; p < s.size(); p++) {
        int num = s[p];
        batcher.addSprite(*spriteset, num, x, y, 0.5f, 0.5f);
        x += 12;
    }
}

void ObjectSystem::drawEditMode(GPUBatcher& batcher,
                                const ppl7::grafix::Rect& viewport,
                                const ppl7::grafix::Point& worldcoords,
                                Objects::Object::Layer layer) const
{
    std::map<uint64_t, Objects::Object*>::const_iterator it;
    ppl7::grafix::Point coords(viewport.x1 - worldcoords.x, viewport.y1 - worldcoords.y);
    for (it = visible_object_map.begin(); it != visible_object_map.end(); ++it) {
        const Objects::Object* object = it->second;
        if (object->type() == Objects::Type::Projectile) {
            if (object->texture != NULL && object->myLayer == layer) {
                object->draw(batcher, coords);
            }
        } else {
            if (object->texture != NULL && object->myLayer == layer) {
                object->drawEditMode(batcher, coords);
                drawId(batcher, spritesets->fonts, object->p.x + coords.x, object->p.y + coords.y, object->id);
                if (object->p != object->initial_p && object->spawned == false)
                    drawId(batcher, spritesets->fonts, object->initial_p.x + coords.x, object->initial_p.y + coords.y, object->id);
            }
        }
    }
}

Objects::Object* ObjectSystem::findMatchingObject(const ppl7::grafix::Point& worldcoords, const ppl7::grafix::Point& p) const
{
    Objects::Object* found_object = NULL;
    std::map<uint64_t, Objects::Object*>::const_iterator it;
    for (it = visible_object_map.begin(); it != visible_object_map.end(); ++it) {
        Objects::Object* item = it->second;
        if (p.inside(item->initial_boundary) == true && item->spawned == false) {
            if (item->texture) {
                const ppl7::grafix::Drawable draw = item->texture->getDrawable(item->sprite_no_representation);
                if (draw.width()) {
                    int x = p.x - item->initial_boundary.x1;
                    int y = p.y - item->initial_boundary.y1;
                    ppl7::grafix::Color c = draw.getPixel(x, y);
                    if (c.alpha() > 92) {
                        found_object = item;
                    }
                }
            }
        }
    }
    return found_object;
}

bool ObjectSystem::checkCollisionWithObject(const std::list<ppl7::grafix::Point>& checkpoints, const Objects::Object* object)
{
    std::list<ppl7::grafix::Point>::const_iterator p_it;
    for (p_it = checkpoints.begin(); p_it != checkpoints.end(); ++p_it) {
        if ((*p_it).inside(object->boundary)) {
            // printf ("inside boundary\n");
            if (object->pixelExactCollision == false)
                return true;
            else {
                const ppl7::grafix::Drawable draw = object->texture->getDrawable(object->sprite_no);
                if (draw.width()) {
                    int x = (*p_it).x - object->boundary.x1;
                    int y = (*p_it).y - object->boundary.y1;
                    ppl7::grafix::Color c = draw.getPixel(x, y);
                    if (c.alpha() > 92) return true;
                }
            }
        }
    }
    return false;
}

void ObjectSystem::detectCollision(const std::list<ppl7::grafix::Point>& checkpoints, std::list<Objects::Object*>& object_list)
{
    std::map<uint64_t, Objects::Object*>::const_iterator it;
    std::list<ppl7::grafix::Point>::const_iterator p_it;
    uint8_t dm = getDifficultyMatrix();
    for (it = visible_object_map.begin(); it != visible_object_map.end(); ++it) {
        Objects::Object* item = it->second;
        if (item->texture != NULL && item->collisionDetection == true && item->enabled == true && (item->difficulty_matrix & dm)) {
            if (ObjectSystem::checkCollisionWithObject(checkpoints, item)) object_list.push_back(item);
        }
    }
}

Objects::Object* ObjectSystem::getObject(uint32_t object_id)
{
    std::map<uint32_t, Objects::Object*>::iterator it;
    it = object_list.find(object_id);
    if (it != object_list.end()) return it->second;
    return NULL;
}

void ObjectSystem::drawSelectedSpriteOutline(GPUBatcher& batcher,
                                             const ppl7::grafix::Rect& viewport,
                                             const ppl7::grafix::Point& worldcoords,
                                             int id)
{
    std::map<uint32_t, Objects::Object*>::const_iterator it;
    it = object_list.find(id);
    if (it != object_list.end()) {
        const Objects::Object* item = it->second;
        if (item->texture) {
            item->texture->drawOutlines(batcher, item->initial_p.x + viewport.x1 - worldcoords.x,
                                        item->initial_p.y + viewport.y1 - worldcoords.y, item->sprite_no_representation, item->scale);
        }
    }
}

SpriteTexture* ObjectSystem::getTexture(int sprite_set) const
{
    return spritesets->getSpriteset(sprite_set);
}

void ObjectSystem::drawPlaceSelection(GPUBatcher& batcher, const ppl7::grafix::Point& p, int object_type)
{
    Objects::Representation repr = Objects::getRepresentation(object_type);
    if (repr.sprite_set >= 0) {
        SpriteTexture* texture = getTexture(repr.sprite_set);
        if (texture) {
            texture->draw(batcher, p.x, p.y, repr.sprite_no);
            texture->drawOutlines(batcher, p.x, p.y, repr.sprite_no, 1.0f);
        }
    }
}

Objects::Object* ObjectSystem::getInstance(int object_type) const
{
    switch (object_type) {
    case Objects::Type::Coin:
        // return new CoinReward();
        return NULL;
    }
    return NULL;
}

void ObjectSystem::save(ppl7::FileObject& file, unsigned char chunkid, unsigned char layer) const
{
    if (object_list.size() == 0) return;
    std::map<uint32_t, Objects::Object*>::const_iterator it;
    size_t buffersize = 0;
    for (it = object_list.begin(); it != object_list.end(); ++it) {
        Objects::Object* object = it->second;
        if (!object->spawned) buffersize += object->saveSize() + 4;
    }
    unsigned char* buffer = (unsigned char*)malloc(buffersize + 5);
    ppl7::Poke32(buffer + 0, 0);
    ppl7::Poke8(buffer + 4, chunkid);
    ppl7::Poke8(buffer + 5, layer);
    ppl7::Poke8(buffer + 6, 1); // version
    size_t p = 7;
    for (it = object_list.begin(); it != object_list.end(); ++it) {
        Objects::Object* object = it->second;
        if (!object->spawned) {
            size_t object_size = object->saveSize();
            ppl7::Poke32(buffer + p, object_size + 4);
            size_t bytes_saved = object->save(buffer + p + 4, object_size);
            if (bytes_saved == object_size && bytes_saved > 0) {
                p += object_size + 4;
            }
        }
    }
    ppl7::Poke32(buffer + 0, p);
    file.write(buffer, p);
    free(buffer);
}

void ObjectSystem::load(const ppl7::ByteArrayPtr& ba)
{
    clear();
    size_t p = 2;
    const unsigned char* buffer = (const unsigned char*)ba.toCharPtr();
    int version = ppl7::Peek8(buffer + 1);
    // printf("chunk size=%zd\n",ba.size());
    while (p < ba.size()) {
        int save_size = ppl7::Peek32(buffer + p);
        int type = ppl7::Peek16(buffer + p + 5);
        Objects::Object* object = getInstance(type);
        if (object) {
            if (object->load(buffer + p + 4, save_size - 4)) {
                if (object->id >= nextid) nextid = object->id + 1;
                object->texture = spritesets->getSpriteset(object->sprite_set);
                object->updateBoundary();
                object_list.insert(std::pair<uint32_t, Objects::Object*>(object->id, object));
            } else {
                delete object;
            }
        }
        p += save_size;
    }
    // printf ("nextid=%d\n",nextid);
}

ppl7::grafix::Point ObjectSystem::findPlayerStart() const
{
    std::map<uint32_t, Objects::Object*>::const_iterator it;
    for (it = object_list.begin(); it != object_list.end(); ++it) {
        Objects::Object* object = it->second;
        if (object->type() == Objects::Type::PlayerStartpoint) return object->p;
    }
    return ppl7::grafix::Point(0, 0);
}

ppl7::grafix::Point ObjectSystem::nextPlayerStart()
{
    int c = 0;
    player_start++;
    std::map<uint32_t, Objects::Object*>::const_iterator it;
    for (it = object_list.begin(); it != object_list.end(); ++it) {
        Objects::Object* object = it->second;
        if (object->type() == Objects::Type::PlayerStartpoint) {
            if (c == player_start) {
                return object->p;
            }
            c++;
        }
    }
    player_start = 0;
    return findPlayerStart();
}

void ObjectSystem::resetPlayerStart()
{
    player_start = 0;
}

size_t ObjectSystem::count() const
{
    return object_list.size();
}

size_t ObjectSystem::countVisible() const
{
    return visible_object_map.size();
}

void ObjectSystem::getObjectCounter(std::map<int, size_t>& object_counter) const
{
    object_counter.clear();
    std::map<uint32_t, Objects::Object*>::const_iterator it;
    for (it = object_list.begin(); it != object_list.end(); ++it) {
        const Objects::Object* obj = (*it).second;
        if (obj) {
            object_counter[obj->type()]++;
            /*
            if (obj->type() == Objects::Type::ObjectType::TouchEmitter) {
                int t = ((const TouchEmitter*)obj)->emitted_object;
                object_counter[t] += ((const TouchEmitter*)obj)->max_toggles;
            } else if (obj->type() == Objects::Type::ObjectType::SpawnPoint) {
                int t = ((const SpawnPoint*)obj)->emitted_object;
                object_counter[t] += ((const SpawnPoint*)obj)->max_toggles;
            }
            */
        }
    }
}

bool ObjectSystem::findObjectsInRange(const ppl7::grafix::PointF& p, double range, std::list<Objects::Object*>& objects)
{
    objects.clear();
    std::map<uint64_t, Objects::Object*>::const_iterator it;
    for (it = visible_object_map.begin(); it != visible_object_map.end(); ++it) {
        double dist = ppl7::grafix::Distance((*it).second->p, p);
        if (dist < range) objects.push_back((*it).second);
    }
    if (objects.size() > 0) return true;
    return false;
}

static void getCheckPoints(const Objects::Object* object, std::list<ppl7::grafix::Point>& checkpoints)
{
    const ppl7::grafix::Drawable& draw = object->texture->getDrawable(object->sprite_no);
    ppl7::grafix::Rect boundary = object->texture->spriteBoundary(object->sprite_no, 1.0f, object->p.x, object->p.y);
    if (!draw.width()) return;
    int stepx = boundary.width() / 16;
    int stepy = boundary.height() / 16;
    for (int py = boundary.y1; py < boundary.y2; py += stepx) {
        for (int px = boundary.x1; px < boundary.x2; px += stepy) {
            ppl7::grafix::Color c = draw.getPixel(px - boundary.x1, py - boundary.y1);
            if (c.alpha() > 92) {
                checkpoints.push_back(ppl7::grafix::Point(px, py));
            }
        }
    }
}

static bool checkCollision(const Objects::Object* obj1, const std::list<ppl7::grafix::Point> checkpoints1, const Objects::Object* obj2)
{
    if (obj1->pixelExactCollision == false && obj2->pixelExactCollision == false) return true;
    ppl7::grafix::Rect intersection = obj1->boundary.intersected(obj2->boundary);
    std::list<ppl7::grafix::Point>::const_iterator it;
    if (obj1->pixelExactCollision == false) {
        std::list<ppl7::grafix::Point> checkpoints2;
        getCheckPoints(obj2, checkpoints2);
        // ppl7::PrintDebug("we have %d checkpoints\n", (int)checkpoints2.size());
        for (it = checkpoints2.begin(); it != checkpoints2.end(); ++it) {
            if (it->inside(intersection)) return true;
        }
    } else if (obj2->pixelExactCollision == false) {
        for (it = checkpoints1.begin(); it != checkpoints1.end(); ++it) {
            if (it->inside(intersection)) return true;
        }
    } else {
        ppl7::PrintDebug("checkCollision with two pixelExcact objects not implemented yet\n");
    }
    return false;
}

void ObjectSystem::detectObjectCollision(const Objects::Object* object, std::list<Objects::Object*>& collision_object_list)
{
    collision_object_list.clear();
    std::list<ppl7::grafix::Point> checkpoints;
    if (object->pixelExactCollision) getCheckPoints(object, checkpoints);
    std::map<uint32_t, Objects::Object*>::const_iterator it;
    for (it = object_list.begin(); it != object_list.end(); ++it) {
        if (it->second != object && it->second->enabled == true && it->second->visibleAtPlaytime == true &&
            it->second->myPlane == object->myPlane) {
            if (object->boundary.intersects(it->second->boundary)) {
                if (checkCollision(object, checkpoints, it->second)) collision_object_list.push_back(it->second);
            }
        }
    }
}

void ObjectSystem::detectObjectCollision(const ppl7::grafix::Rect& boundary, std::list<Objects::Object*>& collision_object_list)
{
    // ppl7::PrintDebug("ObjectSystem::detectObjectCollision\n");
    collision_object_list.clear();
    std::map<uint32_t, Objects::Object*>::const_iterator it;
    for (it = object_list.begin(); it != object_list.end(); ++it) {
        /*
        if (it->second->type() == Objects::Type::GlimmerNode) {
            // ppl7::PrintDebug("checking against GlimmerNode: %d, rect: %d:%d - %d:%d\n", it->second->id,
            // it->second->boundary.x1, it->second->boundary.y1, it->second->boundary.x2, it->second->boundary.y2);
            if (it->second->enabled == true && it->second->myPlane == PlaneId::Player) {
                if (boundary.intersects(it->second->boundary)) {
                    collision_object_list.push_back(it->second);
                }
            }
        }
            */
    }
}

#ifdef TODO
// Wenn wir für jedes Objekt ein eigenes Include machen wollen, können wir hier schlecht alles includen.
// Vielleicht besser auslagern oder andere Lösung finden.

namespace Objects
{
} // namespace Objects

#endif