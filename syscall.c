#include "deltaos.h"
#include "console.h"
#include "ext2.h"
#include "string.h"
#include "memory.h"
#include "proc.h"

extern void switch_done();
extern void sched();

int sys_write(void *p);
int sys_read(void *p);
int sys_fork(void *p);
int sys_wait(void *p);
int sys_execve(void *p);
int sys_exit(void *p);
int sys_open(void *p);
int sys_getpid(void *p);
int sys_sbrk(void *p);
int sys_close(void *p);
int sys_isatty(void *p);
int sys_fstat(void *p);
int sys_lseek(void *p);
int sys_kill(void *p);
int sys_stat(void *p);
int sys_chdir(void *p);
 
int (*syscalls[])(void *p) = {
[0] = sys_write,
[1] = sys_read,
[2] = sys_fork,
[3] = sys_wait,
[4] = sys_execve,
[5] = sys_exit,
[6] = sys_open,
[7] = sys_getpid,
[8] = sys_sbrk,
[9] = sys_close,
[10] = sys_isatty,
[11] = sys_fstat,
[12] = sys_lseek,
[13] = sys_kill,
[14] = sys_stat,
[15] = sys_chdir,
};

int sys_write(void *p){
    //int fd;
    char *s;
    uint32_t len;
 
    //fd = *(uint32_t *)(p+4);
    s = (char *)*(uint32_t *)(p+8);
    len = *(uint32_t *)(p+12);
 
    //for(i = 0; i < len; i++){
    //    console_write(s[i]);
    //}
    console_write(s, len);

    return len;
}
 
int keyboard_read(char *s, int len);

int sys_read(void *p){
    int fd;
    char *s;
    uint32_t count;
    int ret;
    struct inode *inode;

    fd = *(uint32_t *)(p+4);
    s = (char *)*(uint32_t *)(p+8);
    count = *(uint32_t *)(p+12);

    if(fd < 3){
        //ret = keyboard_read(s, count);
        ret = console_read(s, count);
    }else{
        inode = current->fds[fd];

        if(!inode){
            ret = -1;
        }

        ret = inode->i_ops->read(inode, s, count);
    }

    return ret;
}

int sys_fork(void *p){
    struct proc *p1 = current;
    char *sp;

    struct proc *p2 = (struct proc *)kmalloc(sizeof(struct proc));
    memset(p2, 0, sizeof(struct proc));
    //memcpy(p2, p1, sizeof(struct proc));
    p2->pgdir = pgdir_init();
    p2->parent = p1;
    p1->child = p2;
    p2->state = SLEEP;

    strcpy(p2->name, p1->name);
    uint32_t *pte;

    // TODO, program should start from 0x08048000;
    char *va = (char *)0x08048000;
    char *pa;
    char *m;
    pte = walkpgdir(p1->pgdir, va);
    while(*pte & PTE_P){
        pa = (char *)PTE_ADDR(*pte);
        m = page_alloc();
    //memset(m, 0, 4096);
        memcpy(m, (void *)P2V(pa), 4096);
        mappages(p2->pgdir, va, (char *)V2P(m), 1, PTE_U|PTE_W);
        va += 4096;
        pte = walkpgdir(p1->pgdir, va);
    }

    //pte = walkpgdir(p1->pgdir, (char *)0);
    //uint32_t pa = PTE_ADDR(*pte);
    //uint32_t *m = (uint32_t *)page_alloc();
    ////memset(m, 0, 4096);
    //memcpy(m, (void *)P2V(pa), 4096);
    //mappages(p2->pgdir, (char *)0, (char *)V2P(m), 1, PTE_U|PTE_W);

    pte = walkpgdir(p1->pgdir, (char *)0x1ffff000);
    pa = (char *)PTE_ADDR(*pte);
    m = page_alloc();
    //memset(m, 0, 4096);
    memcpy(m, (void *)P2V(pa), 4096);
    mappages(p2->pgdir, (char *)0x1ffff000, (char *)V2P(m), 1, PTE_U|PTE_W);

    //asm ("cli; hlt");

    p2->kstack = page_alloc();
    memset(p2->kstack, 0, 4096);
    p2->pid = nextpid++;
    p2->next = procs;
    procs = p2;
    sp = p2->kstack + 4096 - sizeof(struct iframe);

    //sp -= sizeof(struct frame);
    p2->frame = (struct iframe *)sp;
    *p2->frame = *p1->frame;
    //p2->frame->esi = p2->frame->ebp;
    p2->frame->eax = 0;
    //p2->frame->cs = 0x1b;
    //p2->frame->ds = 0x23;
    //p2->frame->es = p2->frame->ds;
    //p2->frame->fs = p2->frame->ds;
    //p2->frame->gs = p2->frame->ds;
    //p2->frame->ss = p2->frame->ds;
    //p2->frame->eflags = 0x200;
    ////p2->frame->eip = 0;
    //p2->frame->eip = (uint32_t)*(&p+23);
    ////p2->frame->useresp = 0x1fffefff + 0x1000;
    //p2->frame->useresp = (uint32_t)*(&p+26);

    //int fd;
    struct inode *inode;
    //for(fd = 3; fd < MAFD && p1->fds[fd]; fd++){
    //    inode = kmalloc(sizeof(struct inode));
    //    *inode = *p1->fds[fd];
    //    p2->fds[fd] = inode;
    //    printk("sys_fork %x\n", inode);
    //}
    inode = kmalloc(sizeof(struct inode));
    *inode = *p1->pwd;
    p2->pwd = inode;

    sp -= sizeof(struct context);
    p2->context = (struct context *)sp;
    p2->context->eip = (uint32_t)switch_done;
    p2->state = RUNNING;

    return p2->pid;
}

void proc_free(struct proc *proc){
    struct proc *prev, *p;
    prev = p = procs;

    if(proc == procs)
        procs = proc->next;
    else{
        for(; p; p = p->next){
            if(p == proc)
                prev->next = p->next;
            prev = p;
        }
    }
}

int clean_child(struct proc *proc){
    int pid;
    struct proc *p2;
    p2 = procs;
    for(;;){
        if(p2->parent != proc){
            p2 = p2->next;
            continue;
        }
        if(p2->state == ZOMBIE){
            pid = p2->pid;
            page_free((void *)p2->kstack);
            pgdir_free(p2->pgdir);
            proc_free(p2);
            kfree(p2);
            return pid;
         }
    }
}

int sys_wait(void *p){
    struct proc *p1 = current;
    int childpid;

    while(!p1->child);
    while(p1->child->state != RUNNING);

    // make sure child are running
    if(p1->child->state != RUNNING)
        childpid = clean_child(p1);
    else{
        p1->state = SLEEP;
        sched();
        childpid = clean_child(p1);
    }

    return childpid;
}

extern int proc_execve(const char *, const char **);

int sys_execve(void *p){
    const char *cmd;
    const char **argv;

    cmd = (const char *)*(uint32_t *)(p+4);
    argv = (const char **)*(uint32_t *)(p+8);

    return proc_execve(cmd, argv);
}

int sys_exit(void *p){
    //current->state = SLEEP;
    //memset(current->kstack + 4092, 0, 4);
    current->frame->eip = 0x1fffffff;

    return 0;
}

//TODO, free inode when close the proc
int sys_open(void *p){
    const char *name;
    int i = -1;
    struct inode *inode;

    name = (const char *)*(uint32_t *)(p+4);

    inode = namei(name);
    if(!inode)
        goto error;

    i = 3;
    while(current->fds[i] != NULL){
        if(i++ > MAXFD){
            goto error;
        }
    }

    current->fds[i] = inode;

error:
    return i;
}

int sys_getpid(void *p){
    struct proc *proc;

    proc = current;

    if(proc && proc->pid != 0)
        return proc->pid;

    return -1;
}

int sys_sbrk(void *p){
    uint32_t incr;
    char *pa;
    uint32_t *pgdir = current->pgdir;
    char *heap, *oldheap;
    uint32_t *pte;

    oldheap = heap = current->heap;

    incr = *(uint32_t*)(p+4);

    if(heap){
        if(incr == 0)
            return (uint32_t)current->heap;
        else if(incr < 4096)
            incr = 4096;
        do{
            pte = walkpgdir(pgdir, heap);
            while(*pte & PTE_P){
                heap += 4096;
                pte = walkpgdir(pgdir, heap);
            }
            pa = page_alloc();
            //memset(pa, 0, 4096);
            mappages(current->pgdir, heap, (char*)V2P(pa), 1, PTE_U|PTE_W);
            incr -= 4096;
        }while(incr > 4096);

        current->heap = heap;
    }else{
        heap = (char*)PTE_ADDR(incr);
        pte = walkpgdir(pgdir, heap);
        while(*pte & PTE_P){
            heap += 4096;
            pte = walkpgdir(pgdir, (heap));
        }
        pa = page_alloc();
        mappages(pgdir, heap, (char*)V2P(pa), 1, PTE_U|PTE_W);
        oldheap = current->heap = heap;
    }

    return (uint32_t)oldheap;
}

int sys_close(void *p){
    int fd;
    struct inode *inode;

    fd = *(uint32_t *)(p+4);

    if(fd < 0){
        return -1;
    }

    if(fd > 3){
        return 0;
    }

    inode = current->fds[fd];

    if(!inode){
        return -1;
    }

    kfree(inode);

    current->fds[fd] = NULL;

    return 0;
}

int sys_isatty(void *p){
    int fd;
    //struct inode *inode;

    fd = *(uint32_t *)(p+4);

    //inode = current->fds[fd];
    if(fd < 2)
        return 1;
    else
        return 0;
}

struct timespec {                      
    uint32_t tv_sec;                        
    uint32_t tv_nsec;                         
};

struct stat{
  uint16_t st_dev;
  uint16_t st_ino;
  uint32_t st_mode;
  uint16_t st_nlink;
  uint16_t st_uid;
  uint16_t st_gid;
  uint16_t st_rdev;
  uint32_t st_size;
  struct timespec st_atim;
  struct timespec st_mtim;
  struct timespec st_ctim;
  uint32_t    st_blksize;
  uint32_t st_blocks;
  uint32_t st_spare4[2];
};

int sys_fstat(void *p){
    int fd;
    struct stat *st;
    //struct inode *inode;

    fd = *(uint32_t *)(p+4);
    st = (struct stat *)*(uint32_t *)(p+8);

    // TODO
    if(fd < 2 || fd > MAXFD)
        goto err;

    struct inode *inode;
    inode = current->fds[fd];
    if(!inode)
        goto err;

    memset(st, 0, sizeof(struct stat));

    st->st_uid = 0;
    st->st_gid = 0;
    st->st_ino = inode->i_ino;
    st->st_mode = inode->i_mode;
    st->st_size = inode->i_size;

    return 0;

err:
    return -1;
}

int sys_lseek(void *p){
    //int fd;
    //struct inode *inode;
            
    //fd = *(uint32_t *)(p+4);
            
    //inode = current->fds[fd];
    printk("sys_lseek\n");
    return 1;
}

int sys_kill(void *p){
    int pid;
    int sig;
    //struct inode *inode;

    pid = *(uint32_t *)(p+4);
    sig = *(uint32_t *)(p+8);

    printk("sys_kill %d %d\n", pid, sig);

    return -1;
}

int sys_stat(void *p){
    const char *name;
    struct inode *inode;
    struct stat *st;

    name = (const char *)*(uint32_t *)(p+4);
    st = (struct stat *)*(uint32_t *)(p+8);

    inode = namei(name);
    if(!inode)
        goto error;

    memset(st, 0, sizeof(struct stat));
    st->st_uid = 0;
    st->st_gid = 0;
    st->st_ino = inode->i_ino;
    st->st_mode = inode->i_mode;
    st->st_size = inode->i_size;

    return 0;

error:
    return -1;
}

int sys_chdir(void *p){
    const char *path;
    struct inode *inode;

    path= (const char *)*(uint32_t *)(p+4);

    inode = namei(path);
    if(!inode)
        goto error;

//TODO
    strcpy(inode->name, path);
    current->parent->pwd = inode;

    return 0;

error:
    return -1;
}

