//#include "io.h"
//#include "keyboard.h"
#include "serial.h"
//#include "syscall.h"
#include "deltaos.h"
#include "screen.h"

extern void cputc(char c);
extern uint32_t ticks;

static char *digits = "0123456789abcdef";

static void print_str(char *str){
    while(*str){
        cputc(*str);
        sputc(*str);
        str++;
    }
}

void print_number(uint32_t n, int base){
    char str[32] = "";
    int i = 2;

    do{
        str[i++] = digits[n % base];
    }while((n /= base) != 0);

    while(i){
        cputc(str[i--]);
        sputc(str[i]);
    }
}

void printk(const char *fmt, ...){
    char c;
    int i;
    int flag;
    uint32_t *p = (uint32_t *)&fmt + 1;

    print_str("[ ");
    print_number(ticks, 10);
    print_str("] ");

    flag = 0;
    for(i = 0; c = fmt[i], c; i++){
        if(c == '%'){
            flag = 1;
            continue;
        }
        if(flag){
            switch(c){
            case 's':
                print_str((char*)*p);
                p++;
                break;
            case 'x':
                print_number(*p, 16);
                p++;
                break;
            case 'd':
                print_number(*p, 10);
                p++;
                break;
            case 'b':
                print_number(*p, 2);
                p++;
                break;
            case 'c':
                cputc(*p);
                sputc(*p);
                p++;
                break;
            default:
                cputc(c);
                sputc(c);
                p++;
            }
            flag = 0;
        }else{
            cputc(c);
            sputc(c);
        }
    }
}
