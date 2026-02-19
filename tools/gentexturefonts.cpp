#include <ppl7.h>
#include <ppl7-grafix.h>

int main(int argc, char* argv[])
{
    ppl7::WideString letters = L"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789ÄÖÜäöüß,.-;:_#'+*~^°!§$%&/()[]\\=?<>|@€";

    ppl7::grafix::Grafix grafix;
    ppl7::grafix::ImageFilter_PNG png;

    int font_size = 26;
    grafix.loadFont("tmp/Source_Code_Pro/static/SourceCodePro-Regular.ttf", "SCP");

    ppl7::grafix::Font font;
    font.setName("SCP");
    font.setOrientation(static_cast<ppl7::grafix::Font::Orientation>(static_cast<int>(ppl7::grafix::Font::Orientation::TOP) |
                                                                     static_cast<int>(ppl7::grafix::Font::Orientation::LEFT)));
    font.setSize(font_size);
    font.setAntialias(true);
    font.setColor(ppl7::grafix::Color(255, 255, 255, 255));

    ppl7::grafix::Image img(font_size * 6, font_size * 4, ppl7::grafix::RGBFormat::A8R8G8B8);

    for (size_t i = 0; i < letters.length(); i++) {
        img.cls(ppl7::grafix::Color(0, 0, 0, 0));
        ppl7::WideString s = letters.mid(i, 1);
        ppl7::PrintDebug("Rendering character: %lc\n", s[0]);
        font.setColor(ppl7::grafix::Color(0, 0, 0, 255));
        for (int y = 0; y <= 4; y++) {
            for (int x = 0; x <= 4; x++) {
                img.print(font, x, y + font_size, s);
            }
        }
        font.setColor(ppl7::grafix::Color(255, 255, 255, 255));
        img.print(font, 2, font_size + 2, s);
        png.saveFile(ppl7::ToString("res/fonts/scp/frame_%05u.png", s[0]), img);
    }

    return 0;
}
