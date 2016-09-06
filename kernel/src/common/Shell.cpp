#include "Shell.h"
#include "version.h"
#include "Keyboard.h"
#include "device.h"
#include "vfs.h"
#include "interrupts.h"

Shell::Shell() 
: con(1280, 720)
, currentPath("/")
{
  functions["echo"] = &Shell::executeEcho;
  functions["ls"] = &Shell::executeLs;
  functions["cd"] = &Shell::executeCd;
  functions["cat"] = &Shell::executeCat;

  con.printtext(getVersion());
  con.printtext("\nu@localhost:");
  con.printtext(currentPath.string());
  con.printtext("$ ");
}

void Shell::run() {
  handleChar(getchar());
}

void Shell::handleChar(uint32_t c)
{
  switch(c) {
  case '\n':
    con.printtext("\n");
    execute();
    con.printtext("u@localhost:");
    con.printtext(currentPath.string());
    con.printtext("$ ");
    currentCmd.clear();
    break;
  case '\b':
    {
      currentCmd.pop_back();
      uint32_t t[2] = { c, 0 };
      con.printtext(rodvin::string(t));
    }
    break;
  default:
    {
      currentCmd.push_back(c);
      uint32_t t[2] = { c, 0 };
      con.printtext(rodvin::string(t));
    }
    break;
  }
}

static std::vector<rodvin::string> split(const rodvin::string& str) {
  std::vector<rodvin::string> v;
  auto it = str.begin();
  while (it != str.end() && *it == ' ') ++it;
  auto cur = it;
  while (it != str.end()) {
    if (*it == ' ') {
      v.emplace_back(cur, it);
      while (*it == ' ') ++it;
      cur = it;
    } else {
      ++it;
    }
  }
  v.emplace_back(cur, it);
  return v;
}

void Shell::execute() {
  std::vector<rodvin::string> args = split(currentCmd);
  if (args.empty()) return;
  auto it = functions.find(args[0]);
  if (it != functions.end()) {
    (this->*(it->second))(args);
  } else {
    con.printtext("Unknown command ");
    con.printtext(args[0]);
    con.printtext("\n");
  }
}

void Shell::executeEcho(const std::vector<rodvin::string>& args) {
  auto it = args.begin();
  ++it;
  for (; it != args.end(); ++it) {
    con.printtext(*it);
    con.printtext(" ");
  }
  con.printtext("\n");
}

void Shell::executeLs(const std::vector<rodvin::string>& args) {
  rodvin::path p = currentPath;
  Dir *d;
  if (args.size() > 1) {
    if (*args[1].begin() == '/') {
      p = rodvin::path(args[1]).canonical();
    } else {
      p = (currentPath / args[1]).canonical();
    }
  }
  DirEntry* ent = Vfs::lookup(p);
  if (!ent) {
    con.printtext("Path not found: ");
    con.printtext(p.string());
    con.printtext("\n");
    return;
  }
  d = ent->opendir();
  delete ent;
  DirEntry* e;
  while ((e = d->readdir())) {
    con.printtext(e->name);
    con.printtext("\n");
    delete e;
  }
  delete d;
}

void Shell::executeCd(const std::vector<rodvin::string>& args) {
  if (args.size() == 1) {
    con.printtext("No home dir yet\n");
    return;
  }
  rodvin::path old = currentPath;
  if (*args[1].begin() == '/') {
    currentPath = rodvin::path(args[1]).canonical();
  } else {
    currentPath = (currentPath / args[1]).canonical();
  }

  DirEntry* ent = Vfs::lookup(currentPath);
  if (ent) {
    delete ent;
  } else {
    con.printtext("Path not found: ");
    con.printtext(currentPath.string());
    con.printtext("\n");
    currentPath = old;
  }
}

void Shell::executeCat(const std::vector<rodvin::string>& args) {
  rodvin::path p;
  if (*args[1].begin() == '/') {
    p = rodvin::path(args[1]).canonical();
  } else {
    p = (currentPath / args[1]).canonical();
  }
  DirEntry* ent = Vfs::lookup(p);
  if (!ent) {
    con.printtext("Path not found: ");
    con.printtext(p.string());
    con.printtext("\n");
    return;
  }
  
  File* f = ent->open();
  char *buf = new char[f->filesize()+1];
  f->read((uint8_t*)buf, f->filesize());
  buf[f->filesize()] = 0;
  con.printtext(buf);
  delete [] buf;
  delete f;
  con.printtext("\n");
}


