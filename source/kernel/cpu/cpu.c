#include"cpu/cpu.h"
#include"cpu/irq.h"
#include"os_cfg.h"
#include"comm/cpu_instr.h"

/*********************
GDT表
大小256 * 4bytes = 1kb
*********************/
static segment_desc_t gdt_table[GDT_TABLE_SIZE];

void segment_desc_set(int selector, uint32_t base, uint32_t limit, uint16_t attr)
{
    segment_desc_t * desc = gdt_table + (selector >> 3);


    if(limit > 0xFFFFF)
    {
        //传入的起始为0x0，最大值为0xFFFFFFFF，大小为0x100000000 4GB
        //G为决定limit的单位，为1是4kb
        //limit本是16+4位，最多表示到0xFFFFF，无法表示32位，所以G置一
        attr | 0x8000;
        //设置为以4kb为单位
        limit /= 0x1000;
    }

    desc->limit15_0 = limit & 0xFFFF;
    desc->base15_0 = base & 0xFFFF;
    desc->base23_16 = (base >> 16) & 0xFF;
    //16~23 | 8~16
    desc->attr = attr | (((limit >> 16) & 0xF) << 8);
    desc->base31_24 = (base >> 24) & 0xFF;
}

void gate_desc_set(gate_desc_t * desc ,uint16_t selector, uint32_t offset, uint16_t attr)
{
    desc->offset15_0 = offset & 0xFFFF;
    desc->selector = selector;
    desc->attr = attr;
    desc->offset31_16 = (offset >> 16) & 0xFFFF;
}

void init_gdt(void)
{
    for(int i=0; i< GDT_TABLE_SIZE; i++)
        segment_desc_set((i << 3), 0, 0, 0);

    segment_desc_set(KERNEL_SELECTOR_CS, 0, 0xFFFFFFFF, SEG_P_PRESENT | SEG_DPL0 | SEG_S_NORMAL | SEG_TYPE_CODE
     | SEG_TYPE_RW | SEG_D | SEG_G);

    segment_desc_set(KERNEL_SELECTOR_DS, 0, 0xFFFFFFFF, SEG_P_PRESENT | SEG_DPL0 | SEG_S_NORMAL | SEG_TYPE_DATA
     | SEG_TYPE_RW | SEG_D | SEG_G);

     lgdt((uint32_t)gdt_table, sizeof(gdt_table));
}


void cpu_init(void)
{
    init_gdt();
}

int gdt_alloc_desc()
{
    for(int i = 1; i < GDT_TABLE_SIZE; i++)
    {
        segment_desc_t *desc =gdt_table + i;
        if(desc->attr == 0)
            return i * sizeof(segment_desc_t);
    }

    return -1;
}

void switch_to_tss(int tss_sel)
{
    far_jump(tss_sel, 0);
}