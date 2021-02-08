#ifndef _STRING_H
#define _STRING_H
#include "deltaos.h"

void *memset(void *dst, char c, int n);

void *memcpy(void *dst, const void *src, uint32_t cnt);

char *strtok_r(char *s, const char *sep, char **last);

int strcmp(const char *s1, const char *s2);

uint32_t strlen(const char *s);

char *strcpy(char *dest, const char *src);

char *strstrip(char *s);

char *strcat(char *dest, const char *src);

#endif
