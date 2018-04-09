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

/*
 * open 
 *
 * open/create a file
 *
 * @param pathname the full path of the file to be created/opened 
 * @param flags O_CREAT, O_RDWR, etc.
 * 
 * @return File descriptor if successful, otherwise -1.
 */ 
PUBLIC int open(const char *pathname, int flags)
{
    MESSAGE msg;
    msg.type = OPEN;

    msg.PATHNAME = (void*)pathname;
    msg.FLAGS = flags;
    msg.NAME_LEN = strlen(pathname);

    send_recv(BOTH, TASK_FS, &msg);
    assert(msg.type == SYSCALL_RET);

    return msg.FD;
}
