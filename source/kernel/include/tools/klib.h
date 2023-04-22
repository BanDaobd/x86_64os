#ifndef KLIB_H_
#define KLIB_H_
#include"comm/types.h"
#include"stdarg.h"
/************
定义工具函数
dest目的
src源
************/

//字符串复制
void kernel_strcpy(char *dest, const char *src);

//复制指定大小
void kernel_strncpy(char *dest, const char *src, int size);

//字符串比较
int kernel_strncmp(const char *s1, const char *s2, int size);

//字符串长度计算
int kernel_strlen(const char *str);

//内存复制
void kernel_memcpy(void *dest, void *src, int size);

//内存设置
void kernel_memset(void *dest, uint8_t v, int size);

//内存比较
int kernel_memcmp(void *d1, void *d2, int size);


void kernel_sprintf(char *buf, const char *fmt, ...);
void kernel_vsprintf(char *buf, const char *fmt, va_list args);


/************************************************
如果不是开发版，启用ASSERT断言
__FILE__, __LINE__, __func__是编译器自带的宏定义
可以给出触发断言位置的文件名、行号、函数信息
#expr，#会展开参数信息(把参数转换为字符串输出)
cond就是展开的参数字符串
测试版中把宏定义成不影响程序运行的任意代码即可
*************************************************/
#ifndef RELEASE

#define ASSERT(expr)    \
    if(!(expr)) pannic(__FILE__, __LINE__, __func__, #expr)
void pannic(const char *file, int line, const char *func, const char *cond);  

#else

#define ASSERT(expr)    ((void)0)
#endif

#endif 