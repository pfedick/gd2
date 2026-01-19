#ifndef INCLUDE_LEVEL_H_
#define INCLUDE_LEVEL_H_
#include <map>
#include <ppl7.h>

#include "renderpipelines.h"
#include "background.h"
#include "colorpalette.h"

class ModifiableParameter
{
public:
    Background::Type backgroundType;
    ppl7::String BackgroundImage;
    ppl7::String CurrentSong;
    ppl7::grafix::Color BackgroundColor;
    ppl7::grafix::Color GlobalLighting;
    ModifiableParameter();
    void clear();
    bool operator==(const ModifiableParameter& other) const;
};

class LevelDescription
{
public:
    bool partOfStory;
    bool visibleInLevelSelection;
    int levelSort;
    std::map<ppl7::String, ppl7::String> LevelName;
    std::map<ppl7::String, ppl7::String> Description;
    ppl7::String Author;
    ppl7::ByteArray Thumbnail;

    ppl7::String Filename; // only for Level selection
    bool isCustomLevel;    // only for Level selection

    LevelDescription();
    void clear();
    bool loadFromFile(const ppl7::String& filename);
    void loadFromAssocArray(const ppl7::AssocArray& a);
};

class LevelParameter : public ModifiableParameter, public LevelDescription
{
private:
public:
    int width;
    int height;
    // ppl7::String Name;
    ppl7::String InitialSong;
    std::vector<ppl7::String> SongPlaylist;
    bool randomSong;

    bool drainBattery;
    float batteryDrainRate; // EnergyPointsPerSecond
    bool flashlightOnOnLevelStart;
    std::set<int> InitialItems;

    LevelParameter();
    void clear();
    size_t size() const;
    void save(ppl7::File& ff, int chunk_id) const;
    void load(const ppl7::ByteArrayPtr& ba);
};

void getLevelList(std::list<LevelDescription>& level_list);

class Level
{
    friend class Game;

public:
    LevelParameter params;
    ModifiableParameter runtimeParams;
    ColorPalette palette;

private:
    Plane FarPlane;
    Plane PlayerPlane;
    Plane FrontPlane;
    Plane BackPlane;
    Plane MiddlePlane;
    Plane HorizonPlane;
    Plane NearPlane;
    TileTypePlane TileTypeMatrix;
    SpriteSystem HorizonSprites[2] = {SpriteSystem(palette), SpriteSystem(palette)};
    SpriteSystem FarSprites[2] = {SpriteSystem(palette), SpriteSystem(palette)};
    SpriteSystem MiddleSprites[2] = {SpriteSystem(palette), SpriteSystem(palette)};
    SpriteSystem BackSprites[2] = {SpriteSystem(palette), SpriteSystem(palette)};
    SpriteSystem PlayerSprites[3] = {SpriteSystem(palette), SpriteSystem(palette), SpriteSystem(palette)};
    SpriteSystem FrontSprites[2] = {SpriteSystem(palette), SpriteSystem(palette)};
    SpriteSystem NearSprites[2] = {SpriteSystem(palette), SpriteSystem(palette)};

    /*
    LightLayer HorizonLights=LightLayer(palette);
    LightLayer FarLights=LightLayer(palette);
    LightLayer MiddleLights=LightLayer(palette);
    //LightLayer BackLights=LightLayer(palette);
    LightLayer PlayerLights=LightLayer(palette);
    LightLayer FrontLights=LightLayer(palette);
    LightLayer NearLights=LightLayer(palette);
    */
    LightSystem lights;

    Decker::Objects::ObjectSystem* objects;
    ParticleSystem* particles;
    Waynet waynet;

    ppl7::grafix::Rect viewport;
    SpriteTexture* tileset[MAX_TILESETS + 1];
    SpriteTexture* spriteset[MAX_SPRITESETS + 1];
    SDL_Texture* tex_render_target;
    SDL_Texture* tex_render_lightmap;
    SDL_Texture* tex_render_layer;
    SDL_Texture* tex_blur_temp;
    RenderState* renderstate;

    bool editMode;
    bool showSprites;
    bool showObjects;
    bool showParticles;
    bool lightsEnabled;

    void clear();

public:
    enum LevelChunkId
    {
        chunkPlayerPlane = 1,
        chunkFrontPlane = 2,
        chunkFarPlane = 3,
        chunkBackPlane = 4,
        chunkMiddlePlane = 5,
        chunkNearPlane = 6,
        chunkHorizonPlane = 7,
        chunkPlayerSpritesLayer0 = 10,
        chunkPlayerSpritesLayer1 = 11,
        chunkFrontSpritesLayer0 = 12,
        chunkFrontSpritesLayer1 = 13,
        chunkFarSpritesLayer0 = 14,
        chunkFarSpritesLayer1 = 15,
        chunkBackSpritesLayer0 = 16,
        chunkBackSpritesLayer1 = 17,
        chunkMiddleSpritesLayer0 = 18,
        chunkMiddleSpritesLayer1 = 19,
        chunkTileTypes = 20,
        chunkHorizonSpritesLayer0 = 21,
        chunkHorizonSpritesLayer1 = 22,
        chunkNearSpritesLayer0 = 23,
        chunkNearSpritesLayer1 = 24,
        chunkPlayerSpritesLayer2 = 25,
        chunkObjects = 30,
        chunkWayNet = 31,
        chunkLevelParameter = 32,
        chunkColorPalette = 33,
        chunkLightsHorizon = 40,
        chunkLightsFar = 41,
        chunkLightsMiddle = 42,
        chunkLightsBack = 43,
        chunkLightsPlayer = 44,
        chunkLightsFront = 45,
        chunkLightsNear = 46,
        chunkLights = 47,
    };

private:
    void drawNonePlayerPlane(SDL_Renderer* renderer,
                             PlaneId planeid,
                             const Plane& plane,
                             const SpriteSystem& sprites1,
                             const SpriteSystem& sprites2,
                             const ppl7::grafix::Point& worldcoords,
                             Metrics& metrics,
                             Particle::Layer particle_back,
                             Particle::Layer particle_front);
    void drawPlane(SDL_Renderer* renderer, const Plane& plane, const ppl7::grafix::Point& worldcoords) const;
    void drawParticles(SDL_Renderer* renderer, Particle::Layer layer, const ppl7::grafix::Point& worldcoords, Metrics& metrics);
    void addLightmap(SDL_Renderer* renderer,
                     LightPlaneId plane,
                     LightPlayerPlaneMatrix pplane,
                     const ppl7::grafix::Point& worldcoords,
                     Metrics& metrics);
    void prepareLayer(SDL_Renderer* renderer);
    void blurLayer(SDL_Renderer* renderer, float factor = 0.0f);

public:
    Level();
    ~Level();
    void setEditmode(bool enabled);
    void setShowSprites(bool enabled);
    void setShowObjects(bool enabled);
    void setShowParticles(bool enabled);
    void setEnableLights(bool enabled);
    void setTileset(int no, SpriteTexture* tileset);
    void setSpriteset(int no, SpriteTexture* spriteset);
    void create(int width, int height);
    void load(const ppl7::String& Filename);
    void save(const ppl7::String& Filename);
    void backup(const ppl7::String& Filename);
    void draw(SDL_Renderer* renderer, const ppl7::grafix::Point& worldcoords, Player* player, Metrics& metrics, Glimmer* glimmer);
    void setViewport(const ppl7::grafix::Rect& r);
    void setRenderTargets(SDL_Texture* tex_render_target,
                          SDL_Texture* tex_render_lightmap,
                          SDL_Texture* tex_render_layer,
                          SDL_Texture* tex_blur_temp);
    void setRenderState(RenderState* state);
    Plane& plane(int id);
    SpriteSystem& spritesystem(int plane, int layer);
    // LightLayer& lightsystem(int plane);
    void updateVisibleSpriteLists(const ppl7::grafix::Point& worldcoords, const ppl7::grafix::Rect& viewport);
    void updateVisibleLightsLists(const ppl7::grafix::Point& worldcoords, const ppl7::grafix::Rect& viewport);
    void updateDynamicLightsLists(const ppl7::grafix::Point& worldcoords, const ppl7::grafix::Rect& viewport);
    bool findSprite(
        const ppl7::grafix::Point& p, const ppl7::grafix::Point& worldcoords, SpriteSystem::Item& item, int& plane, int& layer) const;
    size_t countSprites() const;
    size_t countVisibleSprites() const;
    size_t countLights() const;
    size_t countVisibleLights() const;

    void getLevelStats(LevelStats& stats) const;

    size_t tileCount() const;
    ppl7::grafix::Rect getOccupiedArea() const;
    ppl7::grafix::Rect getOccupiedAreaFromTileTypePlane() const;
    void TakeScreenshot(Screenshot* screenshot);
};

#endif