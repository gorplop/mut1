#define _GNU_SOURCE
/*
 * mut1.c - Mutate an aarch64 program (reorder instructions)
 * This is still a work in progress
 * This version detects LDR instructions and adjusts the offsets
 * pointing to the constant pool. It also does not reorder the constant pool
 * words
 */

#include <elf.h>
#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "unilist.h"
#include "mut1.h"


#define BC_MASK     0xff000000
#define BC_OPCODE   0x54000000
#define BC_IMM_MASK 0x00fffff0
#define B_MASK      0xfc000000
#define B_OPCODE    0x14000000
#define BL_OPCODE   0x94000000
#define LDR_MASK     0xbf000000
#define LDR_OPCODE   0x18000000
#define LDR_IMM_MASK 0x00ffffe0

void db_print(datablock_t *x){
  printf("datablock_t: %x+%04x\n", x->addr, x->size);
}

void readhead(int fd, size_t len){
  int position = lseek(fd, 0, SEEK_CUR);
  lseek(fd, 0, SEEK_SET);
  uint8_t buf[len];
  read(fd, &buf, len);
  hexdump(buf, len);
  lseek(fd, position, SEEK_SET);
  
}

void hexdump(uint8_t *data, size_t len){
  size_t i = 0;
  putchar('\n');
  printf("addr   +0 +1 +2 +3 +4 +5 +6 +7 +8 +9 +a +b +c +d +e +f 0123456789abcdef\n"); 
  for(i = 0; i < len;){
    if(i % 16 == 0) printf("0x%04x ", i);
    printf("%02x ", data[i]);
    i++;
    if(i % 16 == 0){
      for(size_t j = 16; j >= 1; j--){
	putchar((data[i-j] < 32 || data[i-j] > 126) ? '.' : data[i-j]);
      }
      putchar('\n');
    }
  }
  for(size_t j = 0; j < 16-i%16; j++) printf("   ");
  for(size_t j = i%16; j >= 1; j--){
    putchar((data[i-j] < 32 || data[i-j] > 127) ? '.' : data[i-j]);
  }
  
  putchar('\n');

}

void mutate_xrefs(uint8_t *elf){
  Elf64_Ehdr *ehdr = (Elf64_Ehdr*) elf;
  Elf64_Shdr *section_headers = (Elf64_Shdr*) &elf[ehdr->e_shoff];
  int text_section = find_section_by_name(elf, ".text");
  Elf64_Shdr *text = &section_headers[text_section];

  
  DEBUG(".text: %d 0x%08x+0x%08x -> 0x%08x\n", text_section,
	section_headers[text_section].sh_offset,
	section_headers[text_section].sh_size,
	section_headers[text_section].sh_addr);

  unilist_t *al = unilist_create(sizeof(datablock_t)); //data list
  uint32_t* code = ((uint32_t*) elf); //elf pointer, 4-byte sized steps
  bool second = false;
  bool swap = true;
  int swapcnt = 0;
  for(size_t i = text->sh_offset /4;
      i < (text->sh_size + text->sh_offset) /4 ; i++) {

    uint32_t addr = i*4 - text->sh_offset + text->sh_addr; //load addr
    uint32_t insn = code[i];
    int offset = 0;

    DEBUG("%s %08x: %08x: ", second ? "odd " : "even", addr, insn);


    //Decide if this pair is swapped
    if(!second) swap = rand() % 5;
    if(!swap) goto end;

    // Detect constant pools used by LDR
    if((insn & LDR_MASK) == LDR_OPCODE) {
      offset = (insn & LDR_IMM_MASK) >> 5;
      DEBUG( "ldr imm19: ofs=%08x ", offset);
      datablock_t db = { (offset << 2) + addr, 8 }; //for now assume everything refers to a qword
      unilist_append(al, &db);
      if(swap){
	offset += second ? 1 : -1;
	DEBUG("new ofs=%08x", offset);
	insn &= ~LDR_IMM_MASK;
	insn |= (offset << 5) & LDR_IMM_MASK;
	DEBUG(" in=%08x", insn);
      }
    }
    
    // Detect and adjust bl insns
    if((insn & B_MASK) == BL_OPCODE || (insn & B_MASK) == B_OPCODE) {
      offset = (insn & (~B_MASK)) << 2;
      DEBUG(" b/bl imm26: ofs=%08x ", offset);
      if(swap){ // patch the instruction
	offset += second ? 4 : -4;
	DEBUG("new ofs=%08x", offset);
	insn &= B_MASK;
	insn |= offset >>2;
	DEBUG(" in=%08x", insn);
      }
    }
    
    if((insn & BC_MASK) == BC_OPCODE) {
      int cond = insn & 0x0f;
      offset = (insn >> 5) & ((1<<20)-1);
      DEBUG(" b.%02x ofs=%08x", cond, offset);
      if(swap){
	offset += second ? 1 : -1;
	DEBUG(" nofs=%08x", offset);
	insn &= ~BC_IMM_MASK;
	insn |= (offset) << 5;
	DEBUG(" in=%08x", insn);

      }
    }

    code[i] = insn;
    
    // if in constant pool, do not swap
    // manually iterate over the datablock list
    unilist_el *el = unilist_first(al);
    while(el != NULL){
      datablock_t *db = el->element;
      if(addr >= db->addr && addr < db->addr + db->size){
	DEBUG(" ### data block of size 0x%x ###", db->size);
	i += (db->size >> 2) -1;
	goto end;
      }
      el = el->next;
    }

    //No LDR detection again, these are already in the address list
    
    if(second){
      uint32_t t = code[i];
      code[i] = code[i-1];
      code[i-1] = t;
      swapcnt++;
      DEBUG(" ^^  swap     ^^"); // swap the second
    }
    
  end:
    second = !second;
    DEBUG("\n");
  }
  
  printf("[i] found %d constant pools in .text:\n", al->count);
  unilist_iter(al, (void (*)(void *)) db_print);

  
}


/** Get the section name from the elf section number */
char *sectionname(uint8_t *elf, int section){
  Elf64_Ehdr *ehdr = (Elf64_Ehdr*) elf;
  Elf64_Shdr *section_headers = (Elf64_Shdr*) &elf[ehdr->e_shoff];
  return &elf[section_headers[ehdr->e_shstrndx].sh_offset + section_headers[section].sh_name];
}

/** Find the text section in the ELF file and return it's number */
int find_section_by_type(uint8_t *elf, uint32_t type, uint32_t stoff){
  Elf64_Ehdr *ehdr = (Elf64_Ehdr*) elf;
  Elf64_Shdr *section_headers = (Elf64_Shdr*) &elf[ehdr->e_shoff];
  int found = -1;
  for(int i = stoff; i < ehdr->e_shnum; i++){
    if(section_headers[i].sh_type == type){
      found = i;
      break;
    }
    //DEBUG("\n");
  }
  return found;
}

int find_section_by_name(uint8_t *elf, char *name){
  Elf64_Ehdr *ehdr = (Elf64_Ehdr*) elf;
  int found = -1;
  for(int i = 0; i < ehdr->e_shnum; i++){
    if(strcmp(sectionname(elf, i), name) == 0) {
      found = i;
    }
  }
  return found;
}

void list_sections(uint8_t *elf){
  Elf64_Ehdr *ehdr = (Elf64_Ehdr*) elf;
  Elf64_Shdr *section_headers = (Elf64_Shdr*) &elf[ehdr->e_shoff];
  DEBUG("SHT_SYMTAB = %x\n", SHT_SYMTAB);

  for(int i = 0; i < ehdr->e_shnum; i++){
    DEBUG("Section %d, \t", i);
    DEBUG("type %x ,\t", section_headers[i].sh_type);
    DEBUG("name %s, \t", sectionname(elf, i));
    DEBUG("offset 0x%x \t", section_headers[i].sh_offset);
    DEBUG("sz 0x%x \t", section_headers[i].sh_size);
    DEBUG("la 0x%x \t", section_headers[i].sh_addr);
    DEBUG("\n");
  }
}
