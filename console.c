#include "deltaos.h"

extern void cputc(char c);
extern void sputc(char c);
extern int keyboard_read(char *s, int len);
extern void screen_init(void);

int console_read(char *s, size_t count){
    int ret;

    ret = keyboard_read(s, count);

    return ret;
}

int console_write(char *s, size_t count){
    int i;

    for(i = 0; i < count; i++){
        cputc(s[i]);
        sputc(s[i]);
    }

    return i + 1;
}

void console_init(void){   
    screen_init();
}
