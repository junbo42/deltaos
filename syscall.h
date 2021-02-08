
struct devsw{
  int (*read)(char*, uint32_t);
  int (*write)(char*, uint32_t);
};
 
struct devsw devsw[2];
 
void sys_write(void *p);                                                                                                                                                 
void sys_read(void *p);
 
//extern void (*syscalls[])(void *p);
extern int (*syscalls[])();
