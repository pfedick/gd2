#include "ui/worldwidget.h"

WorldWidget::WorldWidget()
    : Widget::Widget()
{
    setClientOffset(0, 0, 0, 0);
    show_ui = false;
}

void WorldWidget::setViewport(const ppl7::grafix::Rect& viewport)
{
    // printf("WorldWidget::setViewport: %d x %d\n", viewport.width(), viewport.height());
    this->setPos(viewport.x1, viewport.y1);
    this->setSize(viewport.width(), viewport.height());
    this->needsRedraw();
}

ppl7::String WorldWidget::widgetType() const
{
    return "WorldWidget";
}

static void drawFrame(ppl7::grafix::Drawable& draw,
                      const ppl7::grafix::Point& p1,
                      const ppl7::grafix::Point& p2,
                      const ppl7::grafix::Color& light,
                      const ppl7::grafix::Color& dark)
{
    draw.line(p1.x, p1.y, p2.x, p1.y, dark);
    draw.line(p1.x, p1.y, p1.x, p2.y, dark);
    draw.line(p1.x, p2.y, p2.x, p2.y, light);
    draw.line(p2.x, p1.y, p2.x, p2.y, light);
}

void WorldWidget::setShowUi(bool enable)
{
    show_ui = enable;
}

void WorldWidget::paint(ppl7::grafix::Drawable& draw)
{
    // printf("WorldWidget::paint %d x %d\n", draw.width(), draw.height());
    //  draw.cls();
    if (!show_ui) return;
    ppl7::grafix::Point p0(1, 1);
    ppl7::grafix::Point p1(0, 0);
    ppl7::grafix::Point p2(draw.width(), draw.height());
    drawFrame(draw, p1, p2, ppl7::grafix::Color(192, 192, 196, 255), ppl7::grafix::Color(32, 32, 25, 255));
    p1 += p0;
    p2 -= p0;
    drawFrame(draw, p1, p2, ppl7::grafix::Color(192, 192, 196, 200), ppl7::grafix::Color(32, 32, 25, 200));
    p1 += p0;
    p2 -= p0;
    drawFrame(draw, p1, p2, ppl7::grafix::Color(192, 192, 196, 150), ppl7::grafix::Color(32, 32, 25, 150));
    p1 += p0;
    p2 -= p0;
    drawFrame(draw, p1, p2, ppl7::grafix::Color(192, 192, 196, 100), ppl7::grafix::Color(32, 32, 25, 100));
    p1 += p0;
    p2 -= p0;
    drawFrame(draw, p1, p2, ppl7::grafix::Color(192, 192, 196, 60), ppl7::grafix::Color(32, 32, 25, 60));
    // Widget::paint(draw);
}
