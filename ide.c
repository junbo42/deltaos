#include "deltaos.h"
#include "apic.h"
#include "screen.h"
#include "io.h"
#include "ext2.h"

#define IDE_ERR 0x01
#define IDE_DF  0x20                                                                                  
#define IDE_RDY 0x40
#define IDE_BSY 0x80

static struct bbuf *queue;

static void ide_wait(){
    int r;

    while(((r = inb(0x1f7)) & (IDE_BSY|IDE_RDY)) != IDE_RDY);

    //if((r & (IDE_DF | IDE_ERR)) != 0)
    //    return -1;
    //return 0;
}

void ide_intr(){
    if(queue->valid)
        return;
    insl(0x1f0, queue->data, 256);
    queue->valid = 1;
}

void sleep(uint32_t time);

//void ide_read(uint32_t blknum, struct buf *buf){
void ide_read(uint32_t blknum, struct bbuf *bbuf){
    int sector = (blknum + 1024 ) * 2;

    ide_wait();

    queue = bbuf;
    outb(0x3f6, 0);
    outb(0x1f2, 2);
    outb(0x1f3, sector);
    outb(0x1f4, (sector >> 8) & 0xff);
    outb(0x1f5, (sector >> 16) & 0xff);
    outb(0x1f6, 0xe0 | ((0&1)<<4) | ((sector>>24)&0x0f));
    outb(0x1f7, 0xc4);

    while(!queue->valid);
}

void ide_init(){
    printk("initializing ide\n");

    outb(0x1f6, 0xe0 | (0<<4));

    enable_irq(14, ide_intr);

    ide_wait();
}
