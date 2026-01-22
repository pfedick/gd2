#ifndef INCLUDE_LEVEL_H_
#define INCLUDE_LEVEL_H_
#include <map>
#include <ppl7.h>

#include "renderpipelines.h"
#include "background.h"
#include "colorpalette.h"
#include "spritesystem.h"
#include "particle.h"
#include "tiles.h"
#include "tiletypes.h"

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

enum class ParallaxLayerId
{
    Near,
    Close,
    Front,
    Player,
    Back,
    Middle,
    Far,
    Horizon,
    MaxLayerId
};

class ParallaxLayer
{
private:
public:
    float blur_factor = 0.0f;
    float speed_factor = 1.0f;
    float size_factor = 1.0f;

    Plane tiles;
    enum class SpritePosition
    {
        Background = 0,
        Front
    };
    SpriteSystem front_sprites;
    SpriteSystem background_sprites;
    // TODO:
    // - Lights    (falls das sinn macht)
    // - Objects   (falls das sinn macht)
    // - Particle  (falls das sinn macht)
    ParallaxLayer();
    ~ParallaxLayer();
    void init(float blur, float speed, float size);
    void clear();
};

class Level
{
    friend class Game;

public:
    LevelParameter params;
    ModifiableParameter runtimeParams;
    ColorPalette palette;

private:
    ParallaxLayer parallax_layers[static_cast<int>(ParallaxLayerId::MaxLayerId)];
    TileTypePlane TileTypeMatrix;
    GPUContext* gpu;

    // LightSystem lights;
    // Decker::Objects::ObjectSystem* objects;
    // ParticleSystem* particles;
    // Waynet waynet;

    ppl7::grafix::Rect viewport;
    std::vector<SpriteTexture*> tileset;
    std::vector<SpriteTexture*> spriteset;
    // SDL_GPUTexture* tex_render_target;
    SDL_GPUTexture* tex_render_lightmap;
    SDL_GPUTexture* tex_render_layer;
    SDL_GPUTexture* tex_blur_temp;
    RenderPipelines* renderpipelines;

    bool editMode;
    bool showSprites;
    bool showObjects;
    bool showParticles;
    bool lightsEnabled;

    void clear();

public:
    enum class ChunkId
    {
        Tiles = 1, // Den Layer kodieren wir da rein als Attribut
        TileTypes = 2,
        Sprites = 3, // Den Layer, sowie Front/Background kodieren wir da rein als Attribut
        Objects = 4,
        WayNet = 5,
        LevelParameter = 6,
        ColorPalette = 7,
        Lights = 8,
    };

private:
    /*
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
    */

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
    void initialize(GPUContext& gpu, RenderPipelines* renderpipelines);
    void createRenderTargets(int width, int height);
    // void draw(SDL_Renderer* renderer, const ppl7::grafix::Point& worldcoords, Player* player, Metrics& metrics, Glimmer* glimmer);
    void setViewport(const ppl7::grafix::Rect& r);
    void setRenderPipeline(SDL_GPUGraphicsPipeline* state);
    ParallaxLayer& layer(ParallaxLayerId id);
    Plane& plane(ParallaxLayerId id);
    SpriteSystem& spritesystem(ParallaxLayerId id, ParallaxLayer::SpritePosition layer);
    // SpriteSystem& spritesystem(int layer, int layer);
    //  LightLayer& lightsystem(int plane);
    void updateVisibleSpriteLists(const ppl7::grafix::Point& worldcoords, const ppl7::grafix::Rect& viewport);
    // void updateVisibleLightsLists(const ppl7::grafix::Point& worldcoords, const ppl7::grafix::Rect& viewport);
    // void updateDynamicLightsLists(const ppl7::grafix::Point& worldcoords, const ppl7::grafix::Rect& viewport);
    bool findSprite(const ppl7::grafix::Point& p,
                    const ppl7::grafix::Point& worldcoords,
                    SpriteSystem::Item& item,
                    int& layer,
                    ParallaxLayer::SpritePosition& layer_position) const;
    size_t countSprites() const;
    size_t countVisibleSprites() const;
    // size_t countLights() const;
    // size_t countVisibleLights() const;

    // void getLevelStats(LevelStats& stats) const;

    size_t tileCount() const;
    ppl7::grafix::Rect getOccupiedArea() const;
    ppl7::grafix::Rect getOccupiedAreaFromTileTypePlane() const;
    // void TakeScreenshot(Screenshot* screenshot);
};

#endif