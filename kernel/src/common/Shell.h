#ifndef SHELL_H
#define SHELL_H

#include "Console.h"
#include <path>

class Dir;

class Shell {
public:
  Shell();
  void run();
  void handleChar(uint32_t f);
  void execute();
  void executeEcho(const std::vector<rodvin::string>& args);
  void executeLs(const std::vector<rodvin::string>& args);
  void executeCd(const std::vector<rodvin::string>& args);
  void executeCat(const std::vector<rodvin::string>& args);

  Console con;
  rodvin::path currentPath;
  rodvin::string currentCmd;
  vectormap<rodvin::string, void (Shell::*)(const std::vector<rodvin::string>&)> functions;
  vectormap<rodvin::string, rodvin::string> env;
};

#endif


