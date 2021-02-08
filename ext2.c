#include "deltaos.h"
#include "screen.h"
#include "string.h"
#include "memory.h"
#include "ext2.h"

#define EXT2_SIGNATURE  0xef53

#define EXT2_D_UNKNOW   0
#define EXT2_D_FILE     1
#define EXT2_D_DIR      2
#define EXT2_D_CHAR     3
#define EXT2_D_BLOCK    4
#define EXT2_D_FIFO     5
#define EXT2_D_SOCK     6
#define EXT2_D_SYM      7

struct ext2_inode *rootin;
static struct super_block *sp;
static struct group_desc *gds;
static int num_of_groups;
static int block_size;

void ide_read(uint32_t, struct bbuf*);
struct bbuf *bread(int block);
void bbuf_init();

#ifdef EXT2_DBG

static void ext2_print_sp(){
    printk("block count %d\n", sp->s_blocks_count);
    printk("block size %d\n", block_size);
    printk("blocks per group %d\n", sp->s_blocks_per_group);
    printk("inodes per group %d\n", sp->s_inodes_per_group);
    printk("block group number %d\n", num_of_groups);
    printk("first data block %d\n", sp->s_first_data_block);
    printk("major version %d\n", sp->s_rev_level);
    printk("minor version %d\n", sp->s_minor_rev_level);
    printk("\n");
}

static void ext2_print_gds(){
    int first, last;

    for(int i = 0; i < num_of_groups ; i++){
        first = sp->s_first_data_block + sp->s_blocks_per_group * i;
        last = sp->s_first_data_block + sp->s_blocks_per_group * (i + 1) - 1;
        if(last > sp->s_blocks_count)
            last = sp->s_blocks_count - 1;
        printk("block group %d %d-%d\n", i, first, last);
        printk("free block %d\n", gds[i].bg_free_blocks_count);
        printk("free inode %d\n", gds[i].bg_free_inodes_count);
        printk("block bitmap %d\n", gds[i].bg_block_bitmap);
        printk("inode bitmap %d\n", gds[i].bg_inode_bitmap);
        printk("directory count %d\n", gds[i].bg_used_dirs_count);
        printk("inode table %d\n", gds[i].bg_inode_table);
        printk("\n");
    }
}

#endif

void read_ext2_inode(int i, struct ext2_inode *einode){
    struct group_desc *gd;
    struct ext2_inode *in;
    int index;
    struct bbuf *bbuf;

    i--;
    index = i / sp->s_inodes_per_group;
    gd = gds + index;

    //b.valid = 0;
    //b.p = kmalloc(1024);
    int o1 = i % sp->s_inodes_per_group;
    int o2 = o1 % 8;
    //ide_read(gd->bg_inode_table, &b);
    //ide_read(gd->bg_inode_table + (o1 / 8), &b);
    bbuf = bread(gd->bg_inode_table + (o1 / 8));

    in = (struct ext2_inode *)bbuf->data;
    //in += (i % sp->s_inodes_per_group);
    in += o2;

    *einode = *in;
}

void read_inode(int i, struct inode *inode){
    read_ext2_inode(i, &inode->ext2_inode);
}


//static void list_dir(struct ext2_inode *in){
//    int i = 0;
//    struct buf b;
//
//    while(in->i_block[i]){
//        b.valid = 0;
//        b.p = kmalloc(1024);
//        ide_read(in->i_block[i], &b);
//        struct ext2_dir_entry *e = (struct ext2_dir_entry *)b.p;
//        int size = 0;
//
//        //printk("in %p %d\n", in, in->i_block[0]);
//
//        char buf[255];
//        while(size < in->i_size){
//            if(!e->inode)
//                break;
//            memset(buf, 0, 255);
//            memcpy(buf, e->name, e->name_len);
//            printk("%d %d %s\n", e->inode, e->file_type, buf);
//            e = ((void *)e) + e->rec_len;
//            size += e->rec_len;
//        }
//
//        i++;
//    }
//}

//static int process_blocks(char **offset, uint32_t *size, uint32_t index){
//    int i;
//    char *p = *offset;
//    struct buf b;
//
//    b.valid = 0;
//    b.p = kmalloc(1024);
//    ide_read(index, &b);
//    uint32_t *block = (uint32_t *)b.p;
//
//    for(i = 0; i < 256; i++){
//        b.valid = 0;
//        b.p = kmalloc(1024);
//        ide_read(block[i], &b);
//        //d = (addr + block_size * block[i]);
//        if(*size > 1024){
//            memcpy(p, b.p, 1024);
//            *size -= 1024;
//            p += 1024;
//        }else{
//            memcpy(p, b.p, *size);
//            p += *size;
//            *size -= *size;
//            *offset = p;
//            return 0;
//        }
//    }
//    *offset = p;
//    return 1;
//}

static int process_blocks(char **offset, uint32_t *size, uint32_t index){
    int i;
    char *p = *offset;
    struct bbuf *bbuf;

    //b.valid = 0;
    //b.p = kmalloc(1024);
    //ide_read(index, &b);
    bbuf = bread(index);
    uint32_t *block = (uint32_t *)bbuf->data;

    for(i = 0; i < 256; i++){
        bbuf = bread(block[i]);
        //d = (addr + block_size * block[i]);
        if(*size > 1024){
            memcpy(p, bbuf->data, 1024);
            *size -= 1024;
            p += 1024;
        }else{
            memcpy(p, bbuf->data, *size);
            p += *size;
            *size -= *size;
            *offset = p;
            goto done;
        }
    }

done:
    *offset = p;

    return 1;
}

char prog[10240];

char *get_content(struct ext2_inode *in){
    char *p;
    int i;
    uint32_t size = in->i_size;

    p = prog;
    //printk("11 %p %p\n", buf, p);

    // i_block[0 - 11]
    for(i = 0; i <= 11; i++){
        struct bbuf *bbuf;

        //b.valid = 0;
        //b.p = kmalloc(1024);
        //ide_read(in->i_block[i], &b);
        bbuf = bread(in->i_block[i]);
        if(size > 1024){
            memcpy(p, bbuf->data, 1024);
            size -= 1024;
            p += 1024;
        }else{
            memcpy(p, bbuf->data, size);
            p += size;
            size -= size;
            goto done;
        }
    }

    printk("size %d\n", size);
    // i_block[12]
    if(!process_blocks(&p, &size, in->i_block[12]))
        goto done;

    // > 274432

    //// i_block[13]
    //b.valid = 0;
    //b.p = kmalloc(1024);
    //ide_read(in->i_block[13], &b);
    //uint32_t *dblock = (uint32_t *)b.p;
    //for(i = 0; i < 256; i++){
    //    if(!process_blocks(&p, &size, dblock[i]))
    //        goto done;
    //}

done:
    //remove("aa");
    //int fd = open("aa", O_WRONLY|O_CREAT|O_SYNC, 0666);
    //if(fd == -1)
    //    printk("open failed\n");
    //printk("%d\n", write(fd, buf, in->i_size));
    //close(fd);
    return prog;
}

uint32_t get_content2(struct inode *inode, char *dst, size_t count){
    int i;
    size_t n = count;
    uint32_t *off = &inode->i_off;
    size_t ret = inode->i_off;
    uint32_t size = inode->ext2_inode.i_size;
    struct bbuf *bbuf;

    int block, block_off;
    block = *off / 1024;
    block_off = *off % 1024;

    if(*off >= size)
        return 0;

    if(count > size)
        n = size;

    char *c;
    // i_block[0 - 11]
    if(*off < 12288){
        for(; block < 12; block++){

            //b.valid = 0;
            //b.p = kmalloc(1024);
            //ide_read(inode->ext2_inode.i_block[block], &b);
            c = bread(inode->ext2_inode.i_block[block])->data;

            if(block_off){
                size_t r = 1024 - block_off;
                c += block_off;
                if(n > r){
                    memcpy(dst, c, r);
                    *off += r;
                    n -= r;
                    dst += r;
                }else{
                    memcpy(dst, c, n);
                    *off += n;
                    n -= n;
                    dst += n;
                    goto done;
                }
                block_off = 0;
                continue;
            }
            if(n > 1024){
                memcpy(dst, c, 1024);
                *off += 1024;
                n -= 1024;
                dst += 1024;
            }else{
                memcpy(dst, c, n);
                *off += n;
                n -= n;
                dst += n;
                goto done;
            }
        }
    }
    if(*off < 274432){

        //TODO, count ret
        // i_block[12]
        if(!process_blocks(&dst, &n, inode->ext2_inode.i_block[12]))
            goto done;

    }
    if (*off < 67383296){

        // i_block[13]
        //b.valid = 0;
        //ide_read(inode->ext2_inode.i_block[13], &b);
        bbuf = bread(inode->ext2_inode.i_block[13]);
        uint32_t *dblock = (uint32_t *)bbuf->data;
        for(i = 0; i < 256; i++){
            if(!process_blocks(&dst, &n, dblock[i]))
                goto done;
        }

    }else{
        printk("file to large to read\n");
    }

done:
    return *off - ret;
}

int ext2_file_read(struct inode *inode, char *dst, size_t count){
    int ret = get_content2(inode, dst, count);
    return ret;
}

struct inode_ops ext2_file_ops = {
    &ext2_file_read,
};

int ext2_dir_read(struct inode *inode, char *dst, size_t count){
    struct ext2_dir_entry *e;
    struct bbuf *bbuf;
    int ret = inode->i_off;
    uint32_t *off = &inode->i_off;
    struct dirent *de = (struct dirent *)dst;
    int i;

    bbuf = bread(inode->ext2_inode.i_block[0]);
    e = (struct ext2_dir_entry *)bbuf->data;
    for(i = 0; i < *off; i++){
        e = ((void *)e) + e->rec_len;
        if(!e->rec_len){
            goto done;
        }
    }
//TODO
    while(count){
        memcpy(de->name, e->name, e->name_len);
        de->inode = e->inode;
        *off += 1;
        count--;
    }

done:
    return *off - ret;
}

struct inode_ops ext2_dir_ops = {
    &ext2_dir_read,
};

void ext2_init(){
    struct bbuf *bbuf;

    bbuf_init();

    printk("initializing file system\n");

    bbuf = bread(1);

    sp = (struct super_block *)bbuf->data;

    if(sp->s_magic != EXT2_SIGNATURE){
        printk("this is not an ext2 filesystem\n");
        goto err;
    }

    block_size = 1024 << sp->s_log_block_size;
    num_of_groups = (sp->s_blocks_count - 1) / sp->s_blocks_per_group + 1;

    //b.valid = 0;
    //b.p = kmalloc(1024);
    //ide_read(2, &b);
    bbuf = bread(2);

    gds = (struct group_desc *)bbuf->data;

#ifdef EXT2_DBG
    ext2_print_sp();
    ext2_print_gds();
#endif

    rootin = (struct ext2_inode *)kmalloc(sizeof(struct ext2_inode));
    read_ext2_inode(2, rootin);
    //list_dir(rootin);

err:
    return;
}

struct bbuf *bbuf_head;
int nr_bbufs = 1000;

struct bbuf *bbuf_alloc(){
    struct bbuf *bbuf;

    bbuf =kmalloc(sizeof(struct bbuf));
    memset(bbuf, 0, sizeof(struct bbuf));
    bbuf->prev = NULL;
    bbuf->next = bbuf_head;
    bbuf_head->prev = bbuf;
    bbuf_head = bbuf;

    return bbuf;
}

void bbuf_init(){
    int i;

    bbuf_head =kmalloc(sizeof(struct bbuf));
    memset(bbuf_head, 0, sizeof(struct bbuf));

    for(i = 0; i < nr_bbufs; i++){
        bbuf_alloc();
    }
}

struct bbuf *get_bbuf_cache(int block){
    struct bbuf *bbuf;

    for(bbuf = bbuf_head; bbuf; bbuf = bbuf->next){
        if(bbuf->block == block)
            return bbuf;
    }

    return NULL;
}

struct bbuf *get_free_bbuf(int block){
    struct bbuf *bbuf;

    for(bbuf = bbuf_head; bbuf; bbuf = bbuf->next){
        if(!bbuf->ref)
            break;
    }

    if(!bbuf){
        bbuf = bbuf_alloc();
        //TODO
        //printk("no free bbuf, alloc %x\n", bbuf);
    }

    return bbuf;
}

struct bbuf *bread(int block){
    struct bbuf *bbuf;

    //printk("bread %d\n", block);

    bbuf = get_bbuf_cache(block);
    if(bbuf)
        return bbuf;

    bbuf = get_free_bbuf(block);
    bbuf->ref++;

    ide_read(block, bbuf);
    bbuf->block = block;

    return bbuf;
}
