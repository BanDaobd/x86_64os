#ifndef IRQ_H_
#define IRQ_H_

#include"comm/types.h"

/*********************
异常中断号，带*的有用到
手册3，6-15
*********************/


#define IRQ0_DE     0   //除0异常*
#define IRQ1_DB     1   //
#define IRQ2_NMI    2
#define IRQ3_BP     3
#define IRQ4_OF     4 
#define IRQ5_BR     5 
#define IRQ6_UD     6 
#define IRQ7_NM     7 
#define IRQ8_DF     8 
//#define IRQ9_     9 
#define IRQ10_TS    10  //TSS异常*
#define IRQ11_NP    11  //段异常*
#define IRQ12_SS    12  //段异常*
#define IRQ13_GP    13  //段异常*
#define IRQ14_PF    14  //页异常*
//15保留
#define IRQ16_MF    16  //浮点运算异常
#define IRQ17_AC    17  //
#define IRQ18_MC    18  //
#define IRQ19_XM    19  //SIMD浮点运算异常
#define IRQ20_VE    20  //
#define IRQ21_CP    21  //


/*********************
8259芯片设置
*********************/
//第一块芯片
#define PIC0_ICW1   0x20    //端口地址0x20
#define PIC0_ICW2   0x21    //端口地址0x21
#define PIC0_ICW3   0x21    //端口地址0x21
#define PIC0_ICW4   0x21    //端口地址0x21
#define PIC0_IMR    0x21    //中断屏蔽寄存器

#define PIC0_OCW2   0x20    //中断结束/可重入标志

//第二块芯片
#define PIC1_ICW1   0xA0    //端口地址0xA0
#define PIC1_ICW2   0xA1    //端口地址0xA1
#define PIC1_ICW3   0xA1    //端口地址0xA1
#define PIC1_ICW4   0xA1    //端口地址0xA1
#define PIC1_IMR    0xA1    //中断屏蔽寄存器

#define PIC1_OCW2   0xA0    //中断结束/可重入标志

//根据手册配置
#define PIC_ICW1_ALWAYS_1   (1 << 4)
#define PIC_ICW1_ICW4       (1 << 0)
#define PIC_ICW4_8086       (1 << 0)

#define PIC_OCW2_EOI        (1 << 5)    //写入可重入标志位的值

#define IRQ_PIC_START       0x20        //中断起始序号

#define IRQ0_TIMER          0x20



/*************************************************************
存储异常发生时的寄存器信息
出现异常时异常处理函数需要得知异常发生时的寄存器信息（比如eip）
*************************************************************/

typedef struct _exception_frame_t
{
    uint32_t gs,fs,es,ds;
    uint32_t edi,esi,ebp,esp,edx,ebx,ecx,eax;
    //err错误码的存在与否根据中断类型，不存在设为0
     uint32_t num, err_code;
    uint32_t eip,cs,eflags;
}exception_frame_t;


/**************************************************************/

//异常中断函数的函数指针，简化参数
typedef void (*irq_handler_t) (void);

//初始化IDT表
void irq_init(void);

//在IDT表中注册异常中断函数信息
int irq_install(int irq_num, irq_handler_t handler);




/*******************************************************
中断管理函数
*******************************************************/

//IMR定时器开中断，相应位置1
void irq_enbale(int irq_num);

//IMR定时器关中断，相应位置1
void irq_disbale(int irq_num);

//全局开中断
void irq_enable_global(void);

//全局关中断
void irq_disable_global(void);

//中断重入，每次中断结束需要告诉8259芯片已此次中断结束，参数需要IRQ序号
void pic_send_eoi(int irq_num);


/*************************************************
异常处理函数,IDT表根据中断号调用
不能用纯c写，因为中断返回需要iret指令，c函数只能ret返回
在这个函数中使用call调用do_handler_unknown函数做进一步处理
*************************************************/

void exception_handler_unknown(void);

void exception_handler_divider(void);

void exception_handler_Debug(void);

void exception_handler_NMI(void);

void exception_handler_Breakpoint(void);

void exception_handler_Overflow(void);

void exception_handler_Bound_range(void);

void exception_handler_Invalid_opcode(void);

void exception_handler_Device_unavaliable(void);

void exception_handler_Double_fault(void);

//9

void exception_handler_Invalid_tss(void);

void exception_handler_Segment_not_present(void);

void exception_handler_Stack_segment_fault(void);

void exception_handler_General_protection(void);

void exception_handler_Page_fault(void);

//15

void exception_handler_Fpu_error(void);

void exception_handler_Aligment_check(void);

void exception_handler_Machine_check(void);

void exception_handler_Simd_exception(void);

void exception_handler_Virtual_exception(void);

void exception_handler_Control_exception(void);

void exception_handler_time(void);

#endif