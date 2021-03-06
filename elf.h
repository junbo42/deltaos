#define ELF_MAGIC 0x464c457f

struct elfh{
    uint32_t    e_magic;
    uint8_t     e_ident[12];   
    uint16_t    e_type;
    uint16_t    e_machine;
    uint32_t    e_version;
    uint32_t    e_entry;
    uint32_t    e_phoff;
    uint32_t    e_shoff;
    uint32_t    e_flags;
    uint16_t    e_ehsize;
    uint16_t    e_phentsize;
    uint16_t    e_phnum;
    uint16_t    e_shentsize;
    uint16_t    e_shnum;
    uint16_t    e_shstrndx;
};

struct elfsh{
	uint32_t	sh_name;
	uint32_t	sh_type;
	uint32_t	sh_flags;
	uint32_t	sh_addr;
	uint32_t	sh_offset;
	uint32_t	sh_size;
	uint32_t	sh_link;
	uint32_t	sh_info;
	uint32_t	sh_addralign;
	uint32_t	sh_entsize;
};

struct elfph{
	uint32_t		p_type;
	uint32_t		p_offset;
	uint32_t		p_vaddr;
	uint32_t		p_paddr;
	uint32_t		p_filesz;
	uint32_t		p_memsz;
	uint32_t		p_flags;
	uint32_t		p_align;
};
