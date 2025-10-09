#ifndef UTILS_H
#define UTILS_H
#include <string.h>
#include <errno.h>
void eexit(char *str);
void nexit(const char *__restrict, ...);
void print_ctime();
int set_no_blocking(int fd);
int set_blocking(int fd);
char *gf_time(void);
void printf_error(int se, const char *__restrict, ...);
#endif
