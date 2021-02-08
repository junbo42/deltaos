extern int main(int argc, char **argv);
extern void _exit(int ret);
extern char **environ;
 
extern char _end[];
extern char __bss_start[];

void _start(int argc, char **argv, char **env){

    environ = env;
    char *p;

    for(p = __bss_start; p < _end; p++)
        *p = 0;

    main(argc, argv);

    _exit(0);
}
