#include "deltaos.h"

#define PGSIZE 4096
#define PGALIGN(v) (((v)) & ~(PGSIZE -1))
#define PDX(va) (((uint32_t)(va) >> 22) & 0x3ff)
#define PTX(va) (((uint32_t)(va) >> 12) & 0x3ff)
#define PTE_ADDR(pte) ((uint32_t)(pte) & ~0xFFF)

#define PTE_P 0x001
#define PTE_W 0x002
#define PTE_U 0x004

void vm_init();
uint32_t *pgdir_init();
void pgdir_free(uint32_t *pgdir);
void mappages(uint32_t *pgdir, char *va, char *pa, uint32_t len, uint16_t perm);
char *page_alloc();
void page_free(void *p);
uint32_t *walkpgdir(uint32_t *pgdir, void *va);

void *kmalloc(uint32_t size);
void kfree(void *p);
