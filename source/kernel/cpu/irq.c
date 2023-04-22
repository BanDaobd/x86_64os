#include"cpu/irq.h"
#include"cpu/cpu.h"
#include"os_cfg.h"
#include"comm/cpu_instr.h"
#include"tools/log.h"

/************************************************************
IDT表
IA-32架构最多支持256种异常0~255，0~31保留或固定，剩余可自定义
************************************************************/
static gate_desc_t idt_table[IDT_TABLE_SIZE];



/**************
初始化8259芯片
详见8259A手册
**************/

static void init_pic(void)
{
    //初始化第一块芯片

    //配置ICW1 其他位0，D1 SNGL工作在单个芯片模式下还是和其他芯片，D4固定为1 | D0是IC4，
    outb(PIC0_ICW1, PIC_ICW1_ALWAYS_1 | PIC_ICW1_ICW4);
    //配置ICW2，当IR0发生中断，cpu跳到0x20中断处处理，中断起始序号0x20，占用中断号0x20~0x27
    //对应管脚IR0的中断信号对应0x20、IR1对应0x21，第二块同理，但是从0x28开始
    outb(PIC0_ICW2, IRQ_PIC_START);
    //配置ICW3，IR2管脚可以连接到其他芯片，连接了为1没连为0，所以第二位设置为1
    outb(PIC0_ICW3, 1 << 2);
    //配置ICW4，芯片有多种工作模式，涉及中断嵌套优先级等设置，我们只将D0设为1表明和8086芯片共同工作
    outb(PIC0_ICW4, PIC_ICW4_8086);

    //初始化第二块芯片
    //配置ICW1 其他位0，D1 SNGL工作在单个芯片模式下还是和其他芯片，D4固定为1 | D0是IC4，
    outb(PIC1_ICW1, PIC_ICW1_ICW4 | PIC_ICW1_ALWAYS_1);
    //配置ICW2，当IR0发生中断，cpu跳到0x20中断处处理，中断起始序号0x28
    outb(PIC1_ICW2, IRQ_PIC_START + 8);
    //配置ICW3，IR2管脚可以连接到其他芯片，告诉芯片自身连接到了另一个芯片（第一片）的IRQ2上
    outb(PIC1_ICW3, 2);
    //配置ICW4，芯片有多种工作模式，涉及中断嵌套优先级等设置，我们只将D0设为1表明和8086芯片共同工作
    outb(PIC1_ICW4, PIC_ICW4_8086);

    //此时配置好芯片但还未配置中断控制器，所以设置中断屏蔽寄存器IMR全1屏蔽全部中断,不禁止来自IR2的中断信号,因为连接到IR2的第二片的中断也被屏蔽
    outb(PIC0_IMR, 0xFF & ~(1 << 2));
    //屏蔽第二片
    outb(PIC1_IMR, 0XFF);


}



//IDT表初始化
void irq_init(void)
{
    for(int i = 0 ;i < IDT_TABLE_SIZE; i++)
        gate_desc_set(idt_table + i, KERNEL_SELECTOR_CS, (uint32_t)exception_handler_unknown, 
        GATE_P_PRRESENT | GATE_DPL0 | GATE_TYPE_INT);

    irq_install(IRQ0_DE,exception_handler_divider);
    irq_install(IRQ1_DB,exception_handler_Debug);
    irq_install(IRQ2_NMI,exception_handler_NMI);
    irq_install(IRQ3_BP,exception_handler_Breakpoint);
    irq_install(IRQ4_OF,exception_handler_Overflow);
    irq_install(IRQ5_BR,exception_handler_Bound_range);
    irq_install(IRQ6_UD,exception_handler_Invalid_opcode);
    irq_install(IRQ7_NM,exception_handler_Device_unavaliable);
    irq_install(IRQ8_DF,exception_handler_Double_fault);
    irq_install(IRQ10_TS,exception_handler_Invalid_tss);
    irq_install(IRQ11_NP,exception_handler_Segment_not_present);
    irq_install(IRQ12_SS,exception_handler_Stack_segment_fault);
    irq_install(IRQ13_GP,exception_handler_General_protection);
    irq_install(IRQ14_PF,exception_handler_Page_fault);
    irq_install(IRQ16_MF,exception_handler_Fpu_error);
    irq_install(IRQ17_AC,exception_handler_Aligment_check);
    irq_install(IRQ18_MC,exception_handler_Machine_check);
    irq_install(IRQ19_XM,exception_handler_Simd_exception);
    irq_install(IRQ20_VE,exception_handler_Virtual_exception);
    irq_install(IRQ21_CP,exception_handler_Control_exception);

    lidt((uint32_t)idt_table, sizeof(idt_table));

    //初始化pic控制器
    init_pic();
}

//将相应的异常处理函数定位到IDT表，参数为异常号和异常处理函数的入口地址
int irq_install(int irq_num, irq_handler_t handler)
{
    if(irq_num >= IDT_TABLE_SIZE)
        return -1;
    
    gate_desc_set(idt_table + irq_num, KERNEL_SELECTOR_CS, (uint32_t)handler,
    GATE_P_PRRESENT | GATE_DPL0 | GATE_TYPE_INT);

    return 0;
}

static void dump_core_regs(exception_frame_t * frame)
{
    log_printf("IRQ: %d, error code: %d",frame->num, frame->err_code);
    //异常发生时没有传递ss信息，但是设置成相同值所以用fs暂且替代
    log_printf("CS: %d\nDS: %d\nES: %d\nSS; %d\nFS: %d\nGS: %d\n",
    frame->cs, frame->ds, frame->es, frame->fs, frame->fs, frame->gs);

    log_printf("EAX: 0x%x\nEDX: 0x%x\nEBX: 0x%x\nECX; 0x%x\nEDI: 0x%x\nESI: 0x%x\nEBP: 0x%x\nESP: 0x%x\n",
    frame->eax, frame->edx, frame->ebx, frame->ecx, frame->edi, frame->esi, frame->ebp, frame->esp);

    log_printf("EIP: 0x%x\nEFLAGS: 0x%x\n", frame->eip, frame->eflags);
}

static void do_default_handler(exception_frame_t * frame, const char *msg)
{
    log_printf("-----------------------------------------------------------");
    //打印错误信息
    log_printf("IRQ/Exception happend: %s", msg);
    //打印内核寄存器
    dump_core_regs(frame);
    for(;;)
        hlt();
}


/*******************************************************
芯片中断管理函数
中断的打开和关闭受制于EFLAGS的IF标志位和8259的IMR寄存器
想要开启把IMR寄存器的对应位设置为0（和IF相反）
IF全局中断控制，为1开，为0屏蔽
*******************************************************/

//IMR开中断，相应位置0
void irq_enbale(int irq_num)
{
    if(irq_num < IRQ_PIC_START)
        return;
    
    irq_num -= IRQ_PIC_START;
    if(irq_num < 8)
    {
        uint8_t mask = inb(PIC0_IMR) & ~(1 << irq_num);
        outb(PIC0_IMR, mask);
    }else{
        irq_num -= 8;
        uint8_t mask = inb(PIC1_IMR) & ~(1 << irq_num);
        outb(PIC1_IMR, mask);
    }
}

//IMR关中断，相应位置1
void irq_disbale(int irq_num)
{
    if(irq_num < IRQ_PIC_START)
        return;
    
    irq_num -= IRQ_PIC_START;
    if(irq_num < 8)
    {
        uint8_t mask = inb(PIC0_IMR) & (1 << irq_num);
        outb(PIC0_IMR, mask);
    }else{
        irq_num -= 8;
        uint8_t mask = inb(PIC1_IMR) & (1 << irq_num);
        outb(PIC1_IMR, mask);
    }
    
}

//全局开中断
void irq_enable_global(void)
{
    sti();
}

//全局关中断
void irq_disable_global(void)
{
    cli();
}

//告知8259中断结束需要向ocw2寄存器发送一条特定指令
void pic_send_eoi(int irq_num)
{
    irq_num -= IRQ_PIC_START;
    //向第二块芯片eoi位传输一个特殊值
    if(irq_num >= 8){
        outb(PIC1_OCW2, PIC_OCW2_EOI);
    }
        
    outb(PIC0_OCW2, PIC_OCW2_EOI);
}

/*****************************************
IDT表基址+offset指向的中断函数中调用的接口
对真正的处理函数做了一层封装
*****************************************/
void do_handler_unknown(exception_frame_t * frame)
{
    do_default_handler(frame, "unkonwn exception");
}

void do_handler_divider(exception_frame_t * frame)
{
    do_default_handler(frame, "Divder exception");
}

void do_handler_Debug(exception_frame_t * frame)
{
    do_default_handler(frame, "Debug exception");
}

void do_handler_NMI(exception_frame_t * frame)
{
    do_default_handler(frame, "NMI exception");
}

void do_handler_Breakpoint(exception_frame_t * frame)
{
    do_default_handler(frame, "Breakpoint exception");
}

void do_handler_Overflow(exception_frame_t * frame)
{
    do_default_handler(frame, "Overflow exception");
}

void do_handler_Bound_range(exception_frame_t * frame)
{
    do_default_handler(frame, "Bound_range exception");
}

void do_handler_Invalid_opcode(exception_frame_t * frame)
{
    do_default_handler(frame, "Invalid_opcode exception");
}

void do_handler_Device_unavaliable(exception_frame_t * frame)
{
    do_default_handler(frame, "Device_unavaliable exception");
}

void do_handler_Double_fault(exception_frame_t * frame)
{
    do_default_handler(frame, "Double_fault exception");
}

//9

void do_handler_Invalid_tss(exception_frame_t * frame)
{
    do_default_handler(frame, "Invalid_tss exception");
}

void do_handler_Segment_not_present(exception_frame_t * frame)
{
    do_default_handler(frame, "Segment_not_present exception");
}

void do_handler_Stack_segment_fault(exception_frame_t * frame)
{
    do_default_handler(frame, "Stack_segment exception");
}

void do_handler_General_protection(exception_frame_t * frame)
{
    do_default_handler(frame, "General_protection exception");
}

void do_handler_Page_fault(exception_frame_t * frame)
{
    do_default_handler(frame, "Page_fault exception");
}

//15

void do_handler_Fpu_error(exception_frame_t * frame)
{
    do_default_handler(frame, "Divder exception");
}

void do_handler_Aligment_check(exception_frame_t * frame)
{
    do_default_handler(frame, "Aligment_check exception");
}

void do_handler_Machine_check(exception_frame_t * frame)
{
    do_default_handler(frame, "Machine_check exception");
}

void do_handler_Simd_exception(exception_frame_t * frame)
{
    do_default_handler(frame, "Simd exception");
}

void do_handler_Virtual_exception(exception_frame_t * frame)
{
    do_default_handler(frame, "Virtual exception");
}

void do_handler_Control_exception(exception_frame_t * frame)
{
    do_default_handler(frame, "Control exception");
}