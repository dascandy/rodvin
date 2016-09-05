#ifndef STDIO_H
#define STDIO_H

#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

void printf(const char *fmt, ...);
void hexdump(const uint8_t *buf, size_t length);

typedef struct {
} FILE;

extern FILE *stderr;

enum {
  SEEK_SET = 0,
  SEEK_CUR = 1,
  SEEK_END = 2,
};
int fflush(FILE *stream);
int fprintf(FILE *stream, const char *format, ...);
int fclose(FILE *fp);
FILE *fopen(const char *path, const char *mode);
size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream);
size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream);
int fseek(FILE *stream, long offset, int whence);
long ftell(FILE *stream);
void setbuf(FILE *stream, char *buf);
int vfprintf(FILE *stream, const char *format, va_list ap);

#ifdef __cplusplus
}
#endif

#endif


