#include <stdio.h>

#if 1
#define log(fmt, ...) printf("KAIKAIKAI " fmt, __VA_ARGS__)
#else
#define log(fmt, ...)
#endif
