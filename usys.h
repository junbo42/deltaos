#include <stddef.h>
#include <stdint.h>

int write(int, char*, size_t);
int read(int, char*, size_t);
int fork();
int wait();
int execve(char*, char**, char**);
int _exit();
int open(const char *name, int flags);
int getpid();
int sbrk(size_t incr);
int close(int fd);
