#ifndef CPU_INTSTR_H
#define CPU_INTSTR_H
#include"types.h"
/*
即使加了inline，我们对是否内联只有建议权没有决定权，
如果编译器决定不内联，函数会被include到多个头文件中，
因为#ifndef只能防止单文件重复定义，
而函数又默认是外部链接性(其他文件可见),
所以加上static变为内部链接性就能防止多定义
*/

static inline void cli(void)
{
    __asm__ __volatile__("cli");

}

static inline void sti(void)
{
    __asm__ __volatile__("sti");

}

static inline uint8_t inb(uint16_t port)
{
    uint8_t rv;

    //inb al, dx
    __asm__ __volatile__("inb %[p], %[v]":[v]"=a"(rv) : [p]"d"(port));
    return rv;
}

static inline uint16_t inw(uint16_t port)
{
    uint16_t rv;

    //inb用字节去读，in用16位去读
    //inb ax, dx
    __asm__ __volatile__("in %[p], %[v]":[v]"=a"(rv) : [p]"d"(port));
    return rv;
}

static inline void outb(uint16_t port, uint8_t data)
{

    //outb al, dx
    __asm__ __volatile__("outb %[v], %[p]"::[p]"d"(port), [v]"a"(data));
}



static inline void lgdt (uint32_t start, uint32_t size)
{
    //GDTR寄存器
    struct {
        uint16_t limit;
        uint16_t start15_0;
        uint16_t start31_16;
    }gdt;

    gdt.start31_16 = start >> 16;
    gdt.start15_0 = start & 0xFFFF;
    gdt.limit = size - 1;
    
    //lgdt
    __asm__ __volatile__("lgdt %[g]"::[g]"m"(gdt));    
}

static inline void lidt (uint32_t start, uint32_t size)
{
    //IDTR寄存器
    struct {
        uint16_t limit;
        uint16_t start15_0;
        uint16_t start31_16;
    }idt;

    idt.start31_16 = start >> 16;
    idt.start15_0 = start & 0xFFFF;
    idt.limit = size - 1;
    
    //lidt
    __asm__ __volatile__("lidt %[g]"::[g]"m"(idt));    
}

static inline uint16_t read_cr0(void)
{
    uint32_t cr0;

    __asm__ __volatile__("mov %%cr0, %[v]":[v]"=r"(cr0));
    return cr0;
}

static inline void write_cr0(uint32_t v)
{
    __asm__ __volatile__("mov %[v], %%cr0"::[v]"r"(v));
}

static inline void far_jump(uint32_t selector, uint32_t offset)
{
    uint32_t addr[]={offset,selector};

    __asm__ __volatile__("ljmpl *(%[a])"::[a]"r"(addr));
}

static inline void hlt(void)
{
    __asm__ __volatile__("hlt");
}

static inline void write_tr(uint16_t tss_sel)
{
    __asm__ __volatile("ltr %%ax"::"a"(tss_sel));
}
#endif