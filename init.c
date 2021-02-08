#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>

//#define TEST

char *strstrip(char *s);
char buf[256];

char cmd1[16] = "a";
char cmd2[16] = "dir"; 
char cmd3[16] = "b";
char cmd4[16] = "cat a.txt";
char cmd5[16] = "cd /bin";
char cmd6[16] = "dir";
char cmd7[16] = "b";
char cmd8[16] = "cd";
char *test[] = {cmd1, cmd2, cmd3, cmd4, cmd5, cmd6, cmd7, cmd8};
int testn = 10000;

int main(int argc, char *argv[])
{
    int pid;
    struct stat st;
    char *col;
    char *cole;
    char *cmd;
    char cmdp[64];
    char *newargv[10];
    int idx;

    printf("\nDelta Network Operating System\n\n");

    char *p = buf;
    //while(1){
    //    read(0, p, 1);
    //    if(*p == '\n'){
    //        printf("input %s", buf);
    //        memset(buf, 0, sizeof(buf));
    //        p = buf;
    //    }else if(*p == '\b'){
    //        write(1, p, 1);
    //        p--;
    //    }else{
    //        write(1, p, 1);
    //        p++;
    //    }
    //}

    //char *p;
    while(1){
        //printf("1111 %p\n", buf);
        memset(buf, 0, 256);
        memset(newargv, 0, 40);
        p = buf;
        idx = 0;

        write(1, "# ", 3);
#ifdef TEST
        memcpy(buf, test[testn-- % 8], 16);
        printf("%d cmd %s\n", testn, buf);
 
        if(testn<0){
            printf("init done\n");
            while(1);
        }
#else
        while(1){

            read(0, p, 1);
            if(*p == '\n'){
                //printf("1 %d\n", *p);
                //printf("input %s", buf);
                //memset(buf, 0, sizeof(buf));
                //p = buf;
                //if(strlen(buf) == 1){
                //    write(1, p, 1);
                //    write(1, "# ", 3);
                //}else{
                //p++;
                //*p = 0;
                write(1, p, 1);
                break;
                //}
            }else if(*p == '\b'){
                if(p == buf)
                    continue;
                write(1, p, 1);
                *p = 0;
                p--;
            }else{
                write(1, p, 1);
                p++;
            }

            //if(*p == '\n' || p - buf > 64)
            //    break;

            //p++;
        }
#endif
        p = strstrip(buf);
        if(*p == 0)
            continue;

        col = strtok_r(p, " ", &cole);

        if(*col != '/'){
            memset(cmdp, 0, sizeof(cmdp));
            strcpy(cmdp, "/bin/");
            strcat(cmdp, col);
        }

        if(stat(cmdp, &st) < 0){
            printf("%s command not found\n", col);
            continue;
        }

        newargv[idx++] = col;
        col = strtok_r(NULL, " ", &cole);
        while(col){
            newargv[idx++] = col;
            col = strtok_r(NULL, " ", &cole);
        }

        pid = fork();

        if(pid == 0){
            execve(cmdp, newargv, NULL);
            printf("command not found\n");
            exit(1);
        }else{
            //printf("execute %s, pid %lu\n", col, pid);
            wait(NULL);
        }
    }
}
