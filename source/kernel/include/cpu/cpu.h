#ifndef CPU_H_
#define CPU_H_

#include"comm/types.h"

/*************

GDT表的宏定义

*************/

#define SEG_G (1 << 15) //控制limit的单位 单位4kb 1
#define SEG_D (1 << 14) //控制代码段和栈段是16位和32位， 32位 1
//L
//AVL保留位
//limit4位
#define SEG_P_PRESENT   (1 << 7)    //段描述符存在标志，存在 1

#define SEG_DPL0 (0 << 5)  //段描述符指向的段的特权级 4位
#define SEG_DPL3 (3 << 5)  //段描述符指向的段的特权级

#define SEG_S_SYSTEM    (0 << 4)    //决定代码是系统段 0,中断门TSS等
#define SEG_S_NORMAL    (1 << 4)    //还是代码段数据段 1

//TYPE 4位,除了以下的设置还能设置是否可扩展和一致性
#define SEG_TYPE_CODE   (1 << 3)    //第三位决定是代码段 1
#define SEG_TYPE_DATA   (0 << 3)    //还是数据段 0
#define SEG_TYPE_TSS    (9 << 0)    //根据手册的设置TSS的TYPE值

#define SEG_TYPE_RW     (1 << 1)    //对于数据段为1可读写，对于代码段为1是可读取执行为0是只能执行


/********************
    IDT表的宏定义
********************/

#define GATE_P_PRRESENT (1 << 15)   //段存在标志
#define GATE_DPL0       (0 << 13)   //权限位
#define GATE_DPL3       (3 << 13)
#define GATE_TYPE_INT   (0xE << 8)  //IDT门类型

/********************
    TSS的宏定义
********************/
#define EFLAGS_DEFAULT  (1 << 1)
#define EFLAGS_IF       (1 << 9)



/**********************************
GDT表/IDT表中断门/TSS结构的表项定义
***********************************/

#pragma pack(1)
typedef struct _segment_desc_t
{
    uint16_t limit15_0;
    uint16_t base15_0;
    uint8_t base23_16;
    uint16_t attr;
    uint8_t base31_24;
}segment_desc_t;

typedef struct _gate_desc_t
{
    uint16_t offset15_0;
    uint16_t selector;
    uint16_t attr;
    uint16_t offset31_16;
}gate_desc_t;

typedef struct _tss_t
{
    uint32_t pre_link;
    uint32_t esp0, ss0, esp1, ss1, sep2, ss2;
    uint32_t cr3;
    uint32_t eip, eflags, eax, ecx, edx, ebx, esp, ebp, esi, edi;
    uint32_t es, cs, ss, ds, fs, gs;
    uint32_t ldt;
    uint32_t iomap;
    //ssp
}tss_t;

#pragma pack()


//表项设置
void segment_desc_set(int selector, uint32_t base, uint32_t limit, uint16_t attr);

//GDT表的选择子/.../.../...
void gate_desc_set(gate_desc_t * desc ,uint16_t selector, uint32_t offset, uint16_t attr);


void init_gdt(void);

void cpu_init(void);

//在GDT表中为TSS选择子找空闲位置,返回选择子
int gdt_alloc_desc();

void switch_to_tss(int tss_sel);
#endif