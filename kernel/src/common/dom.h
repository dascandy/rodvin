#ifndef DOM_H
#define DOM_H

#include <vector>
#include <vectormap>
#include <string>
#include <string.h>

class Element;

class Node {
public:
  enum nodetype {
    Text,
    Element,
  } type;
  Node(::Element *parent, nodetype type)
  : parent(parent)
  , type(type)
  {}
  virtual ~Node() {}
  ::Element *parent;
};

class TextNode : public Node {
public:
  TextNode(::Element *parent, const rodvin::string &text)
  : Node(parent, Node::Text)
  , text(text)
  {}
  rodvin::string text;
};

class Element : public Node {
public:
  Element(Element *parent, const rodvin::string &tagname)
  : Node(parent, Node::Element)
  , tagname(tagname)
  {}
  ~Element() {
    for (Node *n : children) {
      delete n;
    }
  }
  Element *getSibling(Element *node) {
    int offset = 0;
    while (children[offset] != node && offset != children.size()) offset++;
    if (offset == 0 || offset == children.size()) return NULL;

    offset--;
    while (offset >= 0 && children[offset]->type != Node::Element) offset--;
    if (offset < 0) return NULL;
    return (Element *)children[offset];
  }
  rodvin::string tagname;
  vectormap<rodvin::string, rodvin::string> attributes;
  std::vector<Node *> children;
};

#endif


