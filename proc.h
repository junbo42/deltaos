#include "ext2.h"

#define MAXFD 512

enum proc_state {SLEEP, RUNNING, ZOMBIE};

struct frame{
    uint32_t ds;
    uint32_t es;
    uint32_t fs;
    uint32_t gs;
    uint32_t eip;
    uint32_t cs;
    uint32_t eflags;
    uint32_t useresp;
    uint32_t ss;
};

struct iframe{
   uint32_t ds;                                     // Data segment selector
   uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax; // Pushed by pusha.
   uint32_t irq_no, err_code;                       // Interrupt number and error code (if applicable)
   uint32_t eip, cs, eflags, useresp, ss;           // Pushed by the processor automatically.
}; 

struct context{
    uint32_t edi;
    uint32_t esi;
    uint32_t ebx;
    uint32_t ebp;
    uint32_t eip;
};

struct proc{
    enum proc_state state;
    struct proc *next;
    struct proc *parent;
    struct proc *child;
    char name[32];
    int pid;
    uint32_t *pgdir;
    uint32_t entry;
    char *kstack;
    char *heap;
    struct inode *fds[MAXFD];
    struct inode *pwd;
    //struct frame *frame;
    struct iframe *frame;
    struct context *context;
};

struct tss_entry{
    uint32_t prev_tss;
    uint32_t esp0;
    uint32_t ss0;
    uint32_t esp1;
    uint32_t ss1;
    uint32_t esp2;
    uint32_t ss2;
    uint32_t cr3;
    uint32_t eip;
    uint32_t eflags;
    uint32_t eax;
    uint32_t ecx;
    uint32_t edx;
    uint32_t ebx;
    uint32_t esp;
    uint32_t ebp;
    uint32_t esi;
    uint32_t edi;
    uint32_t es;         
    uint32_t cs;        
    uint32_t ss;        
    uint32_t ds;        
    uint32_t fs;       
    uint32_t gs;         
    uint32_t ldt;      
    uint16_t trap;
    uint16_t iomap_base;
} __attribute__((packed));
 
extern struct tss_entry tss;
//extern struct proc procs[128];
extern struct proc *procs;
extern struct proc *current;
extern uint32_t nextpid;
