#ifndef FS_H
#define FS_H

#include <stdint.h>
#include <stddef.h>
#include <path>

struct DiskBlock;
struct Filesystem;
struct Dir;
struct File;

class Storage;

struct DirEntry {
  Filesystem* fs;
  uint64_t size;
  uint32_t flags;
  uint32_t inodeId;
  rodvin::string name;
  Dir* opendir();
  File* open();
};

extern DirEntry* sysroot;

struct Dir {
  virtual DirEntry *readdir() = 0;
  virtual ~Dir() {}
};

enum SeekPosition {
  FromStartOfFile,
  FromCurrentPosition,
  FromEndOfFile,
};

struct File {
  File(size_t size) 
  : position(0) 
  , size(size)
  {}
  virtual ~File() {}
  virtual size_t seek(int64_t position, SeekPosition location) = 0;
  virtual size_t filesize() = 0;
  virtual size_t read(uint8_t *buffer, size_t amount) = 0;
  virtual size_t write(const uint8_t *buffer, size_t amount) = 0;
protected:
  size_t position, size;
};

struct Filesystem {
  virtual ~Filesystem() {}
  virtual Dir* openrootdir() = 0;
  virtual Dir* opendir(DirEntry* entry) = 0;
  virtual File* open(DirEntry* entry) = 0;
};

struct Vfs {
  static DirEntry* lookup(const rodvin::path& name);
  static void Register(Filesystem* fs);
private:
  static Filesystem* fs;
};

#endif


