#ifndef _EXT2_H
#define _EXT2_H

// https://www.nongnu.org/ext2-doc/ext2.html

struct super_block{            
    uint32_t  s_inodes_count;
    uint32_t  s_blocks_count;
    uint32_t  s_r_blocks_count;
    uint32_t  s_free_blocks_count;
    uint32_t  s_free_inodes_count;
    uint32_t  s_first_data_block;
    uint32_t  s_log_block_size;
    uint32_t  s_log_frag_size;
    uint32_t  s_blocks_per_group;
    uint32_t  s_frags_per_group;
    uint32_t  s_inodes_per_group;
    uint32_t  s_mtime;
    uint32_t  s_wtime;
    uint16_t  s_mnt_count;
    uint16_t  s_max_mnt_count;
    uint16_t  s_magic;
    uint16_t  s_state;
    uint16_t  s_errors;
    uint16_t  s_minor_rev_level;
    uint32_t  s_lastcheck;
    uint32_t  s_checkinterval;
    uint32_t  s_creator_os;
    uint32_t  s_rev_level;
    uint16_t  s_def_resuid;
    uint16_t  s_def_resgid;
    uint8_t unused[940];
} __attribute__((packed));

struct group_desc{
    uint32_t    bg_block_bitmap;
    uint32_t    bg_inode_bitmap;
    uint32_t    bg_inode_table;
    uint16_t    bg_free_blocks_count;
    uint16_t    bg_free_inodes_count;
    uint16_t    bg_used_dirs_count;
    uint8_t     pad[14];
} __attribute__((packed));

struct ext2_inode{
    uint16_t  i_mode;
    uint16_t  i_uid;
    uint32_t  i_size;
    uint32_t  i_atime;
    uint32_t  i_ctime;
    uint32_t  i_mtime;
    uint32_t  i_dtime;
    uint16_t  i_gid;
    uint16_t  i_links_count;
    uint32_t  i_blocks;
    uint32_t  i_flags;
    uint32_t  i_reserved1;
    uint32_t  i_block[15];
    uint32_t  i_version;
    uint32_t  i_file_acl;
    uint32_t  i_dir_acl;
    uint32_t  i_faddr;
    uint8_t   i_frag;
    uint8_t   i_fsize;
    uint16_t  i_pad1;
    uint32_t  i_reserved2[2];
} __attribute__((packed));

#define EXT2_FT_UNKNOWN 0
#define EXT2_FT_REG_FILE 1
#define EXT2_FT_DIR	    2
#define EXT2_FT_CHRDEV  3
#define EXT2_FT_BLKDEV  4
#define EXT2_FT_FIFO    5
#define EXT2_FT_SOCK    6
#define EXT2_FT_SYMLINK	7

struct ext2_dir_entry{
    uint32_t inode;
    uint16_t rec_len;
    uint8_t  name_len;
    uint8_t  file_type;     // require file_type feature
    char     name[255];
};

struct inode{
    uint32_t i_ino;
    uint32_t i_off;
    uint32_t i_size;
    uint32_t i_mode;
    struct inode_ops *i_ops;
    struct ext2_inode ext2_inode;
    char name[255];
};

struct inode_ops{
    int (*read)(struct inode *, char *, size_t);
};

struct bbuf{
    int ref;
    int block;
    int valid;
    struct bbuf *next;
    struct bbuf *prev;
    char data[1024];
};

struct dirent{
    uint32_t inode;
    char name[255];
};       

struct ext2_inode *ext2_lookup(char *path);
char *get_content(struct ext2_inode *in);
void read_inode(int i, struct inode *inode);
void read_ext2_inode(int i, struct ext2_inode *einode);

struct inode *namei(const char *);

struct bbuf *bread(int block);

#endif
