#ifndef _MUT1_H
#define _MUT1_H


#ifdef WITH_DEBUG
#define DEBUG(...) fprintf(stderr, __VA_ARGS__)
#else
#define DEBUG(...) 
#endif

typedef struct {
  uint32_t addr;
  uint32_t size;
} datablock_t;

void db_print(datablock_t *x);
void readhead(int fd, size_t len);
void hexdump(uint8_t *data, size_t len);
void mutate_xrefs(uint8_t *elf);
char *sectionname(uint8_t *elf, int section);
int find_section_by_name(uint8_t *elf, char *name);
int find_section_by_type(uint8_t *elf, uint32_t type, uint32_t stoff);
void list_sections(uint8_t *elf);




#endif // _MUT1_H
