#ifndef INCLUDE_TILETYPESELECTION_H_
#define INCLUDE_TILETYPESELECTION_H_
#include <ppl7.h>
#include <ppl7-grafix.h>
#include <ppltk.h>

class Game;
class TilesFrame;
class SpriteTexture;

class TileTypeSelection : public ppltk::Frame
{
private:
    Game* game;
    TilesFrame* tiletypesframe;

public:
    TileTypeSelection(int x, int y, int width, int height, Game* game, SpriteTexture* tiletypes);
    ppl7::String widgetType() const override;

    void setTileType(int nr);
    int tileType() const;
};

#endif /* INCLUDE_TILETYPESELECTION_H_ */