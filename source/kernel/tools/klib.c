#include"tools/klib.h"
#include"tools/log.h"
#include"comm/cpu_instr.h"

/****************************
一些基础函数
包括内存处理和字符串处理函数
*****************************/

//字符串复制
void kernel_strcpy(char *dest, const char *src)
{
    if(!dest || !src)
        return ;

    //或许不用加？
    if(dest == src)
        return ;

    char *d = dest;
    const char *s = src;

    while(*d && *s)
        *d++ = *s++;
    
    //后缀运算符，复制了完整的字符串并在最后加'\0'
    *d ='\0';
}

//复制指定大小
void kernel_strncpy(char *dest, const char *src, int size)
{
     if(!dest || !src || !size)
        return ;

    if(dest == src)
        return ;

    char *d = dest;
    const char *s = src;

    while((size-- > 0) && (*s))
        *d++ = *s++;

    /**************************************************************
    情况一
    size==0时循环直接结束，复制了size个字符，
    *d++是先存入值指针再移动，所以最后一次循环指向size+1个字符位置，
    因此复制结果是大小size的大小包含'\0'
    也就是说复制size-1个字符，第size个字符是'\0'

    情况二
    要复制的字符串大小小于size
    size还>0时*s就先到了'\0'，但是循环到'\0'就停止了
    并且指针指向src指向的字符串大小的后一个字符位置，
    也就是说复制了完整的src字符串，并在末尾加'\0'
    ***************************************************************/
    if(size == 0)
        *(d - 1) = '\0';
    else
        *d ='\0';
    
}

//字符串比较,如果一个字符串是另一个字符串从第一个字符开始计算的子串，那么也算作相等
int kernel_strncmp(const char *s1, const char *s2, int size)
{
    if(!s1 || !s2 || !size)
        return 1;

    if(s1 == s2)
        return -1;
    
    while(*s1 && *s2 && (*s1 == *s2) && size--)
    {
        s1++;
        s2++;
    }

    if(*s1 == '\0' || *s2 == '\0' || *s1 == *s2)
        return 0; 
}

//字符串长度计算
int kernel_strlen(const char *str)
{
    if(!str)
        return 0;
    
    const char *c = str;
    int len = 0;
    while(*c++)
        len++;

    return len;
}

//内存复制
void kernel_memcpy(void *dest, void *src, int size)
{
    if(!dest || !src || !size)
        return ;

    if(dest == src)
        return ;

    uint8_t *s=(uint8_t *)src;
    uint8_t *d=(uint8_t *)dest;
    while(size--)
        *d++ = *s++;
}

//内存设置
void kernel_memset(void *dest, uint8_t v, int size)
{
    if(!dest || !size)
        return ;

    uint8_t *d = (uint8_t *)dest;
    while(size--)
        *d++ = v;
}

//内存比较
int kernel_memcmp(void *d1, void *d2, int size)
{
    if(!d1 || !d2 || !size)
        return 1;

    if(d1 == d2)
        return -1;

    uint8_t *p_d1=(uint8_t *)d1;
    uint8_t *p_d2=(uint8_t *)d2;

    while(size--)
    {
        if(*p_d1++ != *p_d2++)
            return 1;
    }
    return 0;
}


/**************
格式化输出处理
**************/

void kernel_itoa(char *buf, int num, int base)
{

    static const char *num2ch = {"FEDCBA9876543210123456789ABCDEF"};
    char *p = buf;
    int old_num = num;

    if((base != 2) && (base != 8) && (base != 10) && (base != 16))
    {
        *p = '\0';
        return;
    }

    if((num < 0) && (base == 10))
    {
        *p++ = '-';
    }

    //num如果是负数，求余得到负值，因此加入偏移量15（因为数组下标从0开始）
    //负数得到的下标从偏移量处向前寻找，正数得到的下标从偏移量处向后寻找
    char ch;
    do
    {
        ch = num2ch[num % base + 15];
        *p++ = ch;
        num = num / base;

    }while(num);

    *p-- = '\0';

    //取余得到的数是相反排列的，我们从两边交换数字
    //交换数字时如果是负数，那么需要从负号后开始交换，
    //但是num在取余后变为0，所以要使用old_num保存num的初值来判断数的正负
    //p指向末尾，start指向开头
    char *start = (old_num > 0) ? buf : buf + 1;
    char temp;
    while(start < p)
    {
        temp = *start;
        *start = *p;
        *p = temp;

        p--;
        start++;
    }

}

void kernel_sprintf(char *buf, const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);

    kernel_vsprintf(buf, fmt, args);

    va_end(args);
}



/******************************************************************************************************************
格式化显示
buf指向缓冲区，显示的是缓冲区的值
fmt是printf的字符串参数，args是格式化输入对应的值
curr是指向缓冲区的指针，ch保存字符串参数的值
"Version: %s", "1.0.0"
其中Version: %s是字符串参数fmt指向的内容，1.0.0是格式化输出对应的值args（可以有多个）
通过状态机解析fmt中的%后跟的参数，把%对应的值（有可能是数字）转换为字符，两者都写入缓冲区，后者写入%参数的对应位置
把缓冲区传给串口输出
*******************************************************************************************************************/

void kernel_vsprintf(char *buf, const char *fmt, va_list args)
{
    enum{NORMAL, READ_FMT} state =NORMAL;
    char *curr = buf;
    char ch;
    //需要定义两种状态机NORMAL普通状态，READ_FMT格式化输出状态
    //因为arg保存上一次的值所以即使连续遇到格式化字符%也不会重复停留
    while((ch = *fmt++))
    {
        switch(state)
        {
            case NORMAL:
                if(ch == '%')
                    state = READ_FMT;
                else
                    *curr++ =ch;
                break;
            case READ_FMT:
                if(ch == 's')
                {
                    const char *str = va_arg(args, char *);
                    uint32_t len = kernel_strlen(str);
                    while(len--)
                        *curr++ = *str++;
                }
                else if(ch == 'd')
                {
                    int num = va_arg(args, int);
                    //数字转换成ascii对应的字符串才能输出,10表示10进制
                    kernel_itoa(curr, num, 10);
                    //这个函数中缓冲区已经写入字符但curr指针没动，所以要移动相应的大小
                    //strlen返回从当前下标到字符串末尾的距离值
                    //注意strlen不计算'\0'
                    curr += kernel_strlen(curr);
                }
                else if(ch == 'x')
                {
                    int num = va_arg(args, int);
                    //数字转换成ascii对应的字符串才能输出，16进制
                    kernel_itoa(curr, num, 16);
                    curr += kernel_strlen(curr);
                }
                else if(ch == 'c')
                {
                    char c = va_arg(args, int);
                    *curr++ = c;
                }
                state = NORMAL;
                break;
        }
    }
}

void pannic(const char *file, int line, const char *func, const char *cond)
{
    log_printf("assert failed! %s", cond);

    log_printf("file: %s\nline: %d\nfunc: %s\n", file, line, func);
    
    for(;;)
        hlt();
}