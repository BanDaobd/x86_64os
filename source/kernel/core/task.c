#include"core/task.h"
#include"tools/klib.h"
#include"cpu/cpu.h"
#include"os_cfg.h"
#include"tools/log.h"

//第一次切换到需要初始化
static int tss_init(task_t *task, uint32_t entry, uint32_t esp)
{

    int tss_sel = gdt_alloc_desc();
    if(tss_sel < 0)
    {
        log_printf("Alloc tss failed!.\n");
        return -1;
    }

    segment_desc_set(tss_sel, (uint32_t)&task->tss, sizeof(tss_t),
    SEG_P_PRESENT | SEG_DPL0 | SEG_TYPE_TSS);

    //
    kernel_memset(&task->tss, 0, sizeof(tss_t));
    task->tss.eip = entry;
    //要先写入esp0，因为特权级0寄存器会从特定寄存器取栈地址
    task->tss.esp = task->tss.esp0 = esp;
    task->tss.ss = task->tss.ss0 = KERNEL_SELECTOR_DS;
    task->tss.es = task->tss.ds = task->tss.fs = task->tss.gs = KERNEL_SELECTOR_DS;
    task->tss.cs = KERNEL_SELECTOR_CS;
    task->tss.eflags = EFLAGS_DEFAULT | EFLAGS_IF;

    task->tss_sel = tss_sel;

    return 0;
}

int task_init(task_t *task, uint32_t entry, uint32_t esp)
{
    ASSERT(task != (task_t *)0);

    //第一次切换时没有上一个任务的栈信息被push，此时pop指令会出错
    //所以我们要进行初始化
    //由于pop出来的几个寄存器的值可以随意设置值，所以我们可以对其进行初始化
    uint32_t * p_esp = (uint32_t *)esp;
    if(p_esp)
    {
        //第一次切换时栈的返回地址设置为另一个任务的入口地址，这样ret返回时就会跳转到另一个任务执行
        *(--p_esp) = entry;
        *(--p_esp) = 0;
        *(--p_esp) = 0;
        *(--p_esp) = 0;
        *(--p_esp) = 0;
        task->stack = p_esp;
    }
    //tss_init(task, entry, esp);

    return 0;
}


//参数是tss中的*stack，也就是栈指针，栈指针本来是存在tss中
//我们把tss中的*stack传进来就把栈指针变成函数参数压入栈，然后在汇编中取栈指针
//这里为什么是二重指针
void simple_switch(uint32_t **from, uint32_t *to);



void task_switch_from_to(task_t *from, task_t *to)
{
    //在cpu.c中，tss硬件自动传递
    //switch_to_tss(to->tss_sel);

    //手动传栈指针
    simple_switch(&from->stack, to->stack);
}

