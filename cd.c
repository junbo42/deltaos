#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

int main(int argc, char *argv[])
{
    int fd;
    char path[128];

    if(argc > 1){
        memcpy(path, argv[1], strlen(argv[1])+1);
    }else{
        memcpy(path, "/", 2);
    }

    fd = open(path, O_RDONLY);

    if(fd < 0){
        printf("%s is not a directory\n", path);
        return -1;
    }

    struct stat st;
    if(fstat(fd, &st) < 0){
        printf("fstat failed\n");
        return -1;
    }

    if(S_ISDIR(st.st_mode)){
        chdir(path);
    }else{
        printf("%s is not a directory\n", path);
    }

    close(fd);
}
