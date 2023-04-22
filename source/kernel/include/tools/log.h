#ifndef LOG_H_
#define LOG_H_

void log_init(void);

//...可变参数列表
void log_printf(const char * fmt, ...);

#endif