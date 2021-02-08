#include "io.h"
#include "keyboard.h"
#include "serial.h"
#include "syscall.h"

//#define dev_video 0xb8000
#define dev_video 0xc00b8000

static char *screen = (char*)dev_video;
static int cursor;
extern uint32_t ticks;

//static int get_cursor(){
//    int pos;
//
//    outb(0x3D4, 0x0F);
//    pos |= inb(0x3D5);
//    outb(0x3D4, 0x0E);
//    pos |= ((uint16_t)inb(0x3D5)) << 8;
//
//    return pos;
//}

void screen_move(){
    int i, j;

    for(i = 0; i < 24; i++){
        for(j = 0; j < 160 ; j++){
            screen[i * 160 + j] = screen[i * 160 + 160 + j];
        }
    }

    for(i = 0; i < 80; i++){
        screen[3840 + 2 * i] = ' ';
        screen[3840 + 2 * i + 1] = 0x07;
    }
}

static void move_cursor(){
    outb(0x3D4, 0x0F);
    outb(0x3D5, (uint8_t) (cursor & 0xFF));
    outb(0x3D4, 0x0E);
    outb(0x3D5, (uint8_t) ((cursor >> 8) & 0xFF));

    if(cursor >= 2000){
        cursor -= 80;
        screen_move();
        move_cursor();
    }
}

void cputc(char c){
    int pos = cursor;

    switch(c){
    case 0:
        break;
    case '\b':
        if(!(cursor % 80))
            break;
        cursor--;
        move_cursor();
        screen[cursor * 2] = ' ';
        screen[cursor * 2 + 1] = 0x07;
        break;
    case '\n':
        cursor += (80 - cursor % 80);
        move_cursor();
        break;
    default:
        screen[pos * 2] = c & 0xff;
        screen[pos * 2 + 1] = 0x07;
        cursor++;
        move_cursor();
    }
}

void screen_init(void){
    int i;

    /* 26 lines, 80 columns.*/
	while(i < 80 * 25 * 2) {
		screen[i] = ' ';
		screen[i+1] = 0x07;
		i = i + 2;
	}

    uint16_t pos = 0;

    outb(0x3D5, (uint8_t) (pos & 0xFF));
    outb(0x3D4, 0x0E);
    outb(0x3D5, (uint8_t) ((pos >> 8) & 0xFF));
    outb(0x21 , 0xFD);
}
