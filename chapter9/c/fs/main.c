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

//    init_fs();

    //open the device: hard disk
    MESSAGE driver_msg;
    driver_msg.type = DEV_OPEN;
    driver_msg.DEVICE = MINOR(ROOT_DEV);
    assert(dd_map[MAJOR(ROOT_DEV)].driver_nr != INVALID_DRIVER);
    send_recv(BOTH, dd_map[MAJOR(ROOT_DEV)].driver_nr, &driver_msg);

    spin("FS");
}

/*
 * init_fs
 *
 * <Ring 1> The main loop of TASK_FS 
 */ 
PRIVATE void init_fs()
{
    //open the device: hard disk 
    MESSAGE driver_msg;
    driver_msg.type = DEV_OPEN;
    driver_msg.DEVICE = MINOR(ROOT_DEV);
    assert(dd_map[MAJOR(ROOT_DEV)].driver_nr != INVALID_DRIVER);
    send_recv(BOTH, dd_map[MAJOR(ROOT_DEV)].driver_nr, &driver_msg);

//    mkfs();
}

/*
 * mkfs 
 *
 * <Ring 1> Make a available Oringe'S FS in disk. It will 
 * - Write a super block to sector 1.
 * - Create three special files: dev_tty0, dev_tty1, dev_tty2
 * - Create the inode map 
 * - Create the sector map 
 * - Create the inodes of the files 
 * - Create '/', the root directory 
 */ 
PRIVATE void mkfs()
{
    MESSAGE driver_msg;
    int i, j;

    int bits_per_sect = SECTOR_SIZE * 8;

    //get the geometry of ROOTDEV 
    struct part_info geo;
    driver_msg.type = DEV_IOCTL;
    driver_msg.DEVICE = MINOR(ROOT_DEV);
    driver_msg.REQUEST = DIOCTL_GET_GEO;
    driver_msg.BUF = &geo;
    driver_msg.PROC_NR = TASK_FS;
    assert(dd_map[MAJOR(ROOT_DEV)].driver_nr != INVALID_DRIVER);
    send_recv(BOTH, dd_map[MAJOR(ROOT_DEV)].driver_nr, &driver_msg);

}
