#include "deltaos.h"
#include "screen.h"
#include "ext2.h"
#include "memory.h"
#include "proc.h"
#include "string.h"

extern struct ext2_inode *rootin;
extern struct inode_ops ext2_file_ops;
extern struct inode_ops ext2_dir_ops;

static struct inode *nameiroot(struct inode *inode){

    inode->i_ino = 2;
    inode->i_off = 0;
    inode->i_size = rootin->i_size;
    inode->ext2_inode = *rootin;
    inode->i_ops = &ext2_dir_ops;
    inode->i_mode = rootin->i_mode;

    return inode;
}

struct inode *namei(const char *path){
    struct ext2_inode *in = kmalloc(sizeof(struct ext2_inode));
    char *col, *cole;
    struct bbuf *bbuf;
    int found = 0;
    int filetype = 0;
    char pth[128] = "";
    memcpy(pth, path, strlen(path));
    struct ext2_dir_entry *e;
    struct inode *inode = kmalloc(sizeof(struct inode));

    if(!strcmp(path, "/")){
        return nameiroot(inode);
    }

    if(path[0] == '.')
        return current->pwd;

    if(path[0] == '/')
        *in = *rootin;
    else
        *in = current->pwd->ext2_inode;

    col = strtok_r(pth, "/", &cole);

    while(col != NULL){
        int i = 0;

        while(in->i_block[i]){
            //b.valid = 0;
            //b.p = kmalloc(1024);
            //ide_read(in->i_block[i++], &b);
            bbuf = bread(in->i_block[i++]);
            e = (struct ext2_dir_entry *)bbuf->data;
            int size = 0;
            found = 0;
                
            //printk("in %p %d\n", in, in->i_block[0]);
            char buf[255];
            while(size < in->i_size){
                if(!e->inode)
                    break;
                memset(buf, 0, 255);
                memcpy(buf, e->name, e->name_len);
                //printk("namei %s %d\n", buf, e->file_type);
                if(!strcmp(col, buf)){
                    //printk("found %s %d\n", col, e->inode);
                    filetype = e->file_type;
                    read_ext2_inode(e->inode, in);
                    found = 1;
                    break;
                }
                //printk("%d %d %s\n", e->inode, e->file_type, buf);
                e = ((void *)e) + e->rec_len;
                //TODO, determine end of the list
                if(!e->rec_len)
                    break;
                size += e->rec_len;
            }
            if(found)
                break;
        }

        col = strtok_r(NULL, "/", &cole);
        if(col && col[strlen(col)-1] == '\n')
            col[strlen(col)-1] = 0;
    }

    if(!found){
        kfree(in);
        return NULL;
    }

    inode->i_ino = e->inode;
    inode->i_off = 0;
    inode->i_size = in->i_size;
    inode->i_mode = in->i_mode;
    inode->ext2_inode = *in;

    if(filetype == EXT2_FT_REG_FILE){
        inode->i_ops = &ext2_file_ops;
    }else if(filetype == EXT2_FT_DIR){
        inode->i_ops = &ext2_dir_ops;
    }

    kfree(in);
    return inode;
}

