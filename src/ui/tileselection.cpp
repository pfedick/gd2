#include "ui/tileselection.h"
#include "widgets/colorselection.h"
#include "game.h"

TilesSelection::TilesSelection(int x, int y, int width, int height, Game* game)
    : ppltk::Frame(x, y, width, height)
{
    tileset = 1;

    this->game = game;
    ppl7::grafix::Rect client = this->clientRect();

    this->addChild(new ppltk::Label(5, 5, 60, 30, "Tileset: "));
    tileset_combobox = new ppltk::ComboBox(60, 5, client.width() - 60, 25);
    tileset_combobox->setEventHandler(this);
    this->addChild(tileset_combobox);

    this->addChild(new ppltk::Label(5, 35, 70, 20, "Layer: "));
    layer0 = new ppltk::RadioButton(60, 35, 40, 20, "-2");
    this->addChild(layer0);

    layer1 = new ppltk::RadioButton(100, 35, 40, 20, "-1");
    this->addChild(layer1);

    layer2 = new ppltk::RadioButton(140, 35, 40, 20, "0", true);
    this->addChild(layer2);

    layer3 = new ppltk::RadioButton(180, 35, 40, 20, "+1");
    this->addChild(layer3);

    layer4 = new ppltk::RadioButton(220, 35, 40, 20, "+2");
    this->addChild(layer4);

    setTileLayer(2);

    tilesframe = new TilesFrame(5, 60, client.width() - 8, client.height() - 60 - 300, game);
    this->addChild(tilesframe);

    colorframe = new ColorSelectionFrame(5, client.height() - 300, client.width() - 8, 300, game->level.palette);
    colorframe->setEventHandler(this);
    this->addChild(colorframe);
    tilesframe->setColor(colorframe->color());
}

ppl7::String TilesSelection::widgetType() const
{
    return "TilesSelection";
}

void TilesSelection::setSelectedTile(int nr)
{
    tilesframe->setSelectedTile(nr);
}

int TilesSelection::selectedTile() const
{
    return tilesframe->selectedTile();
}

int TilesSelection::currentTileLayer() const
{
    if (layer0->checked()) return 0;
    if (layer1->checked()) return 1;
    if (layer2->checked()) return 2;
    if (layer3->checked()) return 3;
    if (layer4->checked()) return 4;
    return 2;
}

void TilesSelection::setTileLayer(int layer)
{
    if (layer == 0) layer0->setChecked(true);
    if (layer == 1) layer1->setChecked(true);
    if (layer == 2) layer2->setChecked(true);
    if (layer == 3) layer3->setChecked(true);
    if (layer == 4) layer4->setChecked(true);
}

void TilesSelection::setCurrentTileSet(int id)
{
    auto it = tilesets.find(id);
    if (it == tilesets.end()) return;
    tileset = id;
    tilesframe->setSprites(it->second.tiles);
    tileset_combobox->setCurrentIndex(id - 1);
    needsRedraw();
}

int TilesSelection::currentTileSet() const
{
    return tileset;
}

void TilesSelection::setTileSet(int id, const ppl7::String& name, SpriteTexture* sprites)
{
    if (id < 1) return;
    tilesets[id] = {name, sprites};
    tileset_combobox->add(name, ppl7::ToString("%d", id));
    if (id == 1) setCurrentTileSet(1);
}

int TilesSelection::colorIndex() const
{
    return colorframe->colorIndex();
}

void TilesSelection::setColorIndex(int index)
{
    colorframe->setColorIndex(index);
}

void TilesSelection::valueChangedEvent(ppltk::Event* event, int value)
{
    if (event->widget() == tileset_combobox) {
        // printf("value=%d\n",value);
        setCurrentTileSet(value + 1);
    } else if (event->widget() == colorframe) {
        tilesframe->setColor(colorframe->color());
    }
}
