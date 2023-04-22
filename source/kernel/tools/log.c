#include"tools/log.h"
#include"comm/cpu_instr.h"
#include"tools/klib.h"
#include"stdarg.h"

/************
串行接口
************/

//接口地址
#define COM1_PORT 0x3F8

//初始化
void log_init(void)
{
    //第一个寄存器写00关串行接口的中断
    outb(COM1_PORT + 1, 0x00);
    //第三个寄存器设置发送速度
    outb(COM1_PORT + 3, 0x80);
    //
    outb(COM1_PORT + 0, 0x3);
    outb(COM1_PORT + 1, 0x00);
    outb(COM1_PORT + 3, 0x03);
    outb(COM1_PORT + 2, 0xc7);
    outb(COM1_PORT + 4, 0x0F);
}

void log_printf(const char * fmt, ...)
{
    char str_buf[128];
    va_list args;

    //缓冲区清零
    kernel_memset(str_buf, 0, sizeof(str_buf));
    //可变参数存储到args中
    va_start(args, fmt);

    kernel_vsprintf(str_buf, fmt, args);

    va_end(args);

    const char * p = str_buf;
    while(*p != '\0')
    {
        //端口忙检测
        while((inb(COM1_PORT + 5) & (1 << 6)) == 0);
        outb(COM1_PORT, *p++);
    }

    //换行，要加一个换行和一个回车,在串行接口中
    //不加换行的话会在下一行显示，但是是从下一行中欧给对齐上一行文字的结尾处显示而不是在下一行开头显示，\r改列位置
    //不加回车会把文字放在同一行显示，其中第一段文字小于第二段的部分覆盖，大于第二段的部分显示，\n改行位置
    outb(COM1_PORT, '\r');
    outb(COM1_PORT, '\n');
}