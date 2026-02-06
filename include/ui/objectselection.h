#ifndef INCLUDE_OBJECTSELECTION_H_
#define INCLUDE_OBJECTSELECTION_H_

#include <ppl7.h>
#include <ppl7-grafix.h>
#include <ppltk.h>
#include "level.h"

class SpriteTexture;
class Game;

class ObjectsFrame : public ppltk::Frame
{
private:
    SpriteTexture* spriteset;
    ppltk::Scrollbar* scrollbar;
    int selected_object;
    bool playerPlaneObjectsVisible;
    class Item
    {
    public:
        int id;
        ppl7::String name;
        int sprite_no;
        Item(int id, const ppl7::String& name, int sprite_no);
    };
    std::map<size_t, Item> object_map;
    void addObject(int id, const ppl7::String& name, int sprite_no);

public:
    ObjectsFrame(int x, int y, int width, int height);

    void showPlayerPlaneObjects();

    void setSpriteSet(SpriteTexture* texture);
    int selectedObjectType() const;
    void setObjectType(int type);

    ppl7::String widgetType() const override;
    void paint(ppl7::grafix::Drawable& draw) override;
    void valueChangedEvent(ppltk::Event* event, int value) override;
    void mouseDownEvent(ppltk::MouseEvent* event) override;
    void mouseWheelEvent(ppltk::MouseEvent* event) override;
};

class ObjectSelection : public ppltk::Frame
{
private:
    Game* game;
    // TilesFrame *tilesframe;
    SpriteTexture* spriteset;
    ppltk::Scrollbar* scrollbar;
    ppltk::ComboBox* layer_selection;
    ppltk::CheckBox* difficulty_easy;
    ppltk::CheckBox* difficulty_normal;
    ppltk::CheckBox* difficulty_hard;
    ObjectsFrame* objects_frame;
    int selected_object;

    class Item
    {
    public:
        int id;
        ppl7::String name;
        int sprite_no;
        Item(int id, const ppl7::String& name, int sprite_no);
    };
    std::map<size_t, Item> object_map;

public:
    ObjectSelection(int x, int y, int width, int height, Game* game);
    void setSpriteSet(SpriteTexture* texture);
    int selectedObjectType() const;
    int currentLayer() const;
    void setLayer(int layer);
    void setObjectType(int type);
    void setObjectDifficulty(uint8_t matrix);
    uint8_t getDifficulty() const;
    ppl7::String widgetType() const override;
    void valueChangedEvent(ppltk::Event* event, int value) override;
    void mouseDownEvent(ppltk::MouseEvent* event) override;
    void toggledEvent(ppltk::Event* event, bool checked);
};

#endif /* INCLUDE_OBJECTSELECTION_H_ */
