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

PRIVATE void mkfs();
PRIVATE void init_fs();
//int test(int x, int y, int z, u64 n, int m,void* l);


//extern struct dev_drv_map dd_map[];

/*
 * task_fs
 *
 * <Ring 1> The main loop of TASK FS
 */ 
PUBLIC void task_fs()
{
    printl("Task FS begins.\n");

    init_fs();

    //open the device: hard disk
    //MESSAGE driver_msg;
    //driver_msg.type = DEV_OPEN;
    //driver_msg.DEVICE = MINOR(ROOT_DEV);
    //assert(dd_map[MAJOR(ROOT_DEV)].driver_nr != INVALID_DRIVER);
    //send_recv(BOTH, dd_map[MAJOR(ROOT_DEV)].driver_nr, &driver_msg);

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

    mkfs();
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

    printl("dev size: 0x%x sectors\n", geo.size);



    //--------super block---------//
    struct super_block sb;
    sb.magic = MAGIC_V1;
    sb.nr_inodes = bits_per_sect; //inode数，一个inode对应1位, 且节点位图占一个扇区
    sb.nr_inode_sects = sb.nr_inodes * INODE_SIZE / SECTOR_SIZE; //节点数组占用的扇区数 
    sb.nr_sects = geo.size;  //partition size in sector 当前分区的扇区数
    sb.nr_imap_sects = 1; //节点位图所占用的扇区数
    sb.nr_smap_sects = sb.nr_sects / bits_per_sect + 1; //扇区位图所占用的占用的扇区数
    sb.n_1st_sect = 1 + 1 + /*boot sector & super block */ 
        sb.nr_imap_sects + sb.nr_smap_sects + sb.nr_inode_sects; //数据开始扇区号
    sb.root_inode = ROOT_INODE; //根目录的节点号
    sb.inode_size = INODE_SIZE; //节点数组的大小

    struct inode x;
    sb.inode_isize_off = (int)&x.i_size - (int)&x;
    sb.inode_start_off = (int)&x.i_start_sect - (int)&x;
    sb.dir_ent_size = DIR_ENTRY_SIZE; 

    struct dir_entry de;
    sb.dir_ent_inode_off = (int)&de.inode_nr - (int)&de;
    sb.dir_ent_fname_off = (int)&de.name - (int)&de;

    memset(fsbuf, 0x90, SECTOR_SIZE); //为什么是0x90？
    memcpy(fsbuf, &sb, SUPER_BLOCK_SIZE);


    //write the super block 
    WR_SECT(ROOT_DEV, 1);
   // rw_sector(1004, ROOT_DEV, 512, 512, 3, 0x600000);
    
    printl("dev base:0x%x00, sb:0x%x00, imap:0x%x00, smap:0x%x00\n"
            "         inodes:0x%x00, 1st_sector:0x%x00\n",
            geo.base * 2,
            (geo.base + 1) * 2,
            (geo.base + 1 + 1) * 2,
            (geo.base + 1 + 1 + sb.nr_imap_sects) * 2,
            (geo.base + 1 + 1 + sb.nr_imap_sects + sb.nr_smap_sects) * 2,
            (geo.base + sb.n_1st_sect) * 2); //为什么都要乘以2 
//while(1){}



    //--------inode map----------//
    memset(fsbuf, 0, SECTOR_SIZE);
    for(i = 0; i < (NR_CONSOLES + 2); i++)
    {
        fsbuf[0] |= 1 << i;
    }

    assert(fsbuf[0] == 0x1F); //0001 1111
    /*
     * bit 0: reserved
     * bit 1: the first inode which indicates  '/'  root directory
     * bit 2: /dev_tty0
     * bit 3: /dev_tty1
     * bit 4: /dev_tty2
     */ 

    WR_SECT(ROOT_DEV, 2);



    //--------sector map-------//
    memset(fsbuf, 0, SECTOR_SIZE);
    //NR_DEFAULT_FILE_SECTS 文件默认占用扇区数， 1代表sector map 第一位为 reserved
    int nr_sects = NR_DEFAULT_FILE_SECTS + 1;

    for(i = 0; i < nr_sects / 8; i++)
    {
        fsbuf[i] = 0xFF;
    }

    for(j = 0; j < nr_sects % 8; j++)
    {
        fsbuf[i] |= (1 << j);
    }

    WR_SECT(ROOT_DEV, 2 + sb.nr_imap_sects); //此处如果根目录文件大小大于 512 * 8 个扇区呢？？？？

    //zeromemory the reset sector-map 
    memset(fsbuf, 0, SECTOR_SIZE);
    for(i = 1; i < sb.nr_smap_sects; i++)
    {
        WR_SECT(ROOT_DEV, 2 + sb.nr_imap_sects + i);
    }


    //-----------inodes-----------//
    //inode of '/'
    memset(fsbuf, 0, SECTOR_SIZE);
    struct inode* pi = (struct inode*)fsbuf;
    pi->i_mode = I_DIRECTORY; //文件模式
    pi->i_size = DIR_ENTRY_SIZE * 4;//4 files //文件实际大小
    // '.'
    // 'dev_tty0', 'dev_tty1', 'dev_tty2'
    
    pi->i_start_sect = sb.n_1st_sect; //文件存放的开始扇区
    pi->i_nr_sects = NR_DEFAULT_FILE_SECTS; //文件占用的扇区数

    //inode of /dev_tty0-2
    for(i = 0; i < NR_CONSOLES; i++)
    {
        pi = (struct inode*)(fsbuf + (INODE_SIZE * (i + 1)));
        pi->i_mode = I_CHAR_SPECIAL; //字符设备类型
        pi->i_size = 0;
        pi->i_start_sect = MAKE_DEV(DEV_CHAR_TTY, i);//对于字符设备类型文件，此值为其设备号
        pi->i_nr_sects = 0;
    }
    WR_SECT(ROOT_DEV, 2 + sb.nr_imap_sects + sb.nr_smap_sects);

    // '/'
   memset(fsbuf, 0, SECTOR_SIZE); 
   struct dir_entry* pde = (struct dir_entry*)fsbuf;

   pde->inode_nr = 1;
   strcpy(pde->name, ".");

   for(i = 0; i < NR_CONSOLES; i++)
   {
       pde++; //跳过 ’/‘ 的 dir_entry 
       pde->inode_nr = i + 2;
       sprintf(pde->name, "dev_tty%d", i);
   }

   WR_SECT(ROOT_DEV, sb.n_1st_sect);

}

/*
 * rw_sector
 *
 * <Ring 1> R/W a sector via messaging with the corresponding driver.
 *
 * @param io_type DEV_READ or DEV_WRITE
 * @param dev device_nr
 * @param pos Byte offset from/to where to r/w.
 * @param bytes r/w count in bytes
 * @param proc_nr To whom the buffer belongs 
 * @param buf r/w buffer
 *
 * @return zero if success
 */ 
PUBLIC int rw_sector(int io_type, int dev, int pos, int bytes, int proc_nr, void* buf)
{
    MESSAGE driver_msg;

    driver_msg.type = io_type;
    driver_msg.DEVICE = MINOR(dev);
    driver_msg.POSITION = pos;
    driver_msg.BUF = buf;
    driver_msg.CNT = bytes;
    driver_msg.PROC_NR = proc_nr;
 /*   
    int n = (int)buf;
    printf("&n = %x\n", n);

    printf("byte = %d\n", bytes);
    printf("type = %d, DEVICE= %d pos= %d, buf = 0x%x, cnt= %x proc= %d\n",
            io_type, MINOR(dev), pos, buf,  bytes, proc_nr);
    printf("byte = %d\n", bytes);
    printf("proc=0x%x\n", proc_nr);
    printf("buf = %d", buf);*/
    assert(dd_map[MAJOR(dev)].driver_nr != INVALID_DRIVER);
//while(1){};
    send_recv(BOTH, dd_map[MAJOR(dev)].driver_nr, &driver_msg);
   // printf("hello\n");

    u8 x = 1;

//    test(1, 2, 3, 4, 5, &x);
    return 0;
}

int test(int x, int y, int z, int n, int m, void* l)
{
    printf("x%d y%d z%d n%d, m%d, l%x\n", x, y, z, n, m, l);
    return 0;
}





