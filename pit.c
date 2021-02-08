#include "deltaos.h"
#include "screen.h"
#include "io.h"
#include "apic.h"

#define PIT_CMD 0x43
#define PIT_CH0 0x40

//TODO
uint32_t ticks;

extern void sched();
void timer(){
    ticks++;
//        if(!(ticks % 1000))
//            printk("%d\n", ticks / 1000);
}

void pit_init(){
    printk("initializing pit, going to 1000hz\n");
    // 1000hz
    //uint16_t divider = 1193;

    uint16_t divider = 21930;

    outb(PIT_CMD, 0x68);
    //outb(PIT_CMD, 0x36);
    outb(PIT_CH0, divider & 0xff);
    outb(PIT_CH0, (divider >> 8) & 0xff);

    enable_irq(0x02, timer);
}

void sleep(uint32_t time){
    uint32_t now = ticks / 1000;
    while(1){
        if(ticks / 1000 - now >= time)
            break;
    }
}
