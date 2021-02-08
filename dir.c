#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

struct dirent{
    uint32_t inode;
    char name[255];
};

char path[255];

int main(int argc, char *argv[])
{
    int fd;

    memset(path, 0, 255);

    if(argc < 2){
        strcpy(path, ".");
    }else{
        strcpy(path, argv[1]);
    }

    fd = open(path, O_RDONLY);

    if(fd < 0){
        printf("failed to open %s\n", path);
        return -1;
    }

    struct stat st;
    if(fstat(fd, &st) < 0){
       printf("fstat failed\n");
    }

    struct dirent de;
    memset(&de, 0, sizeof(struct dirent));

    if(S_ISDIR(st.st_mode)){
        while(read(fd, &de, 1)){
            printf("%s %lu\n", de.name, de.inode);
            memset(&de, 0, sizeof(struct dirent));
        }
    }else{
        printf("%s %lu\n", path, st.st_ino);
    }

    close(fd);
}
