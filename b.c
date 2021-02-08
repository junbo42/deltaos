#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>

int main(int argc, char *argv[])
{
    //char *path = "/a.txt";
    //int fd = open(path, O_RDONLY);

    //if(fd < 0){
    //    printf("failed to open %s\n", path);
    //}

    //printf("%d\n", close(fd));

    //printf("%f\n", 1.001);

    //printf("aaaa %d\n", 1);
    //printf("hello\n");
    //printf("hello\n");
    //printf("hello\n");
    //printf("hello\n");
    //printf("hello\n");
    //printf("hello\n");
    //printf("hello\n");

    //char *s;

    //printf("%p\n", s);
    //s = malloc(10240);
    //s = malloc(10240);
    //printf("%p\n", s);

    //printf("%p\n", sbrk(0));
    //printf("%p\n", sbrk(0));
    //printf("%p\n", sbrk(0x1000));
    //printf("%p\n", sbrk(0x1000));
    //printf("%p\n", sbrk(0));
    //printf("%p\n", sbrk(0));
    printf("%p\n", sbrk(0));
    //printf("%p\n", sbrk(0x2000));
    //printf("%p\n", sbrk(0x3000));
    //printf("%p\n", sbrk(0x4000));

    int pid;

    pid = fork();
    if(pid == 0){
        printf("child pid %d\n", getpid());
        //char *args[] = {"/bin/a", NULL};
        execve("/bin/a", NULL, NULL);
    }else{
        printf("parent pid %d\n", getpid());
        wait(NULL);
    }

    //exit(0);

    //if(pid == 0){
    //    char *args[] = {"/bin/a", NULL};
    //    execve("/bin/a", args, NULL);
    //}else{
    //    printf("pid %d\n", pid);
    //    //wait1();
    //}

}

