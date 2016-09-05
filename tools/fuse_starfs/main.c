#define FUSE_USE_VERSION 26
#define _LARGEFILE64_SOURCE

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <fuse.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include "starfs.h"

struct inode *inodetab;
struct fsheader fshdr;
uint64_t filecount = 0;
int fd;

static void readfromextent(struct extent *ext, char *buffer, uint64_t offset, uint64_t size) {
  lseek64(fd, offset + ((ext->startblock) << 12), SEEK_SET);
  read(fd, buffer, size);
}

static void readfromextents(struct extent *ext, char *buffer, uint64_t offset, uint64_t size) {
  while (size > 0 && ext->length > 0) {
    if (ext->length << 12 > offset) {
      size_t fromthis = size;
      if (size > (ext->length << 12) - offset) 
        fromthis = (ext->length << 12) - offset;
      readfromextent(ext, buffer, offset, fromthis);
      size -= fromthis;
      buffer += fromthis;
    }

    if (offset < ext->length << 12)
      offset = 0;
    else 
      offset -= ext->length << 12;

    ext++;
  }
}

static uint64_t sizefromextents(struct extent *ext) {
  uint64_t size = 0;
  while (ext->length > 0) {
    size += ext->length << 12;
    ext++;
  }
  return size;
}

static struct extent *readbottomextents(uint64_t ino) {
  struct extent *ext = malloc(sizeof(struct extent) * 2);
  ext[0].startblock = inodetab[ino].ext.startblock;
  ext[0].length = inodetab[ino].ext.length;
  ext[1].startblock = 0;
  ext[1].length = 0;
  int indirs = inodetab[ino].flags & INO_EXTENT_INDIRECTION;
  while (indirs) {
    struct extent *newext = malloc(sizefromextents(ext));
    readfromextents(ext, (char *)newext, 0, sizefromextents(ext));
    free(ext);
    ext = newext;
    indirs--;
  }
  return ext;
}

static void set_file_extents(uint64_t ino, struct extent *ext) {
  size_t n = 0;
  while (ext[n].length) n++;
  if (n == 1) {
    inodetab[ino].ext.startblock = ext->startblock;
    inodetab[ino].ext.length = ext->length;
  } else {
    // TO BE DONE
    abort();
  }
}

static char *readinode(uint64_t ino) {
  char *buf = (char *)malloc(inodetab[ino].filesize);
  struct extent *ext = readbottomextents(ino);
  readfromextents(ext, buf, 0, inodetab[ino].filesize);
  free(ext);
  return buf;
}

uint64_t alloc_space(uint64_t blockcount) {
  struct extent *exts = readbottomextents(4);
  for (size_t n = 0; exts[n].length; n++) {
    if (exts[n].length >= blockcount) {
      uint64_t start = exts[n].startblock;
      exts[n].length -= blockcount;
      exts[n].startblock += blockcount;
      set_file_extents(4, exts);
      return start;
    }
  }
  return INVALID_BLOCK;
}

// Most likely called: to extend a current bit of space
uint64_t alloc_adjacent_space(uint64_t location, uint64_t blockcount) {
  struct extent *exts = readbottomextents(4);
  for (size_t n = 0; exts[n].length; n++) {
    if (exts[n].startblock <= location &&
        exts[n].startblock + exts[n].length >= location + blockcount) {
      if (exts[n].startblock != location) {
        abort();
      }
      exts[n].length -= blockcount;
      exts[n].startblock += blockcount;
      set_file_extents(4, exts);
      return location;
    }
  }
  return INVALID_BLOCK;
}

void free_space(uint64_t block, uint64_t length) {
  (void)block;
  (void)length;
  // TBD
}

uint64_t alloc_inode() {
  for (size_t n = 0; n < inodetab[2].filesize / sizeof(struct inode); n++) {
    if (inodetab[n].linkcount == 0) {
      inodetab[n].ext.startblock = INVALID_BLOCK;
      inodetab[n].ext.length = 0;
      inodetab[n].filesize = 0;
      inodetab[n].flags = 0;
      inodetab[n].linkcount = 1;
      return n;
    }
  }
  return INVALID_INODE;
}

void free_inode(uint64_t ino) {
  struct extent *ext = malloc(sizeof(struct extent) * 2);
  ext[0].startblock = inodetab[ino].ext.startblock;
  ext[0].length = inodetab[ino].ext.length;
  ext[1].startblock = 0;
  ext[1].length = 0;
  int indirs = inodetab[ino].flags & INO_EXTENT_INDIRECTION;
  while (indirs) {
    struct extent *newext = malloc(sizefromextents(ext));
    readfromextents(ext, (char *)newext, 0, sizefromextents(ext));
    for (int n = 0; ext[n].length != 0; n++) {
      free_space(ext[n].startblock, ext[n].length);
    }
    free(ext);
    ext = newext;
    indirs--;
  }
  for (int n = 0; ext[n].length != 0; n++) {
    free_space(ext[n].startblock, ext[n].length);
  }
  free(ext);
}

bool writeinode(uint64_t ino, const char *buffer, size_t size) {
  if (inodetab[ino].flags & INO_SYSTEM) {
    // System files may not be moved or resized
    if (size > inodetab[ino].filesize) {
      return false;
    }
  } else {
    // So far, only support single-extent files
    if (size == 0) {
      if (inodetab[ino].filesize) {
        free_space(inodetab[ino].ext.startblock, inodetab[ino].ext.length);
        inodetab[ino].ext.startblock = INVALID_BLOCK;
        inodetab[ino].ext.length = 0;
      }
    } else if (size > inodetab[ino].ext.length << 12) {
      uint64_t addedspace = ((size + 4095) >> 12) - inodetab[ino].ext.length;
      if (inodetab[ino].ext.startblock != INVALID_BLOCK &&
          alloc_adjacent_space(inodetab[ino].ext.startblock, addedspace)) {
        inodetab[ino].ext.length += addedspace;
      } else {
        // allocate a new chunk that's big enough
        uint64_t startblock = alloc_space((size + 4095) >> 12);
        if (inodetab[ino].ext.startblock != INVALID_BLOCK)
          free_space(inodetab[ino].ext.startblock, inodetab[ino].ext.length);
        inodetab[ino].ext.startblock = startblock;
        inodetab[ino].ext.length = ((size + 4095) >> 12);
      }
    } 
    inodetab[ino].filesize = size;
  }
  lseek64(fd, inodetab[ino].ext.startblock << 12, SEEK_SET);
  printf("write inode %lu %lu %lu\n", ino, inodetab[ino].ext.startblock, size);
  write(fd, buffer, size);
  return true;
}

static uint64_t find_in_dir(uint64_t ino, const char *name) {
  if ((inodetab[ino].flags & INO_DIRECTORY) == 0) 
    return INVALID_INODE;

  char *buffer = readinode(ino);
  struct dirformat *fmt = (struct dirformat *)buffer;
  struct direntry *ent = (struct direntry *)(buffer + 8);

  if (fmt->formatid != 1)
    return INVALID_INODE;

  for (uint32_t entry = 0; entry != fmt->entrycount; entry++) {
    if (strcmp(ent->filename, name) == 0) {
      uint64_t ino = ent->inodeno;
      free(buffer);
      return ino;
    }
    ent = (struct direntry *)((char *)ent + ent->filenamelength + 6);
  }

  free(buffer);
  return INVALID_INODE;
}

static bool remove_from_dir(uint64_t ino, const char *name) {
  if ((inodetab[ino].flags & INO_DIRECTORY) == 0) 
    return false;

  char *buffer = readinode(ino);
  struct dirformat *fmt = (struct dirformat *)buffer;
  struct direntry *ent = (struct direntry *)(buffer + 8);

  if (fmt->formatid != 1) {
    free(buffer);
    return false;
  }

  uint32_t entry;
  for (entry = 0; entry != fmt->entrycount; entry++) {
    ent = (struct direntry *)((char *)ent + ent->filenamelength + 6);
    if (strcmp(ent->filename, name) == 0) {
      break;
    }
  }

  if (entry == fmt->entrycount) { // Not found.
    free(buffer);
    return false;
  }

  size_t toremove = ent->filenamelength + 6;
  char *remainder = (char *)ent + toremove;
  char *endptr = buffer + inodetab[ino].filesize;
  memmove(ent, remainder, endptr - remainder);

  writeinode(ino, buffer, inodetab[ino].filesize - toremove);

  free(buffer);

  inodetab[ino].linkcount--;
  if (inodetab[ino].linkcount == 0) {
    free_inode(ino);
  }

  return true;
} 

static bool add_to_dir(uint64_t ino, const char *name, uint64_t newino) {
  printf("add_to_dir ino=%lu name=%s newino=%lu ", ino, name, newino);
  fflush(stdout);
  if ((inodetab[ino].flags & INO_DIRECTORY) == 0) {
    printf("Not a directory\n");
    return false;
  }

  char *buffer = readinode(ino);
  struct dirformat *fmt = (struct dirformat *)buffer;
  struct direntry *ent = (struct direntry *)(buffer + 8);

  printf("x\n");
  fflush(stdout);
  if (fmt->formatid != 1) {
    printf("invalid format dir\n");
    free(buffer);
    return false;
  }

  for (uint32_t entry = 0; entry != fmt->entrycount; entry++) {
    printf("Y\n");
  fflush(stdout);
    ent = (struct direntry *)((char *)ent + ent->filenamelength + 6);
    if (strcmp(ent->filename, name) == 0) { // already exists
      printf("already exists in dir\n");
      free(buffer);
      return false;
    }
  }

  char *newbuffer = malloc(7 + strlen(name) + inodetab[ino].filesize);
  memcpy(newbuffer, buffer, inodetab[ino].filesize);
  free(buffer);

  ((struct dirformat *)newbuffer)->entrycount++;
  ent = (struct direntry *)(newbuffer + inodetab[ino].filesize);
  ent->inodeno = newino;
  ent->filenamelength = strlen(name) + 1;
  strcpy(ent->filename, name);

  printf("writeinode tobecalled %lu %p %lu\n", ino, newbuffer, inodetab[ino].filesize + 7 + strlen(name));

  writeinode(ino, newbuffer, inodetab[ino].filesize + 7 + strlen(name));
  free(newbuffer);
  printf("OK\n");
  return true;
}

static uint64_t getinode(const char *path) {
  if (path[0] == 0) 
    return 3;

  char *p = strdup(path+1);
  char *c = p;
  uint64_t ino = 3;
  while (ino != INVALID_INODE) {
    if (*c == '\0') {
      free(p);
      return ino;
    }
    char *e = strstr(c, "/");
    if (e) {
      *e++ = 0;
      ino = find_in_dir(ino, c);
    } else {
      ino = find_in_dir(ino, c);
      free(p);
      return ino;
    }
    c = e;
  }
  return INVALID_INODE;
}

static uint64_t getparentinode(const char *path) {
  char *npath = strdup(path);
  char *lastslash = strrchr(npath, '/');
  *lastslash = 0;
  uint64_t ino = getinode(npath);
  free(npath);
  return ino;
}

// Describes what a file is.
static int starfs_getattr(const char *path, struct stat *stbuf)
{
  uint64_t ino = getinode(path);
  if (ino == INVALID_INODE) 
    return -ENOENT;

  printf("getattr for %s -> inode %lu\n", path, ino);

  stbuf->st_dev = 0;
  stbuf->st_ino = ino;
  stbuf->st_mode = 0644 | ((inodetab[ino].flags & INO_DIRECTORY) ? S_IFDIR : S_IFREG);
  stbuf->st_nlink = inodetab[ino].linkcount;
  stbuf->st_uid = 0;
  stbuf->st_gid = 0;
  stbuf->st_rdev = 0;
  stbuf->st_size = inodetab[ino].filesize;
  stbuf->st_blksize = 4096;
  stbuf->st_blocks = 8;
  stbuf->st_atime = 0;
  stbuf->st_mtime = 0;
  stbuf->st_ctime = 0;

  return 0;
}

static int starfs_fgetattr(const char *path, struct stat *stbuf,
struct fuse_file_info *fi)
{
  (void)fi;
  return starfs_getattr(path, stbuf);
}

struct starfs_dirp {
  struct inode *i;
  size_t offset;
  size_t entryleft;
};

// Opens a dir for reading
static int starfs_opendir(const char *path, struct fuse_file_info *fi)
{
  uint64_t ino = getinode(path);
  if (ino == INVALID_INODE)
    return -ENOENT;

  struct starfs_dirp *d = malloc(sizeof(struct starfs_dirp));
  if (d == NULL)
    return -ENOMEM;
   
  d->i = &inodetab[ino];
  d->offset = 8;

  struct dirformat fmt;

  lseek64(fd, d->i->ext.startblock << 12, SEEK_SET);
  read(fd, &fmt, sizeof(fmt));
  if (fmt.formatid != 1)
    return -EINVAL;

  d->entryleft = fmt.entrycount;
  fi->fh = (unsigned long) d;
  return 0;
}

// Read next entry from dir
static int starfs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
off_t offset, struct fuse_file_info *fi)
{
  (void)offset;
  (void) path;
  struct starfs_dirp *d = (struct starfs_dirp *) (uintptr_t) fi->fh;

  char buffer[1024];
  struct direntry *ent = (struct direntry *)buffer;

  while (d->entryleft) {
    lseek64(fd, (d->i->ext.startblock << 12) + d->offset, SEEK_SET);
    read(fd, ent, 6);
    read(fd, ent->filename, ent->filenamelength);

    d->offset += 6 + ent->filenamelength;
    d->entryleft--;

    struct stat st;
    memset(&st, 0, sizeof(st));
    st.st_ino = ent->inodeno;
    st.st_mode = S_IFREG | 0644;
    off_t nextoff = 0;
    if (filler(buf, ent->filename, &st, nextoff))
      break;
  }
  
  return 0;
}

// Close resources again
static int starfs_releasedir(const char *path, struct fuse_file_info *fi)
{
  struct starfs_dirp *d = (struct starfs_dirp *) (uintptr_t) fi->fh;
  (void) path;
  free(d);
  return 0;
}

// Create directory
static int starfs_mkdir(const char *path, mode_t mode)
{
  (void)mode;

  uint64_t ino = getparentinode(path);
  uint64_t newino = alloc_inode();
  char *newname = strrchr(path, '/') + 1;
  if (!add_to_dir(ino, newname, newino)) {
    free_inode(newino);
    return -EEXIST;
  }
  inodetab[newino].flags = INO_DIRECTORY;

  struct dirformat hdr;
  hdr.formatid = 1;
  hdr.entrycount = 0;

  writeinode(newino, (char *)&hdr, sizeof(hdr));
  return 0;
}

// Delete file
static int starfs_unlink(const char *path)
{
  uint64_t ino = getinode(path);
  if (inodetab[ino].flags & (INO_DIRECTORY | INO_SYSTEM)) // Can't remove any of these using unlink
    return -EACCES;

  uint64_t parent = getparentinode(path);
  char *filename = strrchr(path, '/') + 1;
  
  remove_from_dir(parent, filename);
  return 0;
}

// Remove directory (that must be empty)
static int starfs_rmdir(const char *path)
{
  uint64_t ino = getinode(path);
  if (inodetab[ino].filesize != 8 ||
      inodetab[ino].flags & INO_SYSTEM ||
      (inodetab[ino].flags & INO_DIRECTORY) != INO_DIRECTORY)
    return -EACCES;

  uint64_t parent = getparentinode(path);
  char *filename = strrchr(path, '/') + 1;
  
  remove_from_dir(parent, filename);
  return 0;
}

// Add another link to a given file
static int starfs_link(const char *from, const char *to)
{
  uint64_t pf = getparentinode(from),
           pt = getparentinode(to);
  const char *fn = strrchr(from, '/') + 1,
             *tn = strrchr(to, '/') + 1;
  uint64_t ino = getinode(from);

  remove_from_dir(pt, tn); // if it exists
  add_to_dir(pt, tn, ino);
  remove_from_dir(pf, fn);
  return 0;
}

// Create file (open)
static int starfs_create(const char *path, mode_t mode, struct fuse_file_info *fi)
{
  (void)mode;

  uint64_t ino = getparentinode(path);
  uint64_t newino = alloc_inode();
  char *newname = strrchr(path, '/') + 1;
  if (!add_to_dir(ino, newname, newino)) {
    free_inode(newino);
    return -EEXIST;
  }
  inodetab[newino].flags = 0;
  inodetab[newino].linkcount++;
  fi->fh = newino;

  return 0;
}

// Open file (open)
static int starfs_open(const char *path, struct fuse_file_info *fi)
{
  uint64_t ino = getinode(path);
  if (ino == INVALID_INODE)
    return -ENOENT;

  inodetab[ino].linkcount++;
  fi->fh = ino;
  return 0;
}

// Read from file
static int starfs_read(const char *path, char *buf, size_t size, off_t offset,
struct fuse_file_info *fi)
{
  (void)path;
  struct inode *i = &inodetab[fi->fh];
  readfromextent(&i->ext, buf, offset, size);
  return size;
}

// Write to file
static int starfs_write(const char *path, const char *buf, size_t size,
off_t offset, struct fuse_file_info *fi)
{
  (void)fi;
  uint64_t ino = getinode(path);
  char *buffer = readinode(ino);
  uint64_t newsize = inodetab[ino].filesize;
  if (newsize < size + offset) {
    newsize = size + offset;
    char *newbuf = (char *)malloc(newsize);
    memcpy(newbuf, buffer, inodetab[ino].filesize);
    free(buffer);
    buffer = newbuf;
  }
  memcpy(buffer + offset, buf, size);
  writeinode(ino, buffer, newsize);
  return size;
}

static int starfs_release(const char *path, struct fuse_file_info *fi)
{
  (void) path;
  inodetab[fi->fh].linkcount--;
  return 0;
}

// Get filesystem statistics
static int starfs_statfs(const char *path, struct statvfs *stbuf)
{
  (void)path;

  stbuf->f_bsize = 4096;
  stbuf->f_frsize = 4096;
  stbuf->f_blocks = fshdr.blockcount;
  stbuf->f_bavail = stbuf->f_bfree = inodetab[4].filesize / 4096;
  stbuf->f_files = filecount;
  stbuf->f_favail = stbuf->f_ffree = (inodetab[2].filesize / sizeof(struct inode)) - filecount;
  stbuf->f_fsid = 42;
  stbuf->f_flag = 0;
  stbuf->f_namemax = 65535;

  return 0;
}

// Rename/move file from A to B, implement with link + unlink
static int starfs_rename(const char *from, const char *to)
{
  return (starfs_link(from, to) || starfs_unlink(from)) ? -EACCES : 0;
}

static int starfs_truncate(const char *path, off_t size)
{
  uint64_t ino = getinode(path);
  if (ino == INVALID_INODE) 
    return -ENOENT;

  if (inodetab[ino].flags & INO_SYSTEM)
    return 0;

  if (size < (off_t)inodetab[ino].filesize) {
    char *buf = readinode(ino);
    writeinode(ino, buf, size);
    free(buf);
  }
  return 0;
}

static int starfs_ftruncate(const char *path, off_t size,
struct fuse_file_info *fi)
{
  (void)path;
  uint64_t ino = fi->fh;
  if (size < (off_t)inodetab[ino].filesize) {
    char *buf = readinode(ino);
    writeinode(ino, buf, size);
    free(buf);
  }
  return 0;
}

/*
 * UNIMPLEMENTED FUNCTIONS:
 */
static int starfs_access(const char *path, int mask)
{
  (void)path;
  return !(mask & X_OK);
}

static int starfs_readlink(const char *path, char *buf, size_t size)
{
  (void)path;
  (void)buf;
  (void)size;
  return -EINVAL;
}

static int starfs_symlink(const char *from, const char *to)
{
  (void)from;
  (void)to;
  return -EACCES;
}

static int starfs_mknod(const char *path, mode_t mode, dev_t rdev)
{
  (void)path;
  (void)mode;
  (void)rdev;
  return -EACCES;
}

static int starfs_chmod(const char *path, mode_t mode)
{
  (void)path;
  (void)mode;
  return 0;
}

static int starfs_chown(const char *path, uid_t uid, gid_t gid)
{
  (void)path;
  (void)uid;
  (void)gid;
  return 0;
}

static int starfs_utimens(const char *path, const struct timespec ts[2])
{
  (void)path;
  (void)ts;
  return 0;
}


static int starfs_flush(const char *path, struct fuse_file_info *fi)
{
  (void) path;
  (void) fi;

  return 0;
}

static int starfs_fsync(const char *path, int isdatasync,
struct fuse_file_info *fi)
{
  (void) path;
  (void)isdatasync;
  (void)fi;
  return 0;
}

static struct fuse_operations starfs_oper = {
  .getattr  = starfs_getattr,
  .fgetattr = starfs_fgetattr,
  .access = starfs_access,
  .readlink = starfs_readlink,
  .opendir  = starfs_opendir,
  .readdir  = starfs_readdir,
  .releasedir = starfs_releasedir,
  .mknod  = starfs_mknod,
  .mkdir  = starfs_mkdir,
  .symlink  = starfs_symlink,
  .unlink = starfs_unlink,
  .rmdir  = starfs_rmdir,
  .rename = starfs_rename,
  .link = starfs_link,
  .chmod  = starfs_chmod,
  .chown  = starfs_chown,
  .truncate = starfs_truncate,
  .ftruncate  = starfs_ftruncate,
  .utimens  = starfs_utimens,
  .create = starfs_create,
  .open = starfs_open,
  .read = starfs_read,
  .write  = starfs_write,
  .statfs = starfs_statfs,
  .flush  = starfs_flush,
  .release  = starfs_release,
  .fsync  = starfs_fsync,

  .flag_nullpath_ok = 1,
};

int main(int argc, char *argv[])
{
  fd = open(argv[1], O_RDWR);
  lseek64(fd, 0x8000, SEEK_SET);
  read(fd, &fshdr, sizeof(fshdr));

  if (fshdr.signature != STARFS_SIGNATURE) {
    printf("Invalid signature! Aborting.\n");
    exit(1);
  }

  lseek64(fd, 0x9000 + sizeof(struct inode) * 2, SEEK_SET);
  struct inode itab;
  read(fd, &itab, sizeof(itab));

  lseek64(fd, 0x9000, SEEK_SET);
  inodetab = (struct inode *)mmap(NULL, itab.filesize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0x9000);

  for (size_t i = 0; i < itab.filesize / sizeof(struct inode); i++) {
    if (inodetab[i].linkcount) filecount++;
  }

  argv++;
  argc--;
  argv[0] = argv[-1];
  umask(0);
  int rv = fuse_main(argc, argv, &starfs_oper, NULL);
  munmap(inodetab, itab.filesize);
  return rv;
}


