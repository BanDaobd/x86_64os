#ifndef TIME_H_
#define TIME_H_

//时钟信号频率,即每秒钟时钟节拍数
#define PIT_OSC_FREQ             1193182

//0x43端口，写入控制命令
#define PIT_COMMAND_MODE_PORT    0x43

//0x40端口，定时器0
#define PIT_CHANNEL0_DATA_PORT   0x40

/*********************
对0x43端口写入的控制位
**********************/

//选择定时器0
#define PIT_CHANNEL0            (0 << 6)
//加载模式是11，决定定时器的初始计数值如何设置，先写低8位
#define PIT_LOAD_LOHI           (3 << 4)
//mode3，简单的每隔一段时间中断并计数
#define PIT_MODE3               (3 << 1)

void time_init(void);

#endif