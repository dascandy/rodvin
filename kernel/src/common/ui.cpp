#include "ui.h"
#include <stdio.h>
#include "math.h"
#include "Framebuffer.h"

Window::Window(float x, float y, float w, float h)
: background(Color(1.0f, 1.0f, 0.0f))
, alpha(1.0f)
{
  UIRoot::Instance().Add(*this);
  this->x = x;
  this->y = y;
  this->w = w;
  this->h = h;
}

class DrawVisitor : public Visitor {
public:
  virtual void Visit(CompositeUIElement *win) {
    for (const auto& p : win->elements) {
      p->Visit(this);
    }
  }
  virtual void Visit(Window *win) override {
    float x = *win->x, y = *win->y, w = *win->w, h = *win->h;
    int x1 = round(x - w/2);
    int y1 = round(y - h/2);

    draw_square(x1, y1, w, h, *win->background);
  }
};

void UIRoot::Draw() {
  DrawVisitor dv;
  dv.Visit(this);
}


