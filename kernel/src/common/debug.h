#ifndef DEBUG_H
#define DEBUG_H

#include <stddef.h>

class debug {
private:
  debug();
public:
  static debug& instance() { 
    static debug inst; 
    return inst; 
  }
  void write(const char* data, size_t length);
  bool readline(char *buffer, size_t limit);
};

#endif


