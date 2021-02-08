#include "serial.h"
#include "apic.h"
#include "io.h"
#include "screen.h"

#define PORT 0x3f8

static int serial_port;

void serial_handler(){
    char c;

    if(!serial_port)
        return;

extern int console_write(char *s, size_t count);
    if(inb(PORT + 5) & 1){
        c = inb(PORT);
        console_write(&c, 1);
    }
}

void sputc(char c){

    while((inb(PORT + 5) & 0x20) == 0);

    outb(PORT, c);
}

void serial_init(){
    printk("initializing serial port\n");

    outb(PORT + 1, 0x01);    // Disable all interrupts
    outb(PORT + 3, 0x80);    // Enable DLAB (set baud rate divisor)
    outb(PORT + 0, 0x03);    // Set divisor to 3 (lo byte) 38400 baud
    outb(PORT + 1, 0x00);    //                  (hi byte)
    outb(PORT + 3, 0x03);    // 8 bits, no parity, one stop bit
    outb(PORT + 2, 0xC7);    // Enable FIFO, clear them, with 14-byte threshold

    /* is there a serial port ? */
    if(inb(PORT + 5) == 0xff)
        return;

    serial_port = 1;

    //enable_irq(0x04, serial_handler);
}
