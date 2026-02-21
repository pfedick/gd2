#include "ui/objectselection.h"
#include "objectsystem.h"
#include "sprite.h"
#include "level.h"
#include "game.h"

ObjectsFrame::Item::Item(Objects::Type id, const ppl7::String& name, int sprite_no)
{
    this->id = id;
    this->name = name;
    this->sprite_no = sprite_no;
}

ObjectsFrame::ObjectsFrame(int x, int y, int width, int height)
    : ppltk::Frame(x, y, width, height)
{
    ppl7::grafix::Rect client = this->clientRect();
    // setBorderStyle(ppltk::Frame::Inset);
    selected_object = Objects::Type::Invalid;
    spriteset = NULL;
    playerPlaneObjectsVisible = false;

    scrollbar = new ppltk::Scrollbar(client.width() - 25, 0, 24, client.height());
    scrollbar->setName("objects-scrollbar");
    scrollbar->setEventHandler(this);
    this->addChild(scrollbar);

    showPlayerPlaneObjects();
}

void ObjectsFrame::showPlayerPlaneObjects()
{
    if (playerPlaneObjectsVisible) return;
    playerPlaneObjectsVisible = true;
    object_map.clear();
    selected_object = Objects::Type::Invalid;

    addObject(Objects::Type::PlayerStartpoint, "Player start", 0);
    addObject(Objects::Type::SpawnPoint, "Object SpawnPoint", 1);
    addObject(Objects::Type::SavePoint, "Savepoint", 2);
    addObject(Objects::Type::Medikit, "Medikit", 3);
    addObject(Objects::Type::Crystal, "Crystal", 15);
    addObject(Objects::Type::Coin, "Coin", 16);
    addObject(Objects::Type::ExtraLife, "Extra Life", 4);
    addObject(Objects::Type::TouchEmitter, "Object emiter", 5);
    addObject(Objects::Type::Speaker, "Speaker", 13);
    addObject(Objects::Type::RainEmitter, "Rain emiter", 12);
    addObject(Objects::Type::ParticleEmitter, "Particle emiter", 11);
    addObject(Objects::Type::VoiceTrigger, "Voice", 14);
    addObject(Objects::Type::ObjectWatcher, "ObjectWatcher", 9);
    // addObject(Objects::Type::ItemTaker, "ItemTaker", 78);
    addObject(Objects::Type::Trigger, "Trigger", 6);
    addObject(Objects::Type::LightTrigger, "Light Trigger", 10);
    addObject(Objects::Type::PlayerTrigger, "Player Trigger", 7);
    addObject(Objects::Type::LevelModificator, "Level Modificator", 8);
    addObject(Objects::Type::CameraControl, "Camera Control", 17);
    addObject(Objects::Type::Arrow, "Arrow", 18);
    scrollbar->setPosition(0);
    scrollbar->setSize(object_map.size() / 2);
    scrollbar->setVisibleItems((height() - 44) / 160 / 2);
}

Objects::Type ObjectsFrame::selectedObjectType() const
{
    return selected_object;
}

void ObjectsFrame::setObjectType(Objects::Type type)
{
    if (type != selected_object) {
        selected_object = type;
        size_t pos = 0;
        std::map<size_t, Item>::const_iterator it;
        for (it = object_map.begin(); it != object_map.end(); ++it) {
            if (it->second.id == type) {
                pos = it->first;
                break;
            }
        }
        scrollbar->setPosition(pos / 2);
        needsRedraw();
    }
}

void ObjectsFrame::addObject(Objects::Type id, const ppl7::String& name, int sprite_no)
{
    object_map.insert(std::pair<size_t, Item>(object_map.size(), Item(id, name, sprite_no)));
}

void ObjectsFrame::setSpriteSet(SpriteTexture* texture)
{
    spriteset = texture;
}

ppl7::String ObjectsFrame::widgetType() const
{
    return "ObjectsFrame";
}

void ObjectsFrame::valueChangedEvent(ppltk::Event* event, int value)
{
    if (event->widget() == scrollbar) {
        needsRedraw();
    }
}

void ObjectsFrame::mouseDownEvent(ppltk::MouseEvent* event)
{
    if (!spriteset) return;
    if (event->p.x >= scrollbar->x()) return;
    if (event->widget() == this && event->buttonMask & ppltk::MouseState::Left) {
        size_t object_pos = ((event->p.y) / 160) * 2 + ((event->p.x) / 130) + scrollbar->position() * 2;
        std::map<size_t, Item>::const_iterator it;
        it = object_map.find(object_pos);
        if (it != object_map.end()) {
            selected_object = it->second.id;
            // TODO
            // game->setSpriteModeToDraw();
            ppltk::Event event(ppltk::Event::ValueChanged);
            event.setWidget(this);
            getParent()->valueChangedEvent(&event, static_cast<int>(selected_object));
            needsRedraw();
        }
    }
}

void ObjectsFrame::mouseWheelEvent(ppltk::MouseEvent* event)
{
    if (!spriteset) return;
    scrollbar->mouseWheelEvent(event);
    /*
    if (event->wheel.y != 0) {
        scrollbar->setPosition(scrollbar->position() + event->wheel.y * -1);
        needsRedraw();
    }
    */
}

void ObjectsFrame::paint(ppl7::grafix::Drawable& draw)
{
    const ppltk::WidgetStyle& style = ppltk::GetWidgetStyle();
    ppl7::grafix::Color myBorderColorLight = style.frameBorderColorLight;
    ppl7::grafix::Color myBorderColorShadow = style.frameBorderColorShadow;
    ppl7::grafix::Font myFont = style.labelFont;
    ppl7::grafix::Color shade1 = style.frameBackgroundColor * 0.70f;
    ppl7::grafix::Color shade2 = style.frameBackgroundColor * 0.50f;

    ppl7::grafix::Color shade3 = style.frameBackgroundColor * 1.5f;
    ppl7::grafix::Color shade4 = style.frameBackgroundColor * 1.2f;

    myFont.setColor(style.labelFontColor);
    myFont.setOrientation(ppl7::grafix::Font::TOP);

    if (!spriteset) return;
    ppltk::Frame::paint(draw);
    ppl7::grafix::Drawable cdraw = clientDrawable(draw);

    ppl7::grafix::Color white(245, 245, 242, 255);
    int x = 0, y = 0, c = 0;
    std::map<size_t, Item>::const_iterator it;
    for (it = object_map.begin(); it != object_map.end(); ++it) {
        const Item& item = it->second;
        if (c >= scrollbar->position() * 2) {
            ppl7::grafix::Drawable frame = cdraw.getDrawable(x, y, x + 128, y + 128 + 30);
            int w = frame.width() - 1;
            int h = frame.height() - 1;
            if (item.id == selected_object) {
                frame.colorGradient(frame.rect(), shade3, shade4, 1);
            } else {
                frame.colorGradient(frame.rect(), shade1, shade2, 1);
            }
            frame.line(0, 0, w, 0, myBorderColorShadow);
            frame.line(0, 0, 0, h, myBorderColorShadow);
            frame.line(0, h, w, h, myBorderColorLight);
            frame.line(w, 0, w, h, myBorderColorLight);

            spriteset->draw(frame, 0, 0, item.sprite_no);
            ppl7::grafix::Size s = myFont.measure(item.name);
            frame.print(myFont, (128 - s.width) >> 1, 128, item.name);
            x += 130;
            if (x > 255) {
                x = 0;
                y += 160;
            }
            if (y > draw.height()) return;
        }
        c++;
    }
}

// ***************************************************************

ObjectSelection::ObjectSelection(int x, int y, int width, int height, Game* game)
    : ppltk::Frame(x, y, width, height)
{
    setClientOffset(4, 4, 4, 4);
    spriteset = NULL;
    this->game = game;
    selected_object = -1;
    ppl7::grafix::Rect client = this->clientRect();

    int yy = 0;
    this->addChild(new ppltk::Label(0, yy, 80, 30, "Layer:"));
    layer_selection = new ppltk::ComboBox(80, yy, width - 90, 30);
    layer_selection->add("Behind Bricks", ppl7::ToString("%d", static_cast<int>(Objects::Object::Layer::BehindBricks)));
    layer_selection->add("Behind Player", ppl7::ToString("%d", static_cast<int>(Objects::Object::Layer::BehindPlayer)));
    layer_selection->add("Before Player", ppl7::ToString("%d", static_cast<int>(Objects::Object::Layer::BeforePlayer)));
    layer_selection->add("Before Bricks", ppl7::ToString("%d", static_cast<int>(Objects::Object::Layer::BeforeBricks)));
    layer_selection->setCurrentIdentifier(ppl7::ToString("%d", static_cast<int>(Objects::Object::Layer::BeforePlayer)));
    layer_selection->setEventHandler(this);
    this->addChild(layer_selection);
    yy += 30;
    this->addChild(new ppltk::Label(0, yy, 80, 30, "difficulty:"));
    difficulty_easy = new ppltk::CheckBox(80, yy, 80, 30, "easy", true);
    difficulty_easy->setEventHandler(this);
    this->addChild(difficulty_easy);
    difficulty_normal = new ppltk::CheckBox(145, yy, 100, 30, "normal", true);
    difficulty_normal->setEventHandler(this);
    this->addChild(difficulty_normal);
    difficulty_hard = new ppltk::CheckBox(225, yy, 90, 30, "hard", true);
    difficulty_hard->setEventHandler(this);
    this->addChild(difficulty_hard);
    yy += 30;
    this->addChild(new ppltk::Label(0, yy, width, 30, "Object selection:"));
    yy += 30;

    objects_frame = new ObjectsFrame(client.left(), yy, client.width(), client.height() - yy);
    objects_frame->setEventHandler(this);
    this->addChild(objects_frame);
}

Objects::Type ObjectSelection::selectedObjectType() const
{
    return objects_frame->selectedObjectType();
}

void ObjectSelection::setObjectType(Objects::Type type)
{
    objects_frame->setObjectType(type);
}

void ObjectSelection::setObjectDifficulty(uint8_t matrix)
{
    difficulty_easy->setChecked(matrix & 1);
    difficulty_normal->setChecked(matrix & 2);
    difficulty_hard->setChecked(matrix & 4);
}

Objects::Object::Layer ObjectSelection::currentLayer() const
{
    return static_cast<Objects::Object::Layer>(layer_selection->currentIdentifier().toInt());
}

void ObjectSelection::setLayer(Objects::Object::Layer layer)
{
    layer_selection->setCurrentIdentifier(ppl7::ToString("%d", static_cast<int>(layer)));
}

void ObjectSelection::setSpriteSet(SpriteTexture* texture)
{
    objects_frame->setSpriteSet(texture);
}

ppl7::String ObjectSelection::widgetType() const
{
    return "ObjectSelection";
}

void ObjectSelection::valueChangedEvent(ppltk::Event* event, int value)
{
    if (event->widget() == objects_frame) {
        game->editor.setSpriteModeToDraw();
    } else if (event->widget() == layer_selection) {
        // ppl7::PrintDebugTime("ObjectSelection::valueChangedEvent\n");
        game->editor.updateObjectLayerForSelectedObject(layer_selection->currentIdentifier().toInt());
    }
}
void ObjectSelection::mouseDownEvent(ppltk::MouseEvent* event)
{
    // printf("ObjectSelection::mouseDownEvent\n");
}

void ObjectSelection::toggledEvent(ppltk::Event* event, bool checked)
{
    ppltk::Widget* w = event->widget();
    if (w == difficulty_easy || w == difficulty_normal || w == difficulty_hard) {
        uint8_t d = getDifficulty();
        game->editor.updateDifficultyForSelectedObject(d);
    }
}

uint8_t ObjectSelection::getDifficulty() const
{
    uint8_t d = 0b11111000;
    if (difficulty_easy->checked()) d |= 1;
    if (difficulty_normal->checked()) d |= 2;
    if (difficulty_hard->checked()) d |= 4;
    return d;
}
