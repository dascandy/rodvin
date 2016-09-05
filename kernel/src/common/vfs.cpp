#include "vfs.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <path>

DirEntry* Vfs::lookup(const rodvin::path &name) {
  rodvin::string dn = name.string();
  Dir* current = fs->openrootdir();
  size_t start = 1;
  size_t pos = dn.find("/", start);
  if (dn.size() == 1) {
    DirEntry *ent = new DirEntry;
    ent->fs = fs;
    ent->inodeId = 0xFFFFFFFF;
    return ent;
  }
  while (pos != dn.npos) {
    rodvin::string filename = dn.substr(start, pos - start);
    start = pos + 1;
    DirEntry* e;
    while ((e = current->readdir())) {
      if (e->name == filename) {
        Dir* d = e->opendir();
        delete e;
        delete current;
        current = d;
        break;
      }
      delete e;
    }
    if (!e) {
      // Can't find part of path
      delete current;
      return NULL;
    }
    pos = dn.find("/", start+1);
  }
  rodvin::string filename = dn.substr(start);
  DirEntry* e;
  while ((e = current->readdir())) {
    printf("found file %s compare to %s\n", e->name.c_str(), filename.c_str());
    if (e->name == filename) {
      delete current;
      return e;
    }
  }
  // Can't find part of path
  delete current;
  return NULL;
}

void Vfs::Register(Filesystem* fs) {
  printf("root vfs set\n");
  Vfs::fs = fs;
}

Dir* DirEntry::opendir() {
  if (inodeId == 0xFFFFFFFF) {
    return fs->openrootdir();
  }
  return fs->opendir(this);
}

File* DirEntry::open() {
  return fs->open(this);
}

Filesystem* Vfs::fs;


