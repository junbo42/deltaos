#include "deltaos.h"

void *memset(void *dst, char c, int n){
    asm volatile("cld; rep stosb;"
                 : : "D"(dst), "a"(c), "c"(n));
    return dst;
}

void *memcpy(void *dst, const void *src, uint32_t cnt){
    char *d;
    const char *s;

    d = dst;
    s = src;
    
    while(cnt-- > 0)
      *d++ = *s++;
    return dst;
}

char *strtok_r(char *s, const char *sep, char **last){
    char *c;

    if(s == NULL)
        s = *last;

    if(s == NULL || *s == 0)
        return NULL;

    c = s;

    if(*s == *sep){
        c = ++s;
    }

    while(1){
        if(*c++ == *sep){
            *last = c;
            c[-1] = 0;
            break;
        }
        if(*c == 0){
            *last = NULL;
            break;
        }
    }

    return s;
}

int strcmp(const char *s1, const char *s2){
    while(*s1 == *s2){
        s1++;
        s2++;
        if(*s1 == 0 || *s2 == 0)
            return *s1 - *s2;
    }
    return *s1 - *s2;
}

uint32_t strlen(const char *s){
    int i = 0;

    while(*s++ != 0)
        i++;                                                                                                                                                             
    return i;
}

char *strcpy(char *dest, const char *src){
    char *d = dest;
    char *s = (char *)src;

    while(*s)
        *d++ = *s++;
    return dest;
}

char *strstrip(char *s){
    int i;
    char *d = (char *)s;
    unsigned int len = strlen(s) - 1;

    for(i = 0; i < len; i++){
        if(s[i] == ' ' || s[i] == '\n')
            d++;
        else
            break;
    }

    s = d;
    len = strlen(s) - 1;

    for(i = len; i >= 0; i++){
        if(s[i] == ' ' || s[i] == '\n')
            s[i] = 0;
        else
            break;
    }

    return s;
}

char *strcat(char *dest, const char *src){
    char *d = dest;
    d += strlen(dest);

    while(*src)
        *d++ = *src++;
     
    return dest;
}    
