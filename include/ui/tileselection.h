#ifndef INCLUDE_TILESELECTION_H_
#define INCLUDE_TILESELECTION_H_
#include <ppl7.h>
#include <ppl7-grafix.h>
#include <ppltk.h>

class Game;
class SpriteTexture;
class ColorSelectionFrame;

class TilesFrame : public ppltk::Frame
{
private:
    Game* game;
    SpriteTexture* tiles;
    ppltk::Scrollbar* scrollbar;
    ppl7::grafix::Color color;
    int selected_tile;

public:
    TilesFrame(int x, int y, int width, int height, Game* game);
    ppl7::String widgetType() const;
    void paint(ppl7::grafix::Drawable& draw) override;
    void mouseDownEvent(ppltk::MouseEvent* event) override;
    void mouseMoveEvent(ppltk::MouseEvent* event) override;
    void mouseWheelEvent(ppltk::MouseEvent* event) override;
    void valueChangedEvent(ppltk::Event* event, int value) override;

    void setSelectedTile(int nr);
    int selectedTile() const;
    void setColor(const ppl7::grafix::Color& color);
    void setSprites(SpriteTexture* tiles);
};

class TilesSelection : public ppltk::Frame
{
private:
    Game* game;
    TilesFrame* tilesframe;
    ColorSelectionFrame* colorframe;
    ppltk::RadioButton* layer0;
    ppltk::RadioButton* layer1;
    ppltk::RadioButton* layer2;
    ppltk::RadioButton* layer3;
    ppltk::RadioButton* layer4;
    ppltk::ComboBox* tileset_combobox;
    class TileSet
    {
    public:
        ppl7::String name;
        SpriteTexture* tiles;
        int min;
        int max;
    };
    std::map<int, TileSet> tilesets;

    // ppl7::String tilesetName[MAX_TILESETS + 1];
    // SpriteTexture *tilesets[MAX_TILESETS + 1];

    int tileset;

public:
    TilesSelection(int x, int y, int width, int height, Game* game);
    ppl7::String widgetType() const;
    // virtual void paint(Drawable &draw);

    void setSelectedTile(int nr);
    int selectedTile() const;
    void setCurrentTileSet(int id);
    int currentTileSet() const;
    void setTileSet(int id, const ppl7::String& name, SpriteTexture* tiles);
    // TODO: Erlauben, dass in einer SpriteTexture mehrere Tilesets enthalten sind,
    // die in einem Bereich von min bis max liegen
    // void addTileSet(int id, const ppl7::String& name, SpriteTexture* tiles, int min, int max);

    int currentTileLayer() const;
    void setTileLayer(int layer);
    int colorIndex() const;
    void setColorIndex(int index);

    void valueChangedEvent(ppltk::Event* event, int value) override;
};

#endif