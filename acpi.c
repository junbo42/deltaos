#include "deltaos.h"
#include "io.h"
#include "screen.h"

struct rsdt_hdr{
  char signature[4];
  uint32_t length;
  uint8_t revision;
  uint8_t checksum;
  char oemid[6];
  char oemtableid[8];
  uint32_t oemrevision;
  uint32_t creatorid;
  uint32_t creatorrevision;
} __attribute__((packed));

void acpi_init(){
    uint8_t *ptr = (uint8_t *)P2V(0x000E0000);
    uint32_t *rsdt;
    for(; *ptr < P2V(0x000FFFFF); ){
        if(*ptr++ == 'R' && *ptr++ == 'S' && *ptr++ == 'D' && *ptr++ == ' ' &&
           *ptr++ == 'P' && *ptr++ == 'T' && *ptr++ == 'R' && *ptr++ == ' '){
            ptr-= 8;
            break;
        }
    }
    printk("acpi rsdp %x\n", ptr);
    ptr += 9;
    printk("acpi rsdp oemid %s\n", ptr);
    ptr += 6;
    printk("acpi rsdp version %x\n", *ptr);
    ptr += 1;
    //rsdt = ((uint32_t *)*(uint32_t *)ptr);
    rsdt = (uint32_t *)P2V(*(uint32_t *)ptr);
    printk("acpi rsdp address %x\n", rsdt);

    printk("acpi dump rsdt\n");
    struct rsdt_hdr *rsdth = (struct rsdt_hdr *)rsdt;
    struct rsdt_hdr *rsdtoh;
    printk("acpi %s\n", rsdth->signature);
    //printk("%s\n", rsdth->oemid);

    int i;
    int size = (rsdth->length - sizeof(struct rsdt_hdr)) / 4;
    uint32_t *rsdto = (uint32_t *)++rsdth;
    for(i = 0; i < size; i++){
        rsdtoh = (struct rsdt_hdr *)P2V(*rsdto);
        printk("acpi %s\n", rsdtoh->signature);
        //printk("%s\n", rsdtoh->oemid);
        rsdto++;
    }
}
