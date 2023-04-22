#ifndef TSAK_H_
#define TASK_H_
#include"comm/types.h"
#include"cpu/cpu.h"

//tss结构为何包装在另一个结构体中
typedef struct _task_t
{
    //保存esp
    uint32_t *stack;

    tss_t tss;

    //保存tss选择子,方便切换
    int tss_sel;
}task_t;

//？？？/入口地址/栈顶指针
int task_init(task_t *task, uint32_t entry, uint32_t esp);

//任务切换
void task_switch_from_to(task_t *from, task_t *to);

#endif