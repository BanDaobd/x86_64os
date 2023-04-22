

  	.code16
 	.text
	.global _start
    .extern loader_entry
_start:
 
	jmp loader_entry


	.code32
	.text
	//外部链接性
	.global protect_mode_entry
	.extern load_kernel
//上面的loader_entry跳到这里
protect_mode_entry:
	//对还处于16位的寄存器做一些修改
	mov $16, %ax
	mov %ax, %ds
	mov %ax, %ss
	mov %ax, %es
	mov %ax, %fs
	mov %ax, %gs
	//真正进入32位模式
	jmp $8, $load_kernel

