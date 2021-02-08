#include "deltaos.h"
#include "apic.h"
#include "acpi.h"
#include "console.h"
#include "gdt.h"
#include "ide.h"
#include "io.h"
#include "keyboard.h"
#include "memory.h"
#include "pit.h"
#include "serial.h"
#include "string.h"

/* https://www.gnu.org/software/grub/manual/multiboot/multiboot.html */
uint8_t *mb;

extern void shell_main();

void print_msg(){
	char *str = "Delta Network Operating System\n\n";
    printk(str);
}

extern struct node *kmem;
uint32_t kpgdir;

void ext2_init();

extern uint32_t kmsize;
extern void ext2_elf();
extern void proc_init();
extern void scheduler();

//char *p[30000];

void kmain(uint32_t addr){
    mb = (uint8_t *)addr;

    console_init();
    vm_init();
    idt_init();
    disable_pic();
    lapic_init();
    keyboard_init();
    gdt_init();
    serial_init();
    //acpi_init();
    pit_init();
    sti();
    ide_init();
    ext2_init();

    proc_init();
    scheduler();
}
