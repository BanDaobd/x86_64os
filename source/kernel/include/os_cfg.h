#ifndef CS_CFG_H_
#define CS_CFG_H_

#define GDT_TABLE_SIZE  256

#define IDT_TABLE_SIZE  128

#define KERNEL_SELECTOR_CS  (1 * 8)

#define KERNEL_SELECTOR_DS  (2 * 8)

#define KERNEL_STACK_SIZE   (8 * 1024)


//设置时钟中断间隔,单位毫秒,每隔10ms中断一次
#define OS_TICK_MS      10


#define DOS_VERSION     "1.0.0"

#endif