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

PUBLIC int read(int fd, void*buf, int count)
{
    MESSAGE msg;
    msg.type = READ;
    msg.FD = fd;
    msg.BUF = buf;
    msg.CNT = count;

    printf("read-1\n");
    send_recv(BOTH, TASK_FS, &msg);
        printf("read-4\n");
    return msg.CNT;
}
