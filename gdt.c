#include "deltaos.h"
#include "screen.h"

struct gdt_entry{
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t  base_middle;
    uint8_t  access;
    uint8_t  flag_limit_high;
    uint8_t  base_high;
} __attribute__((packed));

struct gdt_ptr{
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

struct gdt_entry gdt[6];
struct gdt_ptr gdtptr;

extern void load_gdt(struct gdt_ptr *);

void set_gdt(uint8_t index, uint32_t base, uint32_t limit, uint8_t access, uint8_t flags){
    gdt[index].base_low = base & 0xffff;
    gdt[index].base_middle = (base >> 16) & 0xff;
    gdt[index].base_high = (base >> 24) & 0xff;
    gdt[index].access = access;
    gdt[index].limit_low = limit & 0xffff;
    gdt[index].flag_limit_high = (limit >> 16) & 0x0F;
    gdt[index].flag_limit_high |= flags & 0xF0;
}

void gdt_init(void){
    printk("initializing gdt\n");

    // all zero
    set_gdt(0, 0, 0, 0, 0);
    // kernel code
    set_gdt(1, 0x0, 0xfffff, 0x9a, 0xcf);
    // kernel data
    set_gdt(2, 0x0, 0xfffff, 0x92, 0xcf);
    // user code
    set_gdt(3, 0x0, 0xfffff, 0xfa, 0xcf);
    // user data
    set_gdt(4, 0x0, 0xfffff, 0xf2, 0xcf);

    gdtptr.base = (uint32_t)gdt;
    gdtptr.limit = sizeof(gdt) - 1;

    printk("loading gdt\n");

    load_gdt(&gdtptr);
}
