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
    char path[255];

    if(argc < 2){
        printf("stat <file>\n");
        return -1;
    }

    fd = open(argv[1], O_RDONLY);

    if(fd < 0){
        printf("failed to open a.txt\n");
        return -1;
    }

    struct stat st;
    if(fstat(fd, &st) < 0){
       printf("fstat failed\n");
    }

    printf("mode %x\n", st.st_mode);
    printf("nlink %x\n", st.st_nlink);
    printf("size %x\n", st.st_size);
    printf("rdev %x\n", st.st_rdev);

    close(fd);
}
