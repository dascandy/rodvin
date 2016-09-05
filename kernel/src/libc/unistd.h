#ifndef UNISTD_H
#define UNISTD_H

#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

extern char **environ;
pid_t fork();
int execv(const char *path, char *const argv[]);
int execve(const char *filename, char *const argv[],
           char *const envp[]);
int execvp(const char *file, char *const argv[]);

#ifdef __cplusplus
}
#endif

#endif


