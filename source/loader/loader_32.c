//真正进入32位c代码，可以读取4GB地址
#include"loader.h"
#include"comm/elf.h"

static void read_disk(uint32_t sector,uint32_t sector_count, uint8_t *buf)
{
    outb(0x1F6, 0xE0);
    outb(0x1F2, (uint8_t)(sector_count >> 8));
    outb(0x1F3, (uint8_t)(sector >> 24));
    outb(0x1F4, 0);
    outb(0x1F5, 0);

    outb(0x1F2, (uint8_t)sector_count); 
    outb(0x1F3, (uint8_t)sector); 
    outb(0x1F4, (uint8_t)(sector >> 8)); 
    outb(0x1F5, (uint8_t)(sector >> 16)); 

    outb(0x1F7, 0x24);

    //每次从磁盘读取16位的数据
    //这里读取可能会发生一个错误没处理
    uint16_t *data_buf = (uint16_t *)buf;
    //读取磁盘前判断磁盘忙
    while(sector_count--)
    {
        //没就绪原地等待
        while((inb(0x1F7)  & 0x88) != 0x8)
        {

        }

        //读扇区，一个扇区512字节
        for(int i = 0;i<SECTOR_SIZE / 2;i++)
        {
            *data_buf++ = inw(0x1F0);
        }
    }
}

//用于解析并加载elf文件到内存位置处
static uint32_t reload_elf_file(uint8_t * file_buffer)
{
    ELF32_Ehdr *elf_hdr =(ELF32_Ehdr *)file_buffer;
    //粗略检查文件头是否合法，检查e_ident[]标志符
    if (elf_hdr->e_ident[0] != 0x7F || elf_hdr->e_ident[1] != 'E' 
    || elf_hdr->e_ident[2] != 'L' || elf_hdr->e_ident[3] != 'F')
        return 0;
    
    for(int i=0;i < elf_hdr->e_phnum; i++)
    {
        ELF32_Phdr *phdr = (ELF32_Phdr *)(file_buffer + elf_hdr->e_phoff)+i;

        if(phdr->p_type != PT_LOAD)
            continue;

        //从内存位置offset拷到内存位置addr, 拷filesz字节
        uint8_t *src = file_buffer + phdr->p_offset;
        uint8_t *dest = (uint8_t *)phdr->p_paddr;
        for(int j = 0; j < phdr->p_filesz; j++)
            *dest++ = *src++;

        //填0
        dest = (uint8_t *)(phdr->p_paddr + phdr->p_filesz);
        int n=(phdr->p_memsz - phdr->p_filesz);
        for(int j=0 ;j < n; j++)
            *dest++ =0;
    }

    //返回程序入口地址
    return elf_hdr->e_entry;

}

static void die(int error_code)
{
    for(;;)
    {

    }
}
void load_kernel(void)
{
    //从磁盘读取内核代码
    //内核放在loader后面，100扇区起始，大小500扇区250kb
    read_disk(100, 500, (uint8_t *)SYS_KERNEL_LOAD_ADDR);

    //把elf文件加载到0x100000后还要从这个位置解析并加载到0x10000，
    //因为elf文件需要解析出各个段地址（段地址在脚本中聚合了），并且要得到程序代码入口地址（入口地址不是文件的第一个字节）
    uint32_t kernel_entry = reload_elf_file((uint8_t *)SYS_KERNEL_LOAD_ADDR);

    if(kernel_entry == 0)
        die(-1);

    //SYS_KERNEL_LOAD_ADD 是start的位置
    //不加载二进制文件而是加载elf文件，降低文件大小并且能够得知文件数据信息，方便做读写权限，从kernel_entry进入
    //kernel是解析出来的内核程序入口地址
    ((void (*)(boot_info_t *))kernel_entry)(&boot_info);
}