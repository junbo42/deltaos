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
        printf("cat <file>\n");
        return -1;
    }

    fd = open(path, O_RDONLY);

    if(fd < 0){
        printf("failed to open %s\n", path);
        return -1;
    }

    struct stat st;
    if(fstat(fd, &st) < 0){
        printf("fstat failed\n");
        return -1;
    }

    char buf[128];
    if(S_ISREG(st.st_mode)){
        while(read(fd, buf, sizeof(buf))){
            write(1, buf, sizeof(buf));
            memset(buf, 0, sizeof(buf));
        }
    }else{
        printf("%s is not a text file\n", path);
    }

    close(fd);
}
