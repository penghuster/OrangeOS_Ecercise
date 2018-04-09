#include "type.h"
#include "const.h"
#include "stdio.h"
#include "protect.h"
#include "string.h"
#include "fs.h"
#include "proc.h"
#include "tty.h"
#include "global.h"
#include "proto.h"

PUBLIC int write(int fd, const void *buf, int count)
{
    MESSAGE msg;
    msg.type = WRITE;
    msg.FD = fd;
    msg.BUF = (void*)buf;
    msg.CNT = count;

    send_recv(BOTH, TASK_FS, &msg);
    return msg.CNT;
}
