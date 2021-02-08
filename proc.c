#include "deltaos.h"
#include "apic.h"
#include "screen.h"
#include "elf.h"
#include "gdt.h"
#include "io.h"
#include "string.h"
#include "ext2.h"
#include "memory.h"
#include "proc.h"

extern void load_tss();
extern void load_cr3();
extern void switch_to(struct context **old, struct context *new);
extern void switch_done();
extern char stack_space[];
extern uint32_t *kpgdir;

struct tss_entry tss;
//TODO
uint32_t nextpid = 2;
struct context *kctx;

void exec_init(){
    uint32_t base = (uint32_t)&tss;
    uint32_t limit = sizeof(struct tss_entry) - 1;

    set_gdt(5, base, limit, 0xe9, 0);

    memset(&tss, 0, sizeof(struct tss_entry));
    tss.ss0 = 0x10;
    //tss.esp0 = (uint32_t)stack_space - 0x500;
    tss.esp0 = (uint32_t)stack_space;

	tss.cs = 0x0b;
	tss.ss = 0x13;
	tss.es = 0x13;
	tss.ds = 0x13;
	tss.fs = 0x13;
	tss.gs = 0x13;

    //load_tss(&tss);
    //enter_usermode();
}

//struct proc procs[128];
struct proc *procs;
struct proc *current;

#define PTE_P 0x001
#define PTE_W 0x002
#define PTE_U 0x004

void idle(){
    while(1)
        asm ("hlt");
}


//TODO
char pbuf[2048000];

void run_init(){
    struct proc *p1;
    char *sp;
    struct inode *in;
    struct elfh *e;
    struct elfph *ep;
    uint32_t *ustack;

    p1 = (struct proc *)kmalloc(sizeof(struct proc));

    memset(p1, 0, sizeof(struct proc));
    procs = p1;

    //TODO, this proc works but not expected, redo it.
    p1->pgdir = kpgdir;
    memcpy(p1->name, "idle", 5);
    p1->state = RUNNING;
    p1->pid = 0;
    p1->pwd = namei("/");

    p1->kstack = page_alloc();
    memset(p1->kstack, 0, 4096);

    sp = p1->kstack + 4096;

    sp -= sizeof(struct iframe);
    p1->frame = (struct iframe *)sp;
    p1->frame->cs = 0x08;
    p1->frame->ds = 0x10;
    p1->frame->ss = p1->frame->ds;
    p1->frame->eflags = 0x200;
    p1->frame->eip = (uint32_t)idle;

    sp -= sizeof(struct context);
    p1->context = (struct context *)sp;
    p1->context->eip = (uint32_t)switch_done;

    printk("run init\n");

    in = namei("/bin/init");
    if(!in){
        printk("failed to run init\n");
        goto done;
    }

    in->i_ops->read(in, pbuf, in->i_size);

    p1 = (struct proc *)kmalloc(sizeof(struct proc));
    memset(p1, 0, sizeof(struct proc));

    p1->next = procs;
    procs = p1;

    e = (struct elfh *)pbuf;
    if(e->e_magic != ELF_MAGIC){
        printk("not an elf file\n");
        goto done;
    }

    if(e->e_type != 2){
        printk("not an executable elf file\n");
        goto done;
    }

    p1->pgdir = pgdir_init();
    memset(p1->pgdir, 0, 4);
    memcpy(p1->name, "init", 5);
    p1->entry = e->e_entry;
    p1->pid = 1;
    p1->pwd = namei("/");
    //p1->next = &procs[1];

    //sp = p1->kstack + 4096 - sizeof(struct iframe);
    p1->kstack = page_alloc();
    memset(p1->kstack, 0, 4096);
    sp = p1->kstack + 4096;

    sp -= sizeof(struct iframe);
    p1->frame = (struct iframe *)sp;
    memset(p1->frame, 0, sizeof(struct iframe));
    p1->frame->cs = 0x1b;
    p1->frame->ds = 0x23;
    p1->frame->ss = p1->frame->ds;
    //  eax = 0, irq_no = 0, err_code = 0, eip = 0,
    //p1->frame->eax = 0x23;
    //p1->frame->irq_no = 0x1b;
    p1->frame->eflags = 0x200;
    p1->frame->eip = e->e_entry;
    //p1->frame->useresp = 0x1fffefff + 0x1000;
    p1->frame->useresp = 0x1ffff000 + 0x1000 - 0x10;

    //*(uint32_t *)sp = (uint32_t)switch_done;

    sp -= sizeof(struct context);
    p1->context = (struct context *)sp;

    memset(p1->context, 0, sizeof(struct context));
    p1->context->eip = (uint32_t)switch_done;

    ep = (struct elfph*) (((void *)e) + e->e_phoff);

    int i, ps;
    char *pb, *va, *pa;
    for(i = 0; i < e->e_phnum; i++){
        if(!(ep+i)->p_memsz)
            continue;
        ps = (ep+i)->p_memsz;
        va = (void *)(ep+i)->p_vaddr;
        pb = (void *)e + (ep+i)->p_offset;
        for(; ps > 0; ps -= 4096){
            pa = page_alloc();
            memset(pa, 0, 4096);
            mappages(p1->pgdir, va, (char*)V2P(pa), 1, PTE_U|PTE_W);
            //pte = walkpgdir(p1->pgdir, va);
            //pa = (uint32_t*)PTE_ADDR(*pte);
            if(ps > 4096){
                memcpy(pa, pb, 4096);
                va += 4096;
                pb += 4096;
            }else{
                memcpy(pa, pb, ps);
            }
        }
    }
    //char *pa = page_alloc();
    ////mappages(p1->pgdir, (uint32_t*)0x00000000, (uint32_t*)V2P(pa), 0x1000, PTE_U|PTE_W);
    //mappages(p1->pgdir, (char*)0x00000000, (char*)V2P(pa), 1, PTE_U|PTE_W);
    ustack = (uint32_t*)page_alloc();
    ////mappages(p1->pgdir, (uint32_t*)0x1fffefff, (uint32_t*)V2P(ustack), 0x1000, PTE_U|PTE_W);
    mappages(p1->pgdir, (char*)0x1ffff000, (char*)V2P(ustack), 1, PTE_U|PTE_W);

    p1->state = RUNNING;

done:
    return;
}

int proc_execve(const char *cmd, const char **argv){
    struct inode *in;

    in =namei(cmd);

    if(!in)
        goto done;

    in->i_ops->read(in, pbuf, in->i_size);

    struct proc *p1 = current;

    struct elfh *e = (struct elfh *)pbuf;
    if(e->e_magic != ELF_MAGIC){
        printk("not an elf file\n");
        goto done;
    }

    if(e->e_type != 2){
        printk("not an executable elf file\n");
        goto done;
    }

    uint32_t *oldpgdir, *newpgdir;
    oldpgdir = p1->pgdir;
    newpgdir = pgdir_init();

    char *oldkstack, *newkstack;
    newkstack = page_alloc();
    oldkstack = p1->kstack;

    struct iframe *tf = p1->frame;

    memcpy(p1->name, cmd, strlen(cmd));
    p1->state = RUNNING;
    p1->entry = e->e_entry;
    memset(newkstack, 0, 4096);
    p1->heap = NULL;

    char *sp = newkstack + 4096;

    p1->frame->eip = e->e_entry;

    sp -= sizeof(struct iframe);
    p1->frame = (struct iframe *)sp;
    *p1->frame = *tf;
    tf->eip = p1->frame->eip = e->e_entry;
    

    //*(uint32_t *)sp = (uint32_t)switch_done;

    sp -= sizeof(struct context);
    p1->context = (struct context *)sp;
    //p1->context->eip = (uint32_t)switch_done;
    p1->context->eip = e->e_entry;

    struct elfph *ep;

    ep = (struct elfph*) (((void *)e) + e->e_phoff);
    int ps;
    char *va, *pa;

    int i;
    char *pb;
    for(i = 0; i < e->e_phnum; i++){
        if(!(ep+i)->p_memsz)
            continue;
        ps = (ep+i)->p_memsz;
        va = (void *)(ep+i)->p_vaddr;
        pb = (void *)e + (ep+i)->p_offset;
        for(; ps > 0; ps -= 4096){
            pa = page_alloc();
            memset(pa, 0, 4096);
            mappages(newpgdir, va, (char*)V2P(pa), 1, PTE_U|PTE_W);

            //pte = walkpgdir(p1->pgdir, va);
            //pa = (uint32_t*)PTE_ADDR(*pte);

            if(ps > 4096){
                memcpy(pa, pb, 4096);
                va += 4096;
                pb += 4096;
            }else{
                memcpy(pa, pb, ps);
            }
        }
    }

    char *ustack = page_alloc();
    memset(ustack, 0, 4096);
    mappages(newpgdir, (char*)0x1ffff000, (char*)V2P(ustack), 1, PTE_U|PTE_W);

    ustack += 4096;
    if(argv != NULL){
        char *nu = ustack;
        int argc;
        int alen;
        ustack -= 40;
        uint32_t *argvv = (uint32_t*)ustack;
        for(argc = 0; argv[argc]; argc++){
            alen = strlen(argv[argc]) + 1;
            ustack -= (alen + 3) & ~3;
            memcpy(ustack, argv[argc], alen);
            //argvv[argc] = (uint32_t)ustack;
            argvv[argc] = (uint32_t) (0x1fffffff - (nu - ustack) + 1);
            //argvv++;
        }
        //ustack -= (alen + 5) & ~3;
        //*(uint32_t*)ustack = argc;
        ustack -= 4;
        //*(uint32_t*)ustack = (uint32_t)argvv;
        *(uint32_t*)ustack = (uint32_t) (0x1fffffff - (nu - (char*)argvv) + 1);
        ustack -= 4;
        *(uint32_t*)ustack = argc;
        ustack -= 4;
        //p1->frame->useresp = 0x1fffffff - (4096 - PTE_ADDR(ustack) - (uint32_t)ustack);
        //tf->useresp = p1->frame->useresp = 0x1ffff0ff;
        tf->useresp = p1->frame->useresp = 0x1fffffff - (nu - ustack) + 1;
    }else{
        tf->useresp = p1->frame->useresp = 0x1fffffdf;
    }

    cli();
    exec_init();
    tss.esp0 = (uint32_t)newkstack + 4096;
    load_tss();
    load_cr3(V2P(newpgdir));
    p1->pgdir = newpgdir;
    p1->kstack = newkstack;
    pgdir_free(oldpgdir);
    page_free(oldkstack);
    sti();

    return 0;

done:
    return -1;
}

void proc_init(){
    run_init();
}

int should_sched(){
    //TODO
    int ret = 0;
    if(current)
        ret = 1;
    return ret;
}
void scheduler(){
    struct proc *p;

    while(1){
        for(p = procs; p; p = p->next){
            if(p->state != RUNNING)
                continue;
            current = p;
            if(current->pid == 0){
                switch_to(&kctx, p->context);
            }else{
                exec_init();
                tss.esp0 = (uint32_t)p->kstack + 4096;
                load_tss();
                load_cr3(V2P(p->pgdir));
                switch_to(&kctx, p->context);
                load_cr3(V2P(kpgdir));
                current = 0;
            }
        }
    }
}

void sched(){
    if(should_sched())
        switch_to(&current->context, kctx);
}
