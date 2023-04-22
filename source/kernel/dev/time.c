#include"dev/time.h"
#include"comm/types.h"
#include"comm/cpu_instr.h"
#include"os_cfg.h"
#include"cpu/irq.h"

//计数器，根据时钟频率计数得知系统运行时间
static uint32_t sys_tick;

void do_handler_time (exception_frame_t * frame)
{
    //退出中断需要通知8259，否则只会进入一次中断
    sys_tick++;

    pic_send_eoi(IRQ0_TIMER);
}

//硬件初始化，还未选择定时器
static void init_pit(void)
{
    /*******************************************************************************************
    注意：中断越频繁计时越准，但是定时中断运行时间对系统运行时间的占比也就越大
    计数值计算，PIT_OSC_FREQ每秒钟时钟节拍数，那么每10ms有PIT_OSC_FREQ / 100 次时钟信号，
    在定时器中每 1000 / PIC_OSC_FREQ 毫秒这个数自减一次
    所以想让定时器10ms中断一次，需要把计数值设置为PIT_OSC_FREQ / 100,
    这样1000 / PIC_OSC_FREQ * PIT_OSC_FREQ / 100 = 10 ms，每10毫秒减到0.

    但是这里除法计算有精度损失，有没有什么更加精确的计算方法，（可以用在求职上）
    我们在这里使用一种提高精度的方法
    *******************************************************************************************/
    uint32_t reload_count =  PIT_OSC_FREQ / (1000 / OS_TICK_MS );

    //选择定时器0并设置它的控制命令
    //简单的设置为每隔一段时间中断计数mode3，没有设置其它mode，比如在什么条件下开始计数
    //BCD=0工作在16位模式下
    outb(PIT_COMMAND_MODE_PORT, PIT_CHANNEL0 | PIT_LOAD_LOHI | PIT_MODE3);
    //写定时器计数值，每1 / 1193182秒自减一次，减到0会触发中断然后再重新加载
    //outb一次只能写入8位，16位计数初始值要写两次,哪个写低字节哪个写高字节由PIT_LOAD_LOHI加载模式决定
    outb(PIT_CHANNEL0_DATA_PORT, reload_count & 0xFF);
    outb(PIT_CHANNEL0_DATA_PORT, (reload_count >> 8) & 0xFF);

    irq_install(IRQ0_TIMER, exception_handler_time);
    irq_enbale(IRQ0_TIMER);
}

//定时器0初始化
void time_init(void)
{
    sys_tick = 0;
    init_pit();
}
