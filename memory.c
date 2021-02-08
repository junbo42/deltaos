#include "deltaos.h"
#include "screen.h"
#include "io.h"
#include "memory.h"
#include "string.h"
#define PGINIT1 1024 * 1024 * 8
#define PGINIT2 1024 * 1024 * 127

extern void load_cr3();
extern uint32_t mb;
extern char kend[];

uint32_t *kpgdir;

struct e820{
    uint32_t size;
    uint32_t addrl;
    uint32_t addrh;
    uint32_t lenl;
    uint32_t lenh;
    uint32_t type;
};

static void memory_detect(){
    uint32_t start, end;
    //uint32_t mem_lower = *(uint32_t*)(mb + 4);
    uint32_t mem_upper = *(uint32_t*)(mb + 8);
    struct e820 *mmap = (struct e820*)(P2V(*(uint32_t *)(mb + 48)));
 
    //printk("total memory mem_lower %dkb mem_upper %dkb\n", mem_lower, mem_upper);
    printk("total memory %dkb\n", mem_upper);
 
    while(mmap->lenl){
        start = mmap->addrl;
        end = mmap->addrl + mmap->lenl;

        printk("memory %x-%x %x\n", start, end - 1, mmap->type);

        //if(mmap->type == 1){
        //    printk("free\n");
        //}else if(mmap->type == 2){
        //    printk("reserved\n");
        //}else{
        //    printk("unknown\n");
        //}
        mmap++;
    }
}

struct node {
    struct node *next;
};

struct node *freepages;
uint32_t pageused;
uint32_t *kpgdir;

void page_init(){
    //TODO, do check.
    char *p;
    char *end = (char *)PGALIGN((uint32_t)(P2V(kend)));
    struct node *n;
    static int start = 0;

    if(!start){
        printk("initializing physical page, kend %x\n", end);

        for(p = (char *)P2V(PGINIT1); p > end; p-= PGSIZE){
            //printk("page_init %x\n", p);
            memset(p, 0, 4096);
            n = (struct node *)p;
            n->next = freepages;
            freepages = n;
        }
        start = 1;
    }else{
        for(p = (char *)P2V(PGINIT2); p > (char *)P2V(PGINIT1)+4096; p-= PGSIZE){
            //printk("page_init %x\n", p);
            memset(p, 0, 4096);
            n = (struct node *)p;
            n->next = freepages;
            freepages = n;
        }

    }
}

char *page_alloc(){
    struct node *n;

    n = freepages;
    if(n){
        freepages = n->next;
        pageused++;
    }else{
        // TODO, give detail info, this initial release has memory leak issue!
        printk("OUT OF MEMORY %d\n", pageused);
        panic();
    }

#ifdef MEM_DBG
    printk("page_alloc %x\n", n);
#endif
    //memset(n, 0, 4096);
    return (char *)n;
}

void page_free(void *p){
    struct node *n;

    n = (struct node *)p;
#ifdef MEM_DBG
    printk("page_free %x\n", p);
#endif
    n->next = freepages;
    freepages = n;
    pageused--;
}


uint32_t *walkpgdir(uint32_t *pgdir, void *va){
    uint32_t *pde, *pgtab;
    //char *p;

    pde = &pgdir[PDX(va)];
    if(*pde & PTE_P){
        pgtab = (uint32_t *)P2V((PTE_ADDR(*pde)));
    }else{
        pgtab = (uint32_t *)(page_alloc());
        //printk("walkpgdir %x\n", pgtab);
        memset(pgtab, 0, PGSIZE);
        *pde = (uint32_t)V2P(pgtab) | PTE_P | PTE_W | PTE_U;
    }

    return &pgtab[PTX(va)];
}

void mappages(uint32_t *pgdir, char *va, char *pa, uint32_t len, uint16_t perm){
    char *first, *last;
    uint32_t *pte;

#ifdef MEM_DBG
    printk("mappages va %x pa %x len %x\n", va, pa, len);
#endif
    first = (char *)PGALIGN((uint32_t)va);
    last = (char *)PGALIGN((uint32_t)va + len - 1);
    for(;;){
        pte = walkpgdir(pgdir, first);
//#ifdef MEM_DBG
//        printk("mappages va %x pa %x pte %x %x\n", first, pa, pte, *pte);
//#endif
        if(*pte & PTE_P){
            printk("%x remap\n", pte);
            panic();
        }
        *pte = (uint32_t)pa | perm | PTE_P;
        if(first == last)
            break;
        //printk("%x\n", first);
        //printk("%x\n", *pte);
        first += PGSIZE;
        pa += PGSIZE;
    }
}

uint32_t *pgdir_init(){
    uint32_t *dir;
    size_t len;
    char *va, *pa;

    dir = (uint32_t *)page_alloc();
    memset(dir, 0, 4096);

    // fb still use it
    // map va 0xc0000000-0xc0100000 to pa 0x00000000-0x00100000
    va = (char *)0xc0000000;
    pa = (char *)0x00000000;
    len = 0x100000;
    mappages(dir, va, pa, len, PTE_W);

    // map va 0xc0100000-0xe0000000 to pa 0x00100000-0x20000000
    va = (char *)0xc0100000;
    pa = (char *)0x00100000;
    //len = 0x1f00000 - 0x1000;
    len = 0x20000000 - 0x100000;
    mappages(dir, va, pa, len, PTE_W);

    //// map va 0xfec00000-0xffffffff to pa 0xfec00000-0xffffffff
    //va = (char *)0xfec00000;
    //pa = (char *)0xfec00000;
    //len = 0x13fffff;

    // map va 0xfec00000-0xfeefffff to pa 0xfec00000-0xfeefffff
    va = (char *)0xfec00000;
    pa = (char *)0xfec00000;
    len = 0x2fffff;
    mappages(dir, va, pa, len, PTE_W);

    return dir;
}

//TODO try tracking pgdir
void pgdir_free(uint32_t *dir){
    int i, j;
    uint32_t *pde, *pte;

    for(i = 0; i < 1024; i++){
        pde = dir + i;
        if(*pde & PTE_P){
            if(i < 768){
                for(j = 0; j < 1024; j++){
                    pte = (uint32_t*)P2V(PTE_ADDR(*pde)) + j;
                    if(*pte & PTE_P){
                        page_free((void *)P2V(PTE_ADDR(*pte)));
                    }
                }
            }
            page_free((void *)P2V(PTE_ADDR(*pde)));
        }
    }
    page_free(dir);
}

struct kmpage{
    struct kmpage *next;
    struct kmpage *prev;
    struct kmblock *blocks;
    int size;
    int free;
};

struct kmblock{
    struct kmblock *next;
};

struct{
    struct kmpage *kmpage;
    int size;
    int nblocks;
}kmpages[] = {
{NULL, 32, 127},
{NULL, 64, 63},
{NULL, 128, 31},
{NULL, 256, 15},
{NULL, 512, 7},
{NULL, 1019, 4},
{NULL, 2038, 2},
{NULL, 4076, 1},
};

uint32_t kmused;
struct kmpage *kmpage;

void kmpages_init(){
    int i, j;
    struct kmpage *kmpage;
    struct kmblock *block;

    for(i = 0; i < 8; i++){
        kmpage = (struct kmpage *)page_alloc();
        kmpage->prev = NULL;
        kmpage->next = NULL;
        kmpage->blocks = NULL;
        kmpage->free = kmpages[i].nblocks;
        kmpage->size = kmpages[i].size;
        kmused++;

        kmpages[i].kmpage = kmpage;

        block = (struct kmblock *)((char *)kmpage + sizeof(struct kmpage));

        for(j = 0; j < kmpages[i].nblocks; j++){
            block->next = kmpage->blocks;
            kmpage->blocks = block;
            block += kmpages[i].size / 4;
        }

    }
}

int get_idx(int size){
    int i = 0;

    while(kmpages[i].size < size)
        i++;

    return i;
}

// TODO
void *kmalloc(uint32_t size){
    struct kmpage *page;
    struct kmblock *block;
    int i, idx;

    if(size > 4080){
        printk("kmalloc size to large %d\n", size);
        goto err;
    }

    idx = get_idx(size);

retry:
    page = kmpages[idx].kmpage;

#ifdef MEM_DBG
    printk("kmalloc page %x\n", page);
#endif
    for(; page; page = page->next){
        if((block = page->blocks)){
            page->blocks = block->next;
            page->free--;
#ifdef MEM_DBG
            printk("kmalloc block %x\n", block);
#endif
            //if(block == 0xc030ee94)
            //    printk("alloc hit %x\n", block->next);
            return block;
        }
    }

    page = (struct kmpage *)page_alloc();
    //memset(page, 0, 4096);
    page->prev = NULL;
    page->next = kmpages[idx].kmpage;
    page->blocks = NULL;
    page->free = kmpages[idx].nblocks;
    page->size = kmpages[idx].size;
    kmused++;

    kmpages[idx].kmpage->prev = page;
    kmpages[idx].kmpage = page;
    block = (struct kmblock *)((char *)page + sizeof(struct kmpage));

    for(i = 0; i < kmpages[idx].nblocks; i++){
        block->next = page->blocks;
        page->blocks = block;
        block += kmpages[idx].size / 4;;
    }

    goto retry;

err:
    return NULL;
}

#define PAGE_BASE(x)((uint32_t)x&~(4096-1))

void kfree(void *p){
    struct kmpage *page;
    struct kmblock *block;
    int idx;

#ifdef MEM_DBG
    printk("kfree %x\n", p);
#endif

    block = (struct kmblock *)p;
    page = (struct kmpage *)PAGE_BASE(p);

    block->next = page->blocks;
    page->blocks = block;
    page->free++;

    for(idx = 0; idx < 8; idx++){
        if(kmpages[idx].size == page->size)
            break;
    }

    //printk("kfree size %d\n", page->size);
    if(idx == 8 || page->size == 0){
        printk("idx not found\n");
        panic();
    }

    if(page->free == kmpages[idx].nblocks){
        if(kmpages[idx].kmpage == page){
            kmpages[idx].kmpage = page->next;
        }else{
            page->prev->next = page->next;
        }
        page_free(page);
        kmused--;
    }
}

void vm_init(){
    memory_detect();
    page_init();
    printk("initializing virtual page\n");
    kpgdir = pgdir_init();

    printk("loading kernel page\n");
    load_cr3(V2P(kpgdir));

    //struct kmblock *block;

    //kmpage = (struct kmpage *)page_alloc();
    //kmpage->prev = NULL;
    //kmpage->next = NULL;
    //kmpage->blocks = NULL;
    //kmused++;

    //block = (struct kmblock *)((void *)kmpage + sizeof(struct kmpage));

    //int i;
    //for(i = 0; i < 3; i++){
    //    block->next = kmpage->blocks;
    //    kmpage->blocks = block;
    //    block += 256;
    //}

    page_init();
    kmpages_init();
}

//TODO, va contiguous
struct vmpage{
    struct vmpage *next;
    struct vmpage *prev;
    int size;
};

struct vmpage *vmpages;

char *vmalloc(int size){
    struct vmpage *vmpage;
    char *p, *p2;

    vmpage = (struct vmpage *)page_alloc();
    vmpage->size = size;
    p = ((char *)vmpage) + sizeof(struct vmpage);
    p2 = p;

    size -= 4096 - sizeof(struct vmpage);
    p += 4096;

    while(size > 0){
        p = page_alloc();  
        size -= 4096;
        p += 4096;
    }

    vmpage->prev = vmpages;
    vmpage->next = NULL;
    vmpages = vmpage;

    return p2;
}

void vfree(char *p){
    struct vmpage *vmpage;
    char *next;
    int size;

    vmpage = (struct vmpage *)(p - sizeof(struct vmpage));
    size = vmpage->size;
    
    next = p;
    while(size > 0){
        page_free(next);
        next += 4096;
        size -= 4096;
    }
}
