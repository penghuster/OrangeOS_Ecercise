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

#include "hd.h"

PRIVATE void mkfs();
PRIVATE void init_fs();
PRIVATE void read_super_block(int dev);
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

    while(1)
    {
        printf("fs---========\n");
        send_recv(RECEIVE, ANY, &fs_msg);
        printf("fs -------------\n");

        int src = fs_msg.source;
        pcaller = &proc_table[src];

        switch(fs_msg.type)
        {
        case OPEN:
            fs_msg.FD = do_open();
            break;
        case CLOSE:
            fs_msg.RETVAL = do_close();
            break;
        case READ:
        case WRITE:
            fs_msg.CNT = do_rdwt();
            break;
        case UNLINK:
            fs_msg.RETVAL = do_unlink();
            break;
        case RESUME_PROC:
            src = fs_msg.PROC_NR;
            printf("---%d------\n", src);
            break;
        case SUSPEND_PROC:
            printf("fs---4\n");
            break;
        }

#ifndef ENABLE_DISK_LOG
        char* msg_name[128];
        msg_name[OPEN] = "OPEN";
        msg_name[CLOSE] = "CLOSE";
        msg_name[READ] = "READ";
        msg_name[WRITE] = "LSEEK";
        msg_name[UNLINK] = "UNLINK";

        switch(fs_msg.type)
        {
            case OPEN:
            case CLOSE:
            case READ:
            case WRITE:
            case UNLINK:
                dump_fd_graph("%s just finished.", msg_name[fs_msg.type]);
                break;
            case DISK_LOG:
                break;
            default:
                assert(0);
        }
#endif

        if(fs_msg.type != SUSPEND_PROC)
        {
    printf("????????????????\n");
            fs_msg.type = SYSCALL_RET;
            send_recv(SEND, src, &fs_msg);
        }
    }
    //open the device: hard disk
    //MESSAGE driver_msg;
    //driver_msg.type = DEV_OPEN;
    //driver_msg.DEVICE = MINOR(ROOT_DEV);
    //assert(dd_map[MAJOR(ROOT_DEV)].driver_nr != INVALID_DRIVER);
    //send_recv(BOTH, dd_map[MAJOR(ROOT_DEV)].driver_nr, &driver_msg);

    spin("FS");
}

/**
 * do_rdwt 
 *
 * Read/Write file and return byte count read/written
 *
 * Sector map is not needed to update, since the sectors for the file 
 * have been allocated and the bits are set when the file was created 
 *
 * @return How many bytes have been read/written 
 *
 */ 
PUBLIC int do_rdwt()
{
    int fd = fs_msg.FD; 
    void *buf = fs_msg.BUF;
    int len = fs_msg.CNT;

    int src = fs_msg.source; //caller proc_nr 

    assert((pcaller->filp[fd] >= &f_desc_table[0]) && 
            (pcaller->filp[fd] < &f_desc_table[NR_FILE_DESC]));
    if(!(pcaller->filp[fd]->fd_mode & O_RDWR))
        return -1;

    int pos = pcaller->filp[fd]->fd_pos;

    struct inode *pin = pcaller->filp[fd]->fd_inode;

    assert(pin >= &inode_table[0] && pin < &inode_table[NR_INODE]);

    int imode = pin->i_mode & I_TYPE_MASK;
    if(imode == I_CHAR_SPECIAL)
    {
        int t = fs_msg.type == READ ? DEV_READ : DEV_WRITE;
        fs_msg.type = t;

        int dev = pin->i_start_sect;
        assert(MAJOR(dev) == 4);

        fs_msg.DEVICE = MINOR(dev);
        fs_msg.BUF = buf;
        fs_msg.CNT = len;
        fs_msg.PROC_NR = src;
        assert(dd_map[MAJOR(dev)].driver_nr != INVALID_DRIVER);
        printf("do_rdwt---2\n");
        send_recv(BOTH, dd_map[MAJOR(dev)].driver_nr, &fs_msg);
        printf("fs ---+++++++\n");
        assert(fs_msg.CNT == len);

        return fs_msg.CNT;
    }
    else
    {
        assert(pin->i_mode == I_REGULAR || pin->i_mode == I_DIRECTORY);
        assert(fs_msg.type == READ || fs_msg.type == WRITE);

        int pos_end;
        if(fs_msg.type == READ)
            pos_end = min(pos+len, pin->i_size);
        else
            pos_end = min(pos+len, pin->i_nr_sects * SECTOR_SIZE);

        int off = pos % SECTOR_SIZE;
        int rw_sect_min = pin->i_start_sect + (pos >> SECTOR_SIZE_SHIFT);
        int rw_sect_max = pin->i_start_sect + (pos_end >> SECTOR_SIZE_SHIFT);

        int chunk = min(rw_sect_max - rw_sect_min + 1, FSBUF_SIZE >> SECTOR_SIZE_SHIFT);

        int bytes_rw  = 0;
        int byte_left = len;
        int i;
        for(i = rw_sect_min; i <= rw_sect_max; i += chunk)
        {
            int bytes = min(byte_left, chunk * SECTOR_SIZE - off);
            rw_sector(DEV_READ, pin->i_dev, i * SECTOR_SIZE,
                    chunk * SECTOR_SIZE, TASK_FS, fsbuf);

            if(fs_msg.type == READ)
            {
                phys_copy((void*)va2la(src, buf + bytes_rw),
                        (void*)va2la(TASK_FS, fsbuf + off),
                        bytes);
            }
            else
            {
                phys_copy((void*)va2la(TASK_FS, fsbuf + off),
                        (void*)va2la(src, buf + bytes_rw),
                        bytes);
                rw_sector(DEV_WRITE, pin->i_dev, i * SECTOR_SIZE,
                        chunk * SECTOR_SIZE, TASK_FS, fsbuf);
            }

            off = 0;
            bytes_rw += bytes;
            pcaller->filp[fd]->fd_pos += bytes;
            byte_left -= bytes;
        }

        if(pcaller->filp[fd]->fd_pos > pin->i_size)
        {
            pin->i_size = pcaller->filp[fd]->fd_pos;
            sync_inode(pin);
        }
        return bytes_rw;
    }
}

/*
 * init_fs
 *
 * <Ring 1> The main loop of TASK_FS 
 */ 
PRIVATE void init_fs()
{
    int i;

    //f_desc_talbe[] 
    for(i = 0; i < NR_FILE_DESC; i++)
        memset(&f_desc_table[i], 0, sizeof(struct file_desc));

    for(i = 0; i < NR_INODE; i++)
        memset(&inode_table[i], 0, sizeof(struct inode));


    //super_block[]
    struct super_block *sb = super_block;

    for(; sb < &super_block[NR_SUPER_BLOCK]; sb++)
    {
    if(sb)
    {
       sb->sb_dev = 0;// NO_DEV;
    printf("hello");
    }
    }

    //open the device: hard disk 
    MESSAGE driver_msg;
    driver_msg.type = DEV_OPEN;
    driver_msg.DEVICE = MINOR(ROOT_DEV);
    assert(dd_map[MAJOR(ROOT_DEV)].driver_nr != INVALID_DRIVER);
    send_recv(BOTH, dd_map[MAJOR(ROOT_DEV)].driver_nr, &driver_msg);

    mkfs();

    //load super block of ROOT 
    read_super_block(ROOT_DEV);

    sb = get_super_block(ROOT_DEV);
    assert(sb->magic == MAGIC_V1);

    root_inode = get_inode(ROOT_DEV, ROOT_INODE);
    
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
/*    
    printl("dev base:0x%x00, sb:0x%x00, imap:0x%x00, smap:0x%x00\n"
            "         inodes:0x%x00, 1st_sector:0x%x00\n",
            geo.base * 2,
            (geo.base + 1) * 2,
            (geo.base + 1 + 1) * 2,
            (geo.base + 1 + 1 + sb.nr_imap_sects) * 2,
            (geo.base + 1 + 1 + sb.nr_imap_sects + sb.nr_smap_sects) * 2,
            (geo.base + sb.n_1st_sect) * 2); //为什么都要乘以2 
//while(1){}
*/


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




/*
 * get_inode
 *
 * <Ring 1> get the inode ptr of given inode_nr, a cache ---inode_table[]---
 * is maintained to make things faster. If the inode requested is already there.
 * just return it. otherwise the inode will be read from the disk
 *
 * @param dev Device_nr 
 * @param num i-node nr 
 *
 * @return The inode ptr requested
 */ 
PUBLIC struct inode *get_inode(int dev, int num)
{
    if(0 == num)
        return (struct inode *)0;

    struct inode *p;
    struct inode *q = 0;

    for(p = &inode_table[0]; p <= &inode_table[NR_INODE - 1]; p++)
    {
        //not a free slot
        if(p->i_cnt)
        {
            //this is the inode we want
            if((p->i_dev == dev) && (p->i_num == num))
            {
                p->i_cnt++;
                return p;
            }

        }
        else //a free slot
        {
            if(!q) //a hasn't been assigned yet
                q = p; // q is the 1st free slot 
        }
    }

    if(!q)
        panic("the inode table is full.");

    q->i_dev = dev;
    q->i_num = num;
    q->i_cnt = 1;

    struct super_block *sb = get_super_block(dev);
    int blk_nr = 1 + 1 + sb->nr_imap_sects + sb->nr_smap_sects
        + ((num - 1) / (SECTOR_SIZE / INODE_SIZE)); //此处num - 1是由于imap的第1个bit为reserved; 
    RD_SECT(dev, blk_nr);
    
    struct inode *pinode = (struct inode*)((u8*)fsbuf + 
            ((num - 1) % (SECTOR_SIZE / INODE_SIZE)) * INODE_SIZE);
    q->i_mode = pinode->i_mode;
    q->i_size = pinode->i_size;
    q->i_start_sect = pinode->i_start_sect;
    q->i_nr_sects = pinode->i_nr_sects;
    q->i_nr_sects = pinode->i_nr_sects;
    
    return q;
}

/*
 * put_inode 
 *
 * Decrease the reference nr of  a slot in inode_table[]. when the nr reaches 
 * zero, it means the inode is not used any more and can be overwritten by a new inode 
 *
 * @param pinode I-node ptr 
 */ 
PUBLIC void put_inode(struct inode *pinode)
{
    assert(pinode->i_cnt > 0);
    pinode->i_cnt--;
}

/*
 * sync_inode 
 *
 * <Ring 1>Write the inode back to the disk. commonly invoked as soon as 
 * the inode is changed.
 *
 * @param p I-node ptr 
 */ 
PUBLIC void sync_inode(struct inode *p)
{
    struct inode *pinode;
    struct super_block *sb = get_super_block(p->i_dev);
    int blk_nr = 1 + 1 + sb->nr_imap_sects + sb->nr_smap_sects +
        ((p->i_num - 1) / (SECTOR_SIZE / INODE_SIZE));
    RD_SECT(p->i_dev, blk_nr);
    pinode = (struct inode*)((u8*)fsbuf +
            (((p->i_num - 1) % (SECTOR_SIZE / INODE_SIZE)) * INODE_SIZE));
    pinode->i_mode = p->i_mode;
    pinode->i_size = p->i_size;
    pinode->i_start_sect = p->i_start_sect;
    pinode->i_nr_sects = p->i_nr_sects;
    WR_SECT(p->i_dev, blk_nr);
}


/*
 * read_super_block
 *
 * <Ring 1>Read super block from the given device then write it into 
 * a free super_block[] slot
 *
 * @param dev From which device the super_block comes 
 */ 
PRIVATE void read_super_block(int dev)
{
    int i;
    MESSAGE driver_msg;

    driver_msg.type = DEV_READ;
    driver_msg.DEVICE = MINOR(dev);
    driver_msg.POSITION = SECTOR_SIZE * 1;
    driver_msg.BUF = fsbuf;
    driver_msg.CNT = SECTOR_SIZE;
    driver_msg.PROC_NR = TASK_FS;
    assert(dd_map[MAJOR(dev)].driver_nr != INVALID_DRIVER);
    send_recv(BOTH, dd_map[MAJOR(dev)].driver_nr, &driver_msg);

    //find a free slot in super_block[]
    for(i = 0; i < NR_SUPER_BLOCK; i++)
        if(super_block[i].sb_dev == NO_DEV)
            break;

    if(i == NR_SUPER_BLOCK)
        panic("super block slots used up");

    assert(i == 0);

    struct super_block *psb = (struct super_block *)fsbuf;

    super_block[i] = *psb;
    super_block[i].sb_dev = dev;
}

/*
 * get_super_block 
 *
 * <Ring 1> Get the super block from super_block[]
 *
 * @param dev Device nr 
 *
 * @return Super block ptr 
 */ 
PUBLIC struct super_block* get_super_block(int dev)
{
    struct super_block* sb = super_block;
    for(; sb <= &super_block[NR_SUPER_BLOCK - 1]; sb++)
        if(sb->sb_dev == dev)
            return sb;

    panic("super block of device %d not found.\n", dev);
    return 0;
}
