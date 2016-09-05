#include "Font.h"
#include "Xml.h"
#include "vfs.h"
#include "Image.h"
#include <path>

Font::glyph *readGlyph(XmlNode *node) {
  Font::glyph *g = new Font::glyph;
  g->x = strtol(node->attributes["x"].c_str(), NULL, 10);
  g->y = strtol(node->attributes["y"].c_str(), NULL, 10);
  g->w = strtol(node->attributes["width"].c_str(), NULL, 10);
  g->h = strtol(node->attributes["height"].c_str(), NULL, 10);
  g->xoff = strtol(node->attributes["xoffset"].c_str(), NULL, 10);
  g->yoff = strtol(node->attributes["yoffset"].c_str(), NULL, 10);
  g->xadv = strtol(node->attributes["xadvance"].c_str(), NULL, 10);
  return g;
}

Font::Font(const rodvin::path &filename)
{
  DirEntry* de = Vfs::lookup(filename);
  File* f = de->open();
  size_t length = f->filesize();
  uint8_t *data = new uint8_t[length];
  f->read(data, length);
  delete f;

  printf("font file read, parsing now\n");
  XmlNode *node = XmlRead((char *)data, length);
  printf("after xml parse\n");
  XmlNode *chars = nullptr;
  for (XmlNode *child : node->children) {
    if (child->type == "chars") 
      chars = child;
    else if (child->type == "pages") {
      XmlNode *first = child->children[0];
      rodvin::string textureName = first->attributes["file"];
      image = Image::ConstructFromFile(filename.parent_path() / textureName);
    } else if (child->type == "common") {
      height = strtol(child->attributes["lineHeight"].c_str(), NULL, 10);
    }
  }

  for (XmlNode *node : chars->children) {
    int id = strtol(node->attributes["id"].c_str(), NULL, 10);
    glyphs[id] = readGlyph(node);
  }
  delete node;
  delete [] data;
}

Font::~Font() {
  for (auto &p : glyphs) {
    delete p.second;
  }
}

size_t Font::getWidthFor(const rodvin::string& str) {
  // TODO: also use w/h
  // TODO: use some form of scaling
  size_t cx = 0;
  uint32_t lastChar = 0;
  for (uint32_t letter : str) {
    cx += getKerning(lastChar, letter);
    glyph *g = getGlyph(letter);
    lastChar = letter;
    if (!g) continue;
    cx += g->xadv;
  }
  return cx;
}


