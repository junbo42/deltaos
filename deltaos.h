#ifndef _DELTAOS_H
#define _DELTAOS_H

#include <stddef.h>
#include <stdint.h>

#define KERNEL_OFFSET 0xc0000000
#define P2V(a) ((uint32_t)a + KERNEL_OFFSET)
#define V2P(a) ((uint32_t)a - KERNEL_OFFSET)

void printk(const char *fmt, ...);

//typedef char int8_t;
//typedef unsigned char uint8_t;
//typedef short int16_t;
//typedef unsigned short uint16_t;
//typedef int int32_t;
//typedef unsigned int uint32_t;
//
//#define NULL ((void *)0)

//#define IRQ_DBG
//#define EXT2_DBG
//#define MEM_DBG
//#define SYS_DBG

#endif
