#include "io.h"
#include "apic.h"
#include "screen.h"
#include "string.h"
#include "proc.h"

#define SHIFT    1 << 0
#define ALT      1 << 1

static uint8_t codeset_char[128] = {
     0,    0,   '1',  '2',  '3',  '4',  '5',  '6',
    '7',  '8',  '9',  '0',  '-',  '=',  '\b', '\t',
    'q',  'w',  'e',  'r',  't',  'y',  'u',  'i',
    'o',  'p',  '[',  ']',  '\n',  0,   'a',  's',
    'd',  'f',  'g',  'h',  'j',  'k',  'l',  ';',
    '\'', '`',   0,   '\\', 'z',  'x',  'c',  'v',
    'b',  'n',  'm',  ',',  '.',  '/',   0,    0,
     0,   ' ',   0, 
};  

static uint8_t codeset_shift[128] = {
   0,    0,   '!',  '@',  '#',  '$',  '%',  '^',
  '&',  '*',  '(',  ')',  '_',  '+',  '\b', '\t',
  'Q',  'W',  'E',  'R',  'T',  'Y',  'U',  'I',
  'O',  'P',  '{',  '}',  '\n',  0,   'A',  'S',
  'D',  'F',  'G',  'H',  'J',  'K',  'L',  ':',
  '"',  '~',   0,   '|',  'Z',  'X',  'C',  'V',
  'B',  'N',  'M',  '<',  '>',  '?',   0,   '*',
};  
    
static uint8_t codeset_ctrl[128] = {
    [0x2a] = SHIFT,
    [0x36] = SHIFT,
    [0x38] = ALT,
};

static int shift;
static int capslock;

#define KBUF_LEN 8
static char kbuf[KBUF_LEN];
static int kbuf_p, kbuf_lr;

void keyboard_handler(){
    int key;
    char c;

    if ((inb(0x64) & 0x01) != 0){
        key = inb(0x60);
    }

    if(key & 0x80){
        shift = shift ^ codeset_ctrl[key - 0x80];
        goto finish;                                                                                                                                                                                                                                                             
    }else if(key == 0x3a){
        capslock = capslock ? 0 : 1;
        goto finish;
    }else if(key < 0){
        goto finish;
    }

    shift |= codeset_ctrl[key];
         
    if(capslock && shift & SHIFT)
        c = codeset_char[key];
    else if(capslock || shift & SHIFT)
        c = codeset_shift[key];
    else 
        c = codeset_char[key];

extern int console_write(char *s, size_t count);
    //console(c);
    //console_write(&c, 1);
    kbuf[kbuf_p++ % KBUF_LEN] = c;
    procs->state = RUNNING;

finish:
    return;
}

extern struct proc *current;
extern void sched();
int keyboard_read(char *s, int len){
    char c;
    while(1){
        if(kbuf_lr != kbuf_p){
            c = kbuf[kbuf_lr++ % KBUF_LEN];
            *s = c;
            return 1;
        }else{
            current->state = SLEEP;
            sched();
        }
    }

    return 1;
}

void keyboard_init(){
    printk("initializing keyboard\n");

    enable_irq(0x01, keyboard_handler);
}
