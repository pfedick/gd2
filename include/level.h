#ifndef INCLUDE_LEVEL_H_
#define INCLUDE_LEVEL_H_
#include <map>
#include <ppl7.h>
#include "gpu.h"
#include "renderpipelines.h"
#include "background.h"
#include "colorpalette.h"
#include "spritesystem.h"
#include "particle.h"
#include "tiles.h"
#include "tiletypes.h"
#include "objectsystem.h"
#include "gameviewport.h"
#include "camera.h"
#include "metrics.h"

class Game;

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
    Behind,
    Middle,
    Far,
    Horizon,
    Sky,
    MaxLayerId
};

class RenderState
{
public:
    SDL_GPUCommandBuffer* cmdbuf;
    SDL_GPUTexture* render_target;
    SDL_GPUTexture* render_lightmap;
    SDL_GPUTexture* render_layer;
    SDL_GPUTexture* render_normal;
    SDL_GPUTexture* depth_buffer;
    SDL_GPUTexture* blur_temp;
    GPUContext* gpu;
    RenderPipelines* renderpipelines;
    GPUBatcher* batcher;
    ppl7::grafix::Size render_target_size;

    RenderState();
};

class ParallaxLayer
{
private:
    bool hasVisibleGrafix() const;
    void drawTileGrid(RenderState& renderstate, const ppl7::grafix::PointF& worldcoords, const GameViewport& viewport);

    void blur(RenderState& renderstate, SDL_GPUTexture* texture);
    void copyLayerToTarget(RenderState& renderstate, SDL_GPUTexture* source, SDL_GPUTexture* target);
    Player* player = NULL;

public:
    float blur_factor = 0.0f;
    float speed_factor = 1.0f;
    float size_factor = 1.0f;
    ParallaxLayerId myParallaxLayer;
    bool isVisible = true;
    bool bShowGrid = false;
    bool bShowTileTypes = false;
    bool bBlurEnabled = true;
    bool bLightningEnabled = true;
    bool bShowSprites = true;
    bool bShowObjects = true;
    bool bShowParticles = true;
    bool isEditLayer = false;
    Game* game = NULL;

    TileGrid tiles;
    enum class SpritePosition
    {
        Background = 0,
        Front
    };
    SpriteSystem front_sprites;
    SpriteSystem background_sprites;
    ObjectSystem objects;
    ParticleSystem particles;
    TileTypePlane TileTypeMatrix;
    // TODO:
    // - Lights    (falls das sinn macht) => Ja, macht es!

    ParallaxLayer();
    ~ParallaxLayer();
    void init(ParallaxLayerId layerType, float blur, float speed, float size);
    void setPlayer(Player* p);
    void clear();
    void updateSprites(const GameClock& clock, const ppl7::grafix::PointF& worldcoords, const ppl7::grafix::Size& render_target_size);
    void updateObjects(const GameClock& clock, const ppl7::grafix::PointF& worldcoords, const ppl7::grafix::Size& render_target_size);
    void updateParticles(const GameClock& clock, const ppl7::grafix::PointF& worldcoords, const ppl7::grafix::Size& render_target_size);
    void updateLights(const GameClock& clock, const ppl7::grafix::PointF& worldcoords, const ppl7::grafix::Size& render_target_size);

    void draw(RenderState& renderstate,
              SDL_GPUTexture* swapchainTexture,
              const ppl7::grafix::PointF& worldcoords,
              const GameViewport& viewport,
              Metrics& metrics);
};

class Level
{
    friend class Game;
    friend class ParticleUpdateThread;

public:
    LevelParameter params;
    ModifiableParameter runtimeParams;
    ColorPalette palette;

private:
    ParallaxLayer parallax_layers[static_cast<int>(ParallaxLayerId::MaxLayerId)];
    RenderState renderstate;

    // LightSystem lights;
    // Decker::Objects::ObjectSystem* objects;
    // ParticleSystem* particles;
    // Waynet waynet;

    std::vector<SpriteTexture*> spriteset;
    // SDL_GPUTexture* tex_render_target;
    ppl7::grafix::Size render_target_size;

    bool editMode;
    bool showSprites;
    bool showObjects;
    bool showParticles;
    bool lightsEnabled;
    bool blurEnabled;

    bool bShowGrid;
    bool bShowTileTypes;
    bool bShowCollisions;
    ParallaxLayerId editlayer;
    Player* player;

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

    void clearRenderTarget(SDL_GPUCommandBuffer* cmdbuf);
    void copyRenderTargetToSwapchain(SDL_GPUCommandBuffer* cmdbuf, SDL_GPUTexture* swapchainTexture, const SDL_FRect& destRect);
    void updateVisibility();
    void drawDebug(const Camera& camera, const GameViewport& viewport, const Player* player);

public:
    Level(Game* game);
    ~Level();
    void setPlayer(Player* player);
    void setEditmode(bool enabled);
    void setTileset(int no, SpriteTexture* tileset);
    void setSpriteset(int no, SpriteTexture* spriteset);
    void setTileTypeSpriteset(SpriteTexture* spriteset);
    void create(int width, int height);
    void load(const ppl7::String& Filename);
    void save(const ppl7::String& Filename);
    void backup(const ppl7::String& Filename);
    void initialize(GPUContext& gpu, RenderPipelines& renderpipelines, GPUBatcher& batcher);
    void resizeRenderBuffer(const ppl7::grafix::Size& size);
    void draw(SDL_GPUCommandBuffer* cmdbuf,
              SDL_GPUTexture* swapchainTexture,
              const Camera& worldcoords,
              const GameViewport& viewport,
              Metrics& metrics);

    void update(const GameClock& clock,
                Metrics& metrics,
                const ppl7::grafix::PointF& worldcoords,
                const ppl7::grafix::Size& render_target_size);

    void setEditLayer(ParallaxLayerId layer);
    void setShowTileGrid(bool enable);
    void setShowTileTypes(bool enable);
    void setShowCollisions(bool enable);

    ParallaxLayer& layer(ParallaxLayerId id);
    ParallaxLayer& editLayer();
    SpriteSystem& spritesystem(ParallaxLayerId id, ParallaxLayer::SpritePosition layer);
    // SpriteSystem& spritesystem(int layer, int layer);
    // LightLayer& lightsystem(int plane);

    // void updateVisibleSpriteLists(const ppl7::grafix::Point& worldcoords, const ppl7::grafix::Rect& viewport);
    // void updateVisibleLightsLists(const ppl7::grafix::Point& worldcoords, const ppl7::grafix::Rect& viewport);
    // void updateDynamicLightsLists(const ppl7::grafix::Point& worldcoords, const ppl7::grafix::Rect& viewport);
    bool findSprite(const ppl7::grafix::Point& p,
                    const ppl7::grafix::Point& worldcoords,
                    SpriteSystem::Item& item,
                    ParallaxLayerId& parallax_layer,
                    ParallaxLayer::SpritePosition& layer_position) const;
    size_t countSprites() const;
    size_t countVisibleSprites() const;
    size_t countLights() const;
    size_t countVisibleLights() const;
    size_t countObjects() const;
    size_t countVisibleObjects() const;
    size_t countParticles() const;
    size_t countVisibleParticles() const;

    // void getLevelStats(LevelStats& stats) const;

    size_t tileCount() const;
    // ppl7::grafix::Rect getOccupiedArea() const;
    ppl7::grafix::Rect getOccupiedAreaFromTileTypePlane(ParallaxLayerId layer) const;
    void updateParticles(double time); // Wird vom ParticleUpdateThread aufgerufen!

    ppl7::grafix::Image getScreenshot(int width, int height);
};

#endif