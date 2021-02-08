
void gdt_init(void);
void set_gdt(uint8_t index, uint32_t base, uint32_t limit, uint8_t access, uint8_t flags);
