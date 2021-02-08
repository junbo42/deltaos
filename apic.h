#ifndef _APIC_H
#define _APIC_H

void idt_init(void);
void disable_pic();
void lapic_init();
void enable_irq(int irq, void (* handler)());

#endif
