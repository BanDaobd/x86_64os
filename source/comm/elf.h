#ifndef EIF_H_
#define ELF_H_

//elf文件解析器
#include"types.h"

typedef uint32_t ELF32_Addr;
typedef uint16_t ELF32_Half;
typedef uint32_t ELF32_Off;
typedef uint32_t ELF32_Word;

#define EI_NIDENT   16

//禁止对齐
#pragma pack(1)
//elf header
typedef struct
{
    char e_ident[EI_NIDENT];
    ELF32_Half e_type;
    ELF32_Half e_machine;
    ELF32_Word e_version;
    ELF32_Addr e_entry;
    ELF32_Off e_phoff;
    ELF32_Off e_shoff;
    ELF32_Word e_flags;
    ELF32_Half e_ensize;
    ELF32_Half e_phentsize;
    ELF32_Half e_phnum;
    ELF32_Half e_shentsize;
    ELF32_Half e_shum;
    ELF32_Half e_shstrndx;
}ELF32_Ehdr;

#define PT_LOAD     1
//program header
typedef struct 
{
    ELF32_Word p_type;
    ELF32_Off p_offset;
    ELF32_Addr p_vaddr;
    ELF32_Addr p_paddr;
    ELF32_Word p_filesz;
    ELF32_Word p_memsz;
    ELF32_Word p_flags;
    ELF32_Word p_align;
}ELF32_Phdr;

#pragma pack()

#endif