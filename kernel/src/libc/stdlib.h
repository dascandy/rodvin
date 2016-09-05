#ifndef STDLIB_H
#define STDLIB_H

#include <stddef.h>
#include <stdint.h>

void malloc_add_region(uint64_t start, size_t length);

#ifdef __cplusplus
extern "C" {
#endif

void *malloc(size_t size);
void free(void *p);
void *realloc(void* p, size_t size);
void abort();
int atexit(void (*func)());
int atoi(const char *nptr);
char *getenv(const char *name);

int abs(int j);
long int labs(long int j);
long long int llabs(long long int j);

long int strtol(const char *nptr, char **endptr, int base);

#ifdef __cplusplus
}
#endif

#endif


