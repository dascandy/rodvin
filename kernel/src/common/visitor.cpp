#include "ui.h"

void Visitor::Visit(UIElement* ) {
  printf("visit uielem\n");
}

void Visitor::Visit(CompositeUIElement *win) {
  printf("visit cuielem\n");
  Visit(static_cast<UIElement*>(win));
}

void Visitor::Visit(Window *win) {
  printf("visit window\n");
  Visit(static_cast<CompositeUIElement*>(win));
}


