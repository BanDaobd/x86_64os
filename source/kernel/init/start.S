# gcc支持的大S后缀，（需要使用gcc命令处理）汇编文件引用c文件，as编译器不支持
#include "os_cfg.h"

	.text
	.extern kernel_init
	.extern init_main
	.global _start
	# void start(boot_info_t * boot_info)
_start:

	# 将函数((void (*)(boot_info *))SYS_KERNEL_LOAD_ADDR)(&boot_info)
	# 压入栈的参数boot_info *，传递boot_info指针值给kernel

	push %ebp
	mov %esp, %ebp
	# 只有一个参数，栈中间隔一个存放ebp的栈和返回地址，4（存放ebp）+4（存放返回地址）+%esp即保存参数boot_info *的栈地址
	mov 0x8(%ebp), %eax

	# mov 0x4(%esp), %eax

	# kernel_init(boot_info)
	push %eax
	call kernel_init

	# 做远跳转加载新gdt,选择子会加载进CS
	jmp $KERNEL_SELECTOR_CS, $gdt_reload
	
gdt_reload:
	# 把数据段加载到这些寄存器，选择子重新加载
	mov $KERNEL_SELECTOR_DS, %ax
	mov %ax, %ds
	mov %ax, %ss
	mov %ax, %es
	mov %ax, %fs
	mov %ax, %gs

	# 用新栈而不是loader里设置的
	mov $(stack + KERNEL_STACK_SIZE), %esp
	jmp init_main

//定义栈起始地址和名称和大小
	.bss
.comm stack, KERNEL_STACK_SIZE


//要做现场保护，否则iret返回后寄存器值变化运行会出异常
//宏定义 名称、异常号、是否压入异常错误码
.macro exception_handler name num with_err_code
	.text
	.extern do_handler_\name
	.global	exception_handler_\name
exception_handler_\name:
	//如果没有错误码手动压0，有错误码会自动压入
	//这样就能直接在struct设置一个错误码成员而不用因为错误码是否存在烦恼是否加错误码成员了
	.if \with_err_code == 0
		push $0
	.endif

	push $\num
	pusha
	push %ds
	push %es
	push %fs
	push %gs

	//指向gs起始地址
	push %esp
	call do_handler_\name
	add $4, %esp
	//pop %esp

	pop %gs
	pop %fs
	pop %es
	pop %ds
	popa

	add $8, %esp
	//iret弹出栈时不会弹err
	iret
.endm

exception_handler unknown, -1, 0
exception_handler divider, 0, 0
exception_handler Debug, 1, 0
exception_handler NMI, 2, 0
exception_handler Breakpoint, 3, 0
exception_handler Overflow, 4, 0
exception_handler Bound_range, 5, 0
exception_handler Invalid_opcode, 6, 0
exception_handler Device_unavaliable, 7, 0
exception_handler Double_fault, 8, 1
//9
exception_handler Invalid_tss, 10, 1
exception_handler Segment_not_present, 11, 1
exception_handler Stack_segment_fault, 12, 1
exception_handler General_protection, 13, 1
exception_handler Page_fault, 14, 1
//15
exception_handler Fpu_error, 16, 0
exception_handler Aligment_check, 17, 1
exception_handler Machine_check, 18, 0
exception_handler Simd_exception, 19, 0
exception_handler Virtual_exception, 20, 0
exception_handler Control_exception, 21, 1

//定时器中断
exception_handler time, 0x20, 0


//这是一个栈切换函数simple_switch(&from, to)，参数是栈指针
	.text
	.global simple_switch
simple_switch:
	//取&from参数
	mov 4(%esp), %eax
	//将simple_switch(&from, to) 中的参数to取出来
	mov 8(%esp), %edx

	//保存现在任务的栈
	push %ebp
	push %ebx
	push %esi
	push %edi

	//写到tss的内存stack
	//没push前的栈指针值在c函数中已经写入了
	//push之后指针值会移动，所以我们需要把移动后的指针值再写入
	//%eax是参数from，参数from保存了tss中stack的内存地址，我们使用mov把栈指针值写入到%eax中存储的内存地址处
	//相当于值%esp写入到参数from中stack指向的内存*stack(上一个任务的栈指针) ，也就是现有的栈指针被保存到现在任务的tss中
	//(注意这里%eax的值是4(%esp)，也就是参数form中的stack指针)
	mov %esp, (%eax)

	//栈切换，把上面参数%edx中存的8(%esp)（即参数to（to->stack），下一个任务的栈指针)赋值给%esp实现栈切换
	//也就是传入的tss的stack值给到%esp了
	mov %edx, %esp

	//恢复上一个任务的栈
	pop %edi
	pop %esi
	pop %ebx
	pop %ebp

	ret