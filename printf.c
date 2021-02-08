#include "deltaos.h"
#include "usys.h"
#include <stdarg.h>
#include "string.h"

#define INTMAX 4294967296
#define PINTMAX 2147483648

const char *_digits = "0123456789abcdef";
const char _space = ' ';

int isnumber(char c){
    return 1 ? c > 47 && c < 58 : 0;
}

int printf(const char *fmt, ...){
    va_list ap;
    char buf[256] = "";
    int bufo = 0;

    va_start(ap, fmt);

    int i, j, flag, w;
    char c;
    char *ac;
    int ai;
    char ibuf[64];

    flag = 0;
    w = 0;

    for(i = 0; c = fmt[i], c != 0; i++){
        if(c == '%'){
            flag = 1;
            continue;
        }
        if(flag){
            if(isnumber(c)){
                w = c - 48;
                while(w--){
                    //write(1, _space, 1);
                    buf[bufo++] = _space;
                }
                continue;
            };
            switch(c){
            case 'c':
                buf[bufo++] = (char)va_arg(ap, int);
                break;
            case 's':
                ac = va_arg(ap, char *);
                //write(1, ac, strlen(ac));
                strcpy(buf+bufo, ac);
                bufo += strlen(ac);
                break;
            case 'd':
                j = 0;
                ai = va_arg(ap, unsigned int);
                memset(ibuf, 0, 64);
                if(ai > PINTMAX){
                    buf[bufo++] = 45;
                    ai = INTMAX - ai;
                }
                ibuf[j++] = _digits[ai % 10];
                while(ai /= 10){
                    ibuf[j++] = _digits[ai % 10];
                }
                while(j--){
                    buf[bufo++] = ibuf[j];
                }
                break;
            case 'u':
                j = 0;
                ai = va_arg(ap, unsigned int);
                memset(ibuf, 0, 64);
                ibuf[j++] = _digits[ai % 10];
                while(ai /= 10){
                    ibuf[j++] = _digits[ai % 10];
                }
                while(j--){
                    buf[bufo++] = ibuf[j];
                }
                break;
            case 'x':
                j = 0;
                ai = va_arg(ap, unsigned int);
                memset(ibuf, 0, 64);
                ibuf[j++] = _digits[ai % 16];
                while(ai /= 16){
                    ibuf[j++] = _digits[ai % 16];
                }
                while(j--){
                    buf[bufo++] = ibuf[j];
                }
                break;
            default:
                buf[bufo++] = c;
            }
            flag = 0;
            w = 0;
        }else{
            buf[bufo++] = c;
        }
    }

    va_end(ap);

    return write(1, buf, strlen(buf));
}
