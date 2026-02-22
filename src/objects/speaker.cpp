#include <ppl7.h>
#include <ppl7-grafix.h>
#include "objectsystem.h"
#include "objects/generic.h"
#include "game.h"
#include "widgets.h"
#include "audiopool.h"

/*
 * TODO:
 * - Die Verwendung von Pixel-Koordinaten für den maximalen Abstand ist nicht ideal,
 *   der der Autor nicht wissen sollte, wie groß die logische Renderseize ist.
 *   Besser wäre es, den Abstand in Weltkoordinaten anzugeben, zum Beispiel Tile-With, bzw. Meter!
 *   Wir haben ein Tile als 100x100 Pixel definiert, was 0,5 Meter entspricht, also könnte man den
 *   den Abstand in doppelter Tile-Size angeben, also 200 Pixel, was 1 Meter entspricht. Das wäre auch
 *   einfacher zu verstehen.
 * - Die Berechnung des Abstands zur Bildschirmmitte im AudioSystem scheint falsch zu sein.
 */

namespace Objects
{

Representation Speaker::representation()
{
    return Representation(SpritesetId::GenericObjects, 11);
}

Speaker::Speaker()
    : Object(Type::Speaker)
{
    sprite_set = SpritesetId::GenericObjects;
    sprite_no = 11;
    collisionDetection = false;
    visibleAtPlaytime = false;
    sprite_no_representation = 11;
    audio = NULL;
    sample_id = AudioEffect::none;
    volume = 1.0f;
    max_distance = 3840;
    enabled = true;
    initial_state = true;
    sample_type = SampleType::AudioLoop;
}

Speaker::~Speaker()
{
    if (audio) {
        getAudioPool().stopInstace(audio);
        delete audio;
        audio = NULL;
    }
}

void Speaker::update(const GameClock& clock, TileTypePlane& ttplane, Player& player)
{
    if (enabled) {
        if (sample_type == SampleType::AudioLoop) {
            if (audio == NULL && sample_id != AudioEffect::none) {
                setSample(sample_id, volume, max_distance);
            } else if (audio) {
                ParallaxLayer& layer = GetGame().level.layer(myParallaxLayer);
                double f = layer.speed_factor * layer.size_factor;
                if (f != 1.0f) {
                    const ppl7::grafix::Point& worldcoords = GetGame().getWorldCoords();
                    ppl7::grafix::Point coords = worldcoords * f;
                    ppl7::grafix::Point pp = ppl7::grafix::Point(p) - coords;
                    audio->setPositional(pp + worldcoords, max_distance);
                } else {
                    // ppl7::PrintDebug("Speaker: setPositional %0f,%0f\n", p.x, p.y);
                    audio->setPositional(p, max_distance);
                }
                audio->setVolume(volume);
            }
        }
    } else if (audio) {
        getAudioPool().stopInstace(audio);
        delete audio;
        audio = NULL;
    }
}

size_t Speaker::saveSize() const
{
    return Object::saveSize() + 13;
}

size_t Speaker::save(unsigned char* buffer, size_t size) const
{
    size_t bytes = Object::save(buffer, size);
    if (!bytes) return 0;
    ppl7::Poke8(buffer + bytes, 3); // Object Version

    ppl7::Poke32(buffer + bytes + 1, sample_id);
    ppl7::Poke16(buffer + bytes + 5, max_distance);
    ppl7::PokeFloat(buffer + bytes + 7, volume);
    int flags = 0;
    if (initial_state) flags |= 1;
    if (sample_type == SampleType::Effect) flags |= 2;
    ppl7::Poke16(buffer + bytes + 11, flags);
    return bytes + 13;
}

size_t Speaker::load(const unsigned char* buffer, size_t size)
{
    size_t bytes = Object::load(buffer, size);
    if (bytes == 0 || size < bytes + 1) return 0;
    int version = ppl7::Peek8(buffer + bytes);
    if (version < 3) return 0;

    sample_id = static_cast<AudioEffect>(ppl7::Peek32(buffer + bytes + 1));
    max_distance = ppl7::Peek16(buffer + bytes + 5);
    volume = ppl7::PeekFloat(buffer + bytes + 7);
    int flags = ppl7::Peek16(buffer + bytes + 11);
    if (version == 1 && flags > 3) flags = flags & 1;
    // ppl7::PrintDebug("Speaker::load, flags=%d\n", flags);

    initial_state = (bool)flags & 1;
    sample_type = SampleType::AudioLoop;
    if (flags & 2) sample_type = SampleType::Effect;

    enabled = initial_state;

    if (audio) {
        getAudioPool().stopInstace(audio);
        delete audio;
        audio = NULL;
    }
    return size;
}

void Speaker::setSample(int id, float volume, int max_distance)
{
    AudioPool& pool = getAudioPool();
    if (audio) {
        pool.stopInstace(audio);
        delete audio;
        audio = NULL;
        sample_id = static_cast<AudioEffect>(0);
    }
    if (sample_type == SampleType::AudioLoop) {
        if (id > 0) {
            audio = pool.getInstance(static_cast<AudioLoop>(id), AudioClass::Ambience);
            if (audio) {
                audio->setVolume(volume);
                audio->setAutoDelete(false);
                audio->setLoop(true);
                audio->setPositional(p, max_distance);
                audio->startRandom();
                pool.playInstance(audio);
                sample_id = static_cast<AudioLoop>(id);
            }
        }
    } else {
        sample_id = static_cast<AudioEffect>(id);
    }
}

void Speaker::toggle(bool enable, Object* source)
{
    if (sample_type == SampleType::AudioLoop) {
        this->enabled = enable;
    } else {
        getAudioPool().playOnce(sample_id, p, max_distance, volume);
    }
}

void Speaker::trigger(Object* source)
{
    toggle(!enabled, source);
}

void Speaker::test()
{
    if (sample_type == SampleType::AudioLoop) {
        setSample(sample_id, volume, max_distance);
    } else {
        getAudioPool().playOnce(sample_id, p, max_distance, volume);
    }
}

class SpeakerDialog : public Dialog
{
private:
    ppltk::ComboBox* sample_name;
    ppltk::HorizontalSlider* max_distance;
    ppltk::DoubleHorizontalSlider* volume;
    ppltk::CheckBox *initial_state_checkbox, *current_state_checkbox;
    ppltk::RadioButton *type_audioloop, *type_effect;
    Speaker* object;

    void setupAudioLoop();
    void setupEffect();

public:
    SpeakerDialog(Speaker* object);
    ~SpeakerDialog();

    virtual void valueChangedEvent(ppltk::Event* event, int value);
    virtual void valueChangedEvent(ppltk::Event* event, int64_t value);
    virtual void valueChangedEvent(ppltk::Event* event, double value);
    virtual void toggledEvent(ppltk::Event* event, bool checked) override;
    void dialogButtonEvent(Dialog::Buttons button) override;
};

void Speaker::fillComboBoxWithEffects(ppltk::ComboBox* combobox, int selected_sample)
{
    combobox->clear();
    combobox->add("no sound", ppl7::ToString("%d", AudioEffect::none));
    combobox->add("impact", ppl7::ToString("%d", AudioEffect::impact1));
    combobox->add("Trap 1", ppl7::ToString("%d", AudioEffect::trap1));
    combobox->add("Trap 1", ppl7::ToString("%d", AudioEffect::trap2));
    combobox->add("Crystal", ppl7::ToString("%d", AudioEffect::crystal1));
    combobox->add("Coin 1", ppl7::ToString("%d", AudioEffect::coin1));
    combobox->add("Coin 2", ppl7::ToString("%d", AudioEffect::coin2));
    combobox->add("Arrow swoosh", ppl7::ToString("%d", AudioEffect::arrow_swoosh));
    combobox->sortItems();
    combobox->setCurrentIdentifier(ppl7::ToString("%d", selected_sample));
}

void Speaker::openUi()
{
    SpeakerDialog* dialog = new SpeakerDialog(this);
    GetGameWindow()->addChild(dialog);
}

SpeakerDialog::SpeakerDialog(Speaker* object)
    : Dialog(500, 280, Buttons::OK | Buttons::Test)
{
    ppl7::grafix::Rect client = clientRect();
    this->object = object;
    setWindowTitle(ppl7::ToString("Speaker, Object ID: %u", object->id));

    int y = 0;

    // State
    int sw = width() / 2;
    initial_state_checkbox = new ppltk::CheckBox(0, y, sw, 30, "initial State", object->initial_state);
    initial_state_checkbox->setEventHandler(this);
    addChild(initial_state_checkbox);
    current_state_checkbox = new ppltk::CheckBox(sw, y, sw, 30, "current State", object->enabled);
    current_state_checkbox->setEventHandler(this);
    addChild(current_state_checkbox);
    y += 35;

    type_audioloop = new ppltk::RadioButton(0, y, sw, 30, "Audioloop", (bool)(object->sample_type == Speaker::SampleType::AudioLoop));
    type_audioloop->setEventHandler(this);
    addChild(type_audioloop);

    type_effect = new ppltk::RadioButton(sw, y, sw, 30, "Effect", (bool)(object->sample_type == Speaker::SampleType::Effect));
    type_effect->setEventHandler(this);
    addChild(type_effect);

    y += 35;

    addChild(new ppltk::Label(0, y, 120, 30, "Sample: "));
    sample_name = new ppltk::ComboBox(120, y, client.width() - 120, 30);
    if (object->sample_type == Speaker::SampleType::AudioLoop)
        setupAudioLoop();
    else
        setupEffect();
    sample_name->setEventHandler(this);
    addChild(sample_name);
    y += 35;

    addChild(new ppltk::Label(0, y, 120, 30, "volume: "));
    volume = new ppltk::DoubleHorizontalSlider(120, y, client.width() - 120, 30);
    volume->setLimits(0.0f, 2.0f);
    volume->setValue(object->volume);
    volume->enableSpinBox(true, 0.01f, 3, 80);
    volume->setEventHandler(this);
    addChild(volume);
    y += 35;

    addChild(new ppltk::Label(0, y, 120, 30, "max_distance: "));
    max_distance = new ppltk::HorizontalSlider(120, y, client.width() - 120, 30);
    max_distance->setLimits(800, 8000);
    max_distance->enableSpinBox(true, 100, 80);
    max_distance->setValue(object->max_distance);
    max_distance->setEventHandler(this);
    addChild(max_distance);
    y += 35;
}

SpeakerDialog::~SpeakerDialog()
{
}

void SpeakerDialog::setupAudioLoop()
{
    sample_name->clear();
    sample_name->add("no sound", ppl7::ToString("%d", AudioLoop::none));
    sample_name->add("Birds 1", ppl7::ToString("%d", AudioLoop::birds1));
    sample_name->add("Birds 2", ppl7::ToString("%d", AudioLoop::birds2));
    sample_name->add("Birds 3", ppl7::ToString("%d", AudioLoop::birds3));
    sample_name->add("Birds in the rain", ppl7::ToString("%d", AudioLoop::birds_in_rain));
    sample_name->add("Cave 1", ppl7::ToString("%d", AudioLoop::cave1));
    sample_name->add("Cave 2", ppl7::ToString("%d", AudioLoop::cave2));
    sample_name->add("Cave 3", ppl7::ToString("%d", AudioLoop::cave3));
    sample_name->add("Cave 4", ppl7::ToString("%d", AudioLoop::cave4));
    sample_name->add("Desert at Night", ppl7::ToString("%d", AudioLoop::desert_at_night));
    sample_name->add("Electric", ppl7::ToString("%d", AudioLoop::electric));
    sample_name->add("Fire 1", ppl7::ToString("%d", AudioLoop::fire1));
    sample_name->add("Fire 2", ppl7::ToString("%d", AudioLoop::fire2));
    sample_name->add("Fire 3", ppl7::ToString("%d", AudioLoop::fire3));
    sample_name->add("Fire 4 - Gasburner", ppl7::ToString("%d", AudioLoop::fire4));
    sample_name->add("Fireworks loop", ppl7::ToString("%d", AudioLoop::fireworks_loop));
    sample_name->add("Jungle 1", ppl7::ToString("%d", AudioLoop::jungle1));
    sample_name->add("Jungle 2", ppl7::ToString("%d", AudioLoop::jungle2));

    sample_name->add("Lava loop 1", ppl7::ToString("%d", AudioLoop::lavaloop1));
    sample_name->add("Lava loop 2", ppl7::ToString("%d", AudioLoop::lavaloop2));
    sample_name->add("Lava bubbles", ppl7::ToString("%d", AudioLoop::lavabubbles));

    sample_name->add("Night 1", ppl7::ToString("%d", AudioLoop::night1));
    sample_name->add("Night 2", ppl7::ToString("%d", AudioLoop::night2));
    sample_name->add("Night 3", ppl7::ToString("%d", AudioLoop::night3));
    sample_name->add("Night 4", ppl7::ToString("%d", AudioLoop::night4));
    sample_name->add("Nightowl 1", ppl7::ToString("%d", AudioLoop::nightowl1));
    sample_name->add("Nightowl 2", ppl7::ToString("%d", AudioLoop::nightowl2));
    sample_name->add("Nightowl 3", ppl7::ToString("%d", AudioLoop::nightowl3));

    sample_name->add("Rain 1", ppl7::ToString("%d", AudioLoop::rain1));
    sample_name->add("Rain 2", ppl7::ToString("%d", AudioLoop::rain2));
    sample_name->add("Rain 3", ppl7::ToString("%d", AudioLoop::rain3));

    sample_name->add("Underwater", ppl7::ToString("%d", AudioLoop::underwaterloop1));

    sample_name->add("Waterflow 1", ppl7::ToString("%d", AudioLoop::waterflow1));
    sample_name->add("Waterflow 2", ppl7::ToString("%d", AudioLoop::waterflow2));
    sample_name->add("Waterflow 3", ppl7::ToString("%d", AudioLoop::waterflow3));
    sample_name->add("Waterdrips in a cave", ppl7::ToString("%d", AudioLoop::waterdrips1));
    sample_name->add("Waterdrips 2", ppl7::ToString("%d", AudioLoop::waterdrips2));
    sample_name->add("Waterdrips 3", ppl7::ToString("%d", AudioLoop::waterdrips3));
    sample_name->add("Water Bubbles 1", ppl7::ToString("%d", AudioLoop::water_bubble1));
    sample_name->add("Water Bubbles 2", ppl7::ToString("%d", AudioLoop::water_bubble2));
    sample_name->add("Water Bubbles 3", ppl7::ToString("%d", AudioLoop::water_bubble3));
    sample_name->add("Water Bubbles 4", ppl7::ToString("%d", AudioLoop::water_bubble4));
    sample_name->add("Water Bubbles 5", ppl7::ToString("%d", AudioLoop::water_bubble5));

    sample_name->add("Waves 1", ppl7::ToString("%d", AudioLoop::waves1));
    sample_name->add("Waves 2", ppl7::ToString("%d", AudioLoop::waves2));
    sample_name->add("Waves 3", ppl7::ToString("%d", AudioLoop::waves3));
    sample_name->add("Waves 4", ppl7::ToString("%d", AudioLoop::waves4));

    sample_name->add("Gas 1", ppl7::ToString("%d", AudioLoop::gas1));
    sample_name->add("Gas 2", ppl7::ToString("%d", AudioLoop::gas2));
    sample_name->add("Gas 3", ppl7::ToString("%d", AudioLoop::gas3));

    sample_name->add("Wind and crickets", ppl7::ToString("%d", AudioLoop::wind_crickets));
    sample_name->add("Wind strong", ppl7::ToString("%d", AudioLoop::wind_strong));
    sample_name->add("Wind Howling", ppl7::ToString("%d", AudioLoop::wind1));
    sample_name->add("Wind Desert", ppl7::ToString("%d", AudioLoop::wind2));
    sample_name->add("Wind soft 1", ppl7::ToString("%d", AudioLoop::wind3));
    sample_name->add("Wind soft 2", ppl7::ToString("%d", AudioLoop::wind4));

    sample_name->add("Earthquake", ppl7::ToString("%d", AudioLoop::earthquake));
    sample_name->add("Rumble", ppl7::ToString("%d", AudioLoop::rumble));
    sample_name->add("Waterpuddle", ppl7::ToString("%d", AudioLoop::waterpuddle));

    sample_name->sortItems();
    sample_name->setCurrentIdentifier(ppl7::ToString("%d", AudioLoop::none));
    if (object->sample_type == Speaker::SampleType::AudioLoop) sample_name->setCurrentIdentifier(ppl7::ToString("%d", object->sample_id));
}

void SpeakerDialog::setupEffect()
{
    Speaker::fillComboBoxWithEffects(sample_name, static_cast<int>(AudioEffect::none));
    if (object->sample_type == Speaker::SampleType::Effect) sample_name->setCurrentIdentifier(ppl7::ToString("%d", object->sample_id));
}

void SpeakerDialog::toggledEvent(ppltk::Event* event, bool checked)
{
    if (event->widget() == initial_state_checkbox) {
        object->initial_state = checked;
    } else if (event->widget() == current_state_checkbox) {
        object->enabled = checked;
    } else if (event->widget() == type_audioloop && checked == true) {
        if (object->sample_type != Speaker::SampleType::AudioLoop) {
            setupAudioLoop();
            object->sample_type = Speaker::SampleType::AudioLoop;
            object->sample_id = AudioLoop::none;
        }
    } else if (event->widget() == type_effect && checked == true) {
        if (object->sample_type != Speaker::SampleType::Effect) {
            setupEffect();
            object->sample_type = Speaker::SampleType::Effect;
            object->sample_id = AudioEffect::none;
        }
    }
}

void SpeakerDialog::valueChangedEvent(ppltk::Event* event, int value)
{
    // ppl7::PrintDebugTime("SpeakerDialog::valueChangedEvent (int): >>%d<<", value);
    if (event->widget() == sample_name) {
        object->sample_id = static_cast<AudioEffect>(sample_name->currentIdentifier().toInt());
        object->test();
    }
}

void SpeakerDialog::valueChangedEvent(ppltk::Event* event, int64_t value)
{
    // ppl7::PrintDebugTime("SpeakerDialog::valueChangedEvent (int64_t): >>%d<<", (int)value);
    if (event->widget() == max_distance) {
        object->max_distance = (int)value;
    }
}

void SpeakerDialog::valueChangedEvent(ppltk::Event* event, double value)
{
    // ppl7::PrintDebugTime("SpeakerDialog::valueChangedEvent (volume): >>%0.3f<<", value);
    if (event->widget() == volume) {
        object->volume = value;
    }
}

void SpeakerDialog::dialogButtonEvent(Dialog::Buttons button)
{
    if (button == Buttons::Test) object->test();
}

} // namespace Objects
