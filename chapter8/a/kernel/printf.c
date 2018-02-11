#include "type.h"
#include "const.h"
#include "protect.h"
#include "proc.h"
#include "tty.h"
#include "string.h"
#include "proto.h"
#include "global.h"

int printf(const char *fmt, ...)
{
    int i;
    char buf[256];

    //4 是参数fmt所占堆栈中的大小
    va_list arg = (va_list)((char*)(&fmt) + 4);
    i = vsprintf(buf, fmt, arg);
    //write(buf, i);
    buf[i] = 0;
    printx(buf);

    return i;
}

