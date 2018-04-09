#include "type.h"
#include "const.h"
#include "protect.h"
#include "string.h"
#include "fs.h"
#include "proc.h"
#include "tty.h"
#include "global.h"
#include "proto.h"

#include "hd.h"

extern struct dev_drv_map dd_map[];

/*
 * task_fs
 *
 * <Ring 1> The main loop of TASK FS
 */ 
PUBLIC void task_fs()
{
    printl("Task FS begins.\n");

    //open the device: hard disk
    MESSAGE driver_msg;
    driver_msg.type = DEV_OPEN;
    driver_msg.DEVICE = MINOR(ROOT_DEV);
    assert(dd_map[MAJOR(ROOT_DEV)].driver_nr != INVALID_DRIVER);
    send_recv(BOTH, dd_map[MAJOR(ROOT_DEV)].driver_nr, &driver_msg);

    spin("FS");
}