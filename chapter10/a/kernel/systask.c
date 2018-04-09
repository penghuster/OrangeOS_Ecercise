#include "type.h"
#include "const.h"
#include "protect.h"
#include "tty.h"
#include "proc.h"
#include "string.h"
#include "fs.h"
#include "global.h"
#include "string.h"
#include "proto.h"


/*
 * @function task_sys
 *
 * <Ring 1> The main loop of TASK_SYS
 *
 */ 
PUBLIC void task_sys()
{
    MESSAGE msg;
    while(1)
    {
        send_recv(RECEIVE, ANY, &msg);
        int src = msg.source;

        switch(msg.type)
        {
        case GET_TICKS:
            msg.RETVAL = ticks;
            send_recv(SEND, src, &msg);
            break;
        case GET_PID:
            msg.type = SYSCALL_RET;
            msg.PID = src;
            send_recv(SEND, src, &msg);
            break;
        default:
            panic("unknown msg type");
            break;
        }
    }
}

