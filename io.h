#include "deltaos.h"

static inline uint8_t inb(uint16_t port){
    uint8_t ret;
    asm volatile ("inb %1, %0"
                  : "=a"(ret)
                  : "Nd"(port));
    return ret;
}

static inline uint16_t inw(uint16_t port){
	uint16_t ret;
	asm volatile ("inw %1, %0"
                  : "=a"(ret)
                  : "Nd"(port));
	return ret;
}

static inline uint32_t inl(uint16_t port){
	uint32_t ret;
	asm volatile ("inl %1, %0"
                  : "=a"(ret)
                  : "Nd"(port));
	return ret;
}

static inline void outb(uint16_t port, uint8_t val){
    asm volatile ("out %0, %1"
                  : : "a"(val), "Nd"(port));
}

static inline void outw(uint16_t port, uint16_t val){
	asm volatile ("outw %0, %1"
                  : : "a" (val), "Nd" (port));

}
static inline void outl(uint16_t port, uint32_t val){
	asm volatile ("outl %0, %1"
                  : : "a" (val), "Nd" (port));

}

static inline void hlt(){
    asm volatile ("hlt");
}

static inline void insl(int port, void *addr, int cnt){
    asm volatile ("cld; rep insl"
                  : "=D"(addr), "=c"(cnt)
                  : "d"(port), "0"(addr), "1"(cnt)
                  : "memory", "cc");
}

static inline uint32_t pmem_read32(uint32_t address){
    return *(uint32_t *)(address);
}

static inline void pmem_write32(uint32_t address, uint32_t value){
    *(uint32_t *)(address) = value;
}

static inline void sti(){
    asm volatile ("sti");
}

static inline void cli(){
    asm volatile ("cli");
}

static inline void panic(){
    while(1)
    asm("cli; hlt");
}

