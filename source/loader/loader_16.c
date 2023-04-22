//loader部分工作在16位实模式下,使gccc将代码编译为16位代码
__asm__ (".code16gcc");

#include"loader.h"

boot_info_t boot_info;


static void show_msg(const char * msg)
{
    char c;

    while((c = *msg++) != '\0')
    {
        __asm__ __volatile__
        (
	        "mov $0xe, %%ah\n\t"
	        "mov %[ch], %%al\n\t"
	        "int $0x10"::[ch]"r"(c)
        );
    }
}


static void detect_memory(void)
{
    uint32_t contID=0;
    uint32_t signature, bytes;
    SMAP_entry_t smap_entry;

    show_msg("detecting memory:");

    boot_info.ram_region_count = 0;
    for(int i=0;i<BOOT_RAM_REGION_MAX;i++)
    {

        SMAP_entry_t *entry=&smap_entry;

        __asm__ __volatile__
        (
            "int  $0x15"
			: "=a"(signature), "=c"(bytes), "=b"(contID)
			: "a"(0xE820), "b"(contID), "c"(24), "d"(0x534D4150), "D"(entry)
        );

        if(signature != 0x534D4150)
        {
            show_msg("failed\r\n");
            return;
        }

        if (bytes > 20 && (entry->ACPI & 0x0001) == 0){
			continue;
		}

        if(entry->Type==1)
        {
            boot_info.ram_region_cfg[boot_info.ram_region_count].start = entry->BaseL;
            boot_info.ram_region_cfg[boot_info.ram_region_count].end = entry->LengthL;
            boot_info.ram_region_count++;
        }

        if(contID == 0)
        {
            break;
        }

    }

    show_msg("detect complete!\r\n");
}

// GDT表。临时用，后面内容会替换成自己的
uint16_t gdt_table[][4] = {
    {0, 0, 0, 0},
    {0xFFFF, 0x0000, 0x9A00, 0x00CF},
    {0xFFFF, 0x0000, 0x9200, 0x00CF},
};

static void enter_protect_mode(void)
{
    cli();

    //加载A20地址线
    uint8_t v=inb(0x92);
    outb(0x92, v | 0x2);

    // 加载GDT。由于中断已经关掉，IDT不需要加载
    lgdt((uint32_t)gdt_table, sizeof(gdt_table));

    //cr0置一
    uint32_t cr0 = read_cr0();
    write_cr0(cr0 | (1 << 0)); 

    
    //长跳转到32位入口处，清空流水线中的16位指令
    far_jump(8, (uint32_t)protect_mode_entry);
}
 

void loader_entry(void)
{
    show_msg("......loading......\n\r"); 
    detect_memory();

    enter_protect_mode();
}