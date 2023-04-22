#include"init.h"
#include"comm/boot_info.h"
#include"comm/cpu_instr.h"
#include"cpu/cpu.h"
#include"cpu/irq.h"
#include"dev/time.h"
#include"tools/log.h"
#include"os_cfg.h"
#include"tools/klib.h"
#include"core/task.h"

void kernel_init(boot_info_t * boot_info)
{
    //断定它成立，如果不成立编译器（？）会做错误处理
    ASSERT(boot_info->ram_region_count != 0);
    
    cpu_init();
    //日志初始化放在前面，后面调用的函数就能使用日志打印错误信息
    log_init();
    irq_init();
    time_init();
}

//为两个任务分配tss
static task_t first_task;
static task_t init_task;

//为init_task_entry设置自己的栈，
//为什么能直接设置，因为栈传到tss初始化中，硬件会分配这个栈给它
static uint32_t init_task_stack[1024];
void init_task_entry(void)
{
    int count = 0;
    for(;;)
    {
        log_printf("----------------------------------\r\ninit task: %d\n", count++);
        task_switch_from_to(&init_task, &first_task);
    }
}

void init_main(void)
{
    log_printf("Hello BanDaobd, your Kernel is running...");
    log_printf("Version: %s", DOS_VERSION);
    log_printf("%d %d %x %c", 114514, -1919180, 0x114514, 'a');
    //irq_enable_global();   

    task_init(&init_task, (uint32_t)&init_task_entry, (uint32_t)&init_task_stack[1024]);
    //本函数的程序入口地址和栈指针没必要传，后面会解释
    task_init(&first_task, 0, 0);
    write_tr(first_task.tss_sel);
    int count = 0;
    for(;;)
    {
        log_printf("----------------------------------\r\ninit main: %d\n", count++);
        task_switch_from_to(&first_task, &init_task);
    }
}