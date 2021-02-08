#include "deltaos.h"
#include "screen.h"
#include "io.h"
#include "keyboard.h"
#include "proc.h"
#include "serial.h"
#include "syscall.h"

#define IRQ0 32

// should get it from ACPI
#define LAPIC 0xfee00000
#define IOAPIC 0xfec00000

#define APIC_SIVR 0x00f0 // Spurious Interrupt Vector Register

extern void load_idt(uint32_t *idt_ptr);
extern void load_cr3();

extern void irq_0();
extern void irq_1();
extern void irq_2();
extern void irq_3();
extern void irq_4();
extern void irq_5();
extern void irq_6();
extern void irq_7();
extern void irq_8();
extern void irq_9();
extern void irq_10();
extern void irq_11();
extern void irq_12();
extern void irq_13();
extern void irq_14();
extern void irq_15();

extern void irq_division();
extern void irq_protectionfault();
extern void irq_pagefault();
extern void irq_syscall();

void sched();

typedef struct{
   uint32_t ds;                  // Data segment selector
   uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax; // Pushed by pusha.
   uint32_t irq_no, err_code;               // Interrupt number and error code (if applicable)
   uint32_t eip, cs, eflags, useresp, ss;   // Pushed by the processor automatically.
} interrupt_frame; 

void do_division(){
    printk("division\n");
}

void do_singlestep(){
    printk("singlestep\n");
}

void do_nmi(){
    printk("nmi\n");
}

void do_breakpoint(){
    printk("breakpoint\n");
}

void do_overflow(){
    printk("overflow\n");
}

void do_bound(){
    printk("bound\n");
}

void do_invalidop(){
    printk("invalidop\n");
}

void do_cocpu(){
    printk("cocpu\n");
}

void do_doublefault(){
    printk("doublefault\n");
}

void do_cocpuseg(){
    printk("cocpuseg\n");
}

void do_invalidtss(){
    printk("invalidtss\n");
}

void do_segnp(){
    printk("segnp\n");
}

void do_stackfault(){
    printk("stackfault\n");
}

void do_protectionfault(){
    printk("protectionfault\n");
    panic();
}

void do_pagefault(interrupt_frame frame){
    cli();

    uint32_t addr = 0;

    asm volatile ("mov %%cr2, %0;"
                  : "=r"(addr));

//#ifdef IRQ_DBG
    if(addr != 0x20000000)
        printk("pagefault %x\n", addr);
//#endif

//TODO, quit the fault process.
    if(addr == 0x20000000){
        current->state = ZOMBIE;
        current->parent->state = RUNNING;
        //printk("p2 %s %d\n", current->parent->name, current->parent->state);
        sched();
    }else{
        panic();
    }

    //sti();
}

void do_reserved(){
    printk("do_reserved\n");
}

void do_align(){
    printk("align\n");
}

//void do_syscall(interrupt_frame frame){
void do_syscall(struct iframe frame){
    int ret;
#ifdef SYS_DBG
    printk("syscall %x\n", frame.eax);
#endif

    if(frame.eax > 15){
        printk("wrong syscall number %x\n", frame.eax);
        frame.eax = -1;
        return;
    }

    current->frame = &frame;
    ret = syscalls[frame.eax]((void *)frame.useresp);
    cli();
    frame.eax = ret;
}

static struct irq_handler{
    int irq;
    void (* handler)();
} irq_handlers[256];

struct idt_entry{
    uint16_t offset_low;
    uint16_t selector;
    uint8_t zero;
    uint8_t type;
    uint16_t offset_high;
} idt[256];

static void set_trap_gate(uint8_t i, uint32_t addr){
	idt[i].offset_low= addr & 0xffff;
	idt[i].selector = 0x08;
	idt[i].zero = 0;
	idt[i].type = 0x8f;
	idt[i].offset_high= (addr & 0xffff0000) >> 16;
}

static void set_intr_gate(uint8_t i, uint32_t addr){
	idt[0x20 + i].offset_low= addr & 0xffff;
	idt[0x20 + i].selector = 0x08;
	idt[0x20 + i].zero = 0;
	idt[0x20 + i].type = 0x8e;
	idt[0x20 + i].offset_high= (addr & 0xffff0000) >> 16;
}

void idt_init(void){
	uint32_t idt_address;
	uint32_t idt_ptr[2];

    printk("initializing idt\n");

    set_trap_gate(0x00, (uint32_t)irq_division);
    set_trap_gate(0x01, (uint32_t)do_singlestep);
    set_trap_gate(0x02, (uint32_t)do_nmi);
    set_trap_gate(0x03, (uint32_t)do_breakpoint);
    set_trap_gate(0x04, (uint32_t)do_overflow);
    set_trap_gate(0x05, (uint32_t)do_bound);
    set_trap_gate(0x06, (uint32_t)do_invalidop);
    set_trap_gate(0x07, (uint32_t)do_cocpu);
    set_trap_gate(0x08, (uint32_t)do_doublefault);
    set_trap_gate(0x09, (uint32_t)do_cocpuseg);
    set_trap_gate(0x0a, (uint32_t)do_invalidtss);
    set_trap_gate(0x0b, (uint32_t)do_segnp);
    set_trap_gate(0x0c, (uint32_t)do_stackfault);
    set_trap_gate(0x0d, (uint32_t)do_protectionfault);
    set_trap_gate(0x0e, (uint32_t)irq_pagefault);
    set_trap_gate(0x0f, (uint32_t)do_reserved);
    set_trap_gate(0x11, (uint32_t)do_align);
    set_trap_gate(0x80, (uint32_t)irq_syscall);
    idt[0x80].type = 0xef;

    set_intr_gate(0, (uint32_t)irq_0);
    set_intr_gate(1, (uint32_t)irq_1);
    set_intr_gate(2, (uint32_t)irq_2);
    set_intr_gate(3, (uint32_t)irq_3);
    set_intr_gate(4, (uint32_t)irq_4);
    set_intr_gate(5, (uint32_t)irq_5);
    set_intr_gate(6, (uint32_t)irq_6);
    set_intr_gate(7, (uint32_t)irq_7);
    set_intr_gate(8, (uint32_t)irq_8);
    set_intr_gate(9, (uint32_t)irq_9);
    set_intr_gate(10, (uint32_t)irq_10);
    set_intr_gate(11, (uint32_t)irq_11);
    set_intr_gate(12, (uint32_t)irq_12);
    set_intr_gate(13, (uint32_t)irq_13);
    set_intr_gate(14, (uint32_t)irq_14);
    set_intr_gate(15, (uint32_t)irq_15);

	idt_address = (uint32_t)idt;
	idt_ptr[0] = (sizeof (struct idt_entry) * 256) + ((idt_address & 0xffff) << 16);
	idt_ptr[1] = idt_address >> 16;

    printk("loading idt\n");

	load_idt(idt_ptr);
}

void disable_pic(){
	/* ICW1 */
	outb(0x20 , 0x11);
	outb(0xa0 , 0x11);

	/* ICW2 */
	outb(0x21 , 0xe0);
	outb(0xA1 , 0xe8);

	/* ICW3 */
	outb(0x21 , 0x04);
	outb(0xa1 , 0x02);

	/* ICW4 */
	outb(0x21 , 0x01);
	outb(0xa1 , 0x01);

	/* mask interrupts */
	outb(0x21 , 0xff);
	outb(0xA1 , 0xff);

    printk("pic disabled\n");
}

extern struct proc *current;

//void irq_s2(interrupt_frame frame){
void irq_s2(struct iframe frame){
#ifdef IRQ_DBG
    printk("irq_s2 %x\n", frame.irq_no);
#endif
    if((frame.irq_no) == 0x80)
        current->frame = &frame;
    irq_handlers[frame.irq_no].handler();
    pmem_write32(LAPIC + 0xb0, 0);
    if((frame.irq_no) == 0x02)
        sched();
}

volatile uint32_t *ioapic = (volatile uint32_t*)IOAPIC;

static inline uint32_t ioapic_read(int reg){
    ioapic[0] = reg & 0xff;
    return ioapic[4];
}

static inline void ioapic_write(int reg, uint32_t value){
    ioapic[0] = reg;
    ioapic[4] = value;
}

void enable_irq(int irq, void (* handler)()){
    ioapic_write(0x10 + 2 * irq, IRQ0 + irq);
    ioapic_write(0x10 + 2 * irq + 1, 0 << 24);

    irq_handlers[irq].irq = irq;
    irq_handlers[irq].handler = handler;

    printk("irq %d enabled\n", irq);
}

void lapic_init(){
    uint32_t sivr;
    uint32_t i, max;

    printk("initializing apic\n");
    // enable lapic
    sivr = pmem_read32(LAPIC + APIC_SIVR);
    sivr |= 1 << 8;
    pmem_write32(LAPIC + APIC_SIVR, sivr);
    //pmem_write32(APIC_SIVR, 0x13f);

    // mask all interrupts
    printk("masking ioapic\n");
    max = (ioapic_read(0x01) >> 16) & 0xff;
    for(i = 0; i <= max; i++){
        ioapic_write(0x10 + 2 * i, 0x00010000 | (IRQ0 + i));
        ioapic_write(0x10 + 2 * i + 1, 0);
    }
}
