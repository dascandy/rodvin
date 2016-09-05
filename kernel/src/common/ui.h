#ifndef UI_H
#define UI_H

#include <vector>
#include "Var.h"
#include "Color.h"
#include <stdio.h>

class Visitor;
class Window;

class UIElement {
public:
  virtual ~UIElement() {}
  virtual void Visit(Visitor* v) = 0;
  Var<float> x, y, w, h;
};

class CompositeUIElement : public UIElement {
public:
  std::vector<UIElement*> elements;
};

class Visitor {
public:
  virtual void Visit(UIElement* elem);
  virtual void Visit(CompositeUIElement *win);
  virtual void Visit(Window *win);
};

class Window : public CompositeUIElement {
private:
  void Visit(Visitor* v) override { v->Visit(this); }
public:
  Window(float x, float y, float w, float h);
  Var<Color> background;
  Var<float> alpha;
};

class UIRoot : public CompositeUIElement {
public:
  void Visit(Visitor* v) { v->Visit(this); }
  static UIRoot& Instance() { static UIRoot root; return root; }
  void Draw();
  void Add(Window& elem) {
    elements.push_back(&elem);
  }
};

#endif


