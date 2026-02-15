#include "game.h"
#include "level.h"
#include "ui/spriteselection.h"
#include "widgets/colorselection.h"

SpriteSelection::SpriteSelection(int x, int y, int width, int height, Game* game)
    : ppltk::Frame(x, y, width, height)
{
    notifies_enabled = false;
    tileset = 1;
    this->game = game;
    ppl7::grafix::Rect client = this->clientRect();
    int y1 = 5;

    this->addChild(new ppltk::Label(5, y1, 80, 30, "Spriteset: "));
    tileset_combobox = new ppltk::ComboBox(85, y1, client.width() - 85, 25);
    tileset_combobox->setEventHandler(this);
    this->addChild(tileset_combobox);
    y1 += 30;

    this->addChild(new ppltk::Label(5, y1, 70, 20, "Layer: "));
    sprite_layer_combobox = new ppltk::ComboBox(85, y1, client.width() - 85, 25);
    sprite_layer_combobox->add("behind Tiles", "0");
    sprite_layer_combobox->add("before Tiles", "1");
    sprite_layer_combobox->setEventHandler(this);
    this->addChild(sprite_layer_combobox);
    y1 += 30;

    this->addChild(new ppltk::Label(5, y1, 80, 30, "Z-Axis: "));
    z_axis = new ppltk::HorizontalSlider(85, y1, client.width() - 85, 30);
    z_axis->setLimits(0, 15);
    z_axis->enableSpinBox(true, 1, 70);
    z_axis->setEventHandler(this);
    this->addChild(z_axis);
    y1 += 30;
    this->addChild(new ppltk::Label(5, y1, 80, 30, "Scale: "));
    scale_slider = new ppltk::DoubleHorizontalSlider(85, y1, client.width() - 85, 30);
    scale_slider->setValue(1.0f);
    scale_slider->setLimits(0.1f, 4.0f);
    scale_slider->enableSpinBox(true, 0.05, 2, 70);
    scale_slider->setEventHandler(this);
    this->addChild(scale_slider);
    y1 += 30;

    this->addChild(new ppltk::Label(5, y1, 80, 30, "Rotation: "));
    rotation_slider = new ppltk::DoubleHorizontalSlider(85, y1, client.width() - 85, 30);
    rotation_slider->setValue(0.0f);
    rotation_slider->setLimits(0.0f, 360.0f);
    rotation_slider->enableSpinBox(true, 5.0f, 0, 70);
    rotation_slider->setEventHandler(this);
    this->addChild(rotation_slider);

    y1 += 35;

    tilesframe = new TilesFrame(5, y1, client.width() - 8, client.height() - 300 - y1, game);
    this->addChild(tilesframe);

    colorframe = new ColorSelectionFrame(5, client.height() - 300, client.width() - 8, 300, game->level.palette);
    colorframe->setEventHandler(this);
    this->addChild(colorframe);
    tilesframe->setColor(colorframe->color());
    notifies_enabled = true;
}

void SpriteSelection::enableNotfies(bool enable)
{
    notifies_enabled = enable;
}

void SpriteSelection::setSelectedSprite(int nr)
{
    tilesframe->setSelectedTile(nr);
}

int SpriteSelection::selectedSprite() const
{
    return tilesframe->selectedTile();
}

ParallaxLayer::SpritePosition SpriteSelection::currentSpriteLayer() const
{
    return static_cast<ParallaxLayer::SpritePosition>(sprite_layer_combobox->currentIdentifier().toInt());
}

int SpriteSelection::spriteSetDimensions() const
{
    auto it = spritesets.find(tileset);
    if (it == spritesets.end()) return 1;
    return it->second.dimensions;
}

void SpriteSelection::setCurrentSpriteLayer(ParallaxLayer::SpritePosition layer)
{
    sprite_layer_combobox->setCurrentIdentifier(ppl7::ToString("%d", static_cast<int>(layer)));
}

void SpriteSelection::setCurrentSpriteSet(int id)
{
    auto it = spritesets.find(tileset);
    if (it == spritesets.end()) return;
    if (it->second.sprites == nullptr) return;
    tileset = id;
    tilesframe->setSprites(it->second.sprites);
    tilesframe->setSelectedTile(-1);
    // tileset_combobox->setCurrentIndex(id - 1);
    tileset_combobox->setCurrentIdentifier(ppl7::ToString("%d", id));
}

int SpriteSelection::currentSpriteSet() const
{
    return tileset;
}

void SpriteSelection::setSpriteSet(int id, const ppl7::String& name, SpriteTexture* sprites, int dimensions)
{
    if (id < 1) return;
    SpriteSet sset;
    sset.name = name;
    sset.sprites = sprites;
    sset.dimensions = dimensions;
    spritesets[id] = sset;
    tileset_combobox->add(name, ppl7::ToString("%d", id));
    if (id == 1) setCurrentSpriteSet(1);
}

void SpriteSelection::setSpriteScale(float factor)
{
    if (factor >= 0.1f && factor <= 4.0f) scale_slider->setValue(factor);
}

float SpriteSelection::spriteScale() const
{
    return scale_slider->value();
}

void SpriteSelection::setSpriteRotation(float rotation)
{
    rotation_slider->setValue(rotation);
}

float SpriteSelection::spriteRotation() const
{
    return rotation_slider->value();
}

int SpriteSelection::colorIndex() const
{
    return colorframe->colorIndex();
}

void SpriteSelection::setColorIndex(int index)
{
    colorframe->setColorIndex(index);
}

void SpriteSelection::setZAxis(int z)
{
    z_axis->setValue(z);
}

int SpriteSelection::zAxis() const
{
    return ((int)z_axis->value());
}

void SpriteSelection::valueChangedEvent(ppltk::Event* event, int value)
{
    if (event->widget() == tileset_combobox) {
        int v = tileset_combobox->currentIdentifier().toInt();
        // ppl7::PrintDebug("value=%d, text=%s\n", v, (const char*)tileset_combobox->currentText());
        setCurrentSpriteSet(v);
    } else if (event->widget() == colorframe) {
        tilesframe->setColor(colorframe->color());
        if (notifies_enabled) game->editor.updateSpriteFromUi();
    } else if (event->widget() == sprite_layer_combobox) {
        if (notifies_enabled) game->editor.updateSpriteFromUi();
    }
}

void SpriteSelection::valueChangedEvent(ppltk::Event* event, int64_t value)
{
    if (event->widget() == z_axis) {
        if (notifies_enabled) game->editor.updateSpriteFromUi();
    }
}

void SpriteSelection::valueChangedEvent(ppltk::Event* event, double value)
{
    if (event->widget() == scale_slider || event->widget() == rotation_slider) {
        if (notifies_enabled) game->editor.updateSpriteFromUi();
    }
}

void SpriteSelection::toggledEvent(ppltk::Event* event, bool checked)
{
    // ppl7::PrintDebug("SpriteSelection::toggledEvent\n");
}
