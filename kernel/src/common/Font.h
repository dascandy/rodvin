#ifndef FONT_H
#define FONT_H

#include <vectormap>
#include <string>
#include <path>

class Image;

class Font {
public:
  Font(const rodvin::path& filename);
  ~Font();
  struct glyph {
      int x, y, w, h, xoff, yoff, xadv;
  };
  glyph* getGlyph(uint32_t id) const {
    auto it = glyphs.find(id);
    if (it != glyphs.end()) return it->second;
    return NULL;
  }
  uint32_t getKerning(uint32_t first, uint32_t second) const {
    auto it = kernings.find(std::pair<uint32_t, uint32_t>(first, second));
    if (it != kernings.end()) return it->second;
    return 0;
  }
  size_t getWidthFor(const rodvin::string& str);
  Image *image;
  size_t height;
private:
  vectormap<uint32_t, glyph *> glyphs;
  vectormap<std::pair<uint32_t, uint32_t>, int> kernings;
};

#endif


