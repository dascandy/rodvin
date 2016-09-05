#ifndef __STRING_H
#define __STRING_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

size_t strlen(const char *str);
int strcmp(const char *a, const char *b);
char *strcpy(char *dst, const char *src);
char *strstr(const char *haystack, const char *needle);
void memcpy(void *dst, const void *src, size_t count);
void memmove(void *dst, const void *src, size_t count);
void memset(void *target, uint8_t c, size_t n);
int memcmp(const void *s1, const void *s2, size_t n);
char *strdup(const char *s);
char *strrchr(char *s, char c);
char *strchr(char *s, int c);

#ifdef __cplusplus
}
#endif

#endif


