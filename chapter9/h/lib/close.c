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



/**
 * close 
 *
 * Close a file descriptor 
 *
 * @param fd 
 *
 * @return zero if successful othewise -1
 */ 
PUBLIC int close(int fd)
{
    MESSAGE msg;
    msg.type = CLOSE;
    msg.FD = fd;
    
    send_recv(BOTH, TASK_FS, &msg);
    return msg.RETVAL;
}
