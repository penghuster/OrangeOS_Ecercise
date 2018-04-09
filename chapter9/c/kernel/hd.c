#include "type.h"
#include "const.h"
#include "protect.h"
#include "string.h"
#include "proc.h"
#include "tty.h"
#include "fs.h"
#include "global.h"
#include "keyboard.h"
#include "proto.h"
#include "hd.h"

PRIVATE void init_hd();
PRIVATE void hd_cmd_out(struct hd_cmd* cmd);
PRIVATE int waitfor(int mask, int val, int timeout);
PRIVATE void interrupt_wait();
PRIVATE void hd_identify(int drive);
PRIVATE void print_identify_info(u16* hdinfo);
PRIVATE void get_part_table(int drive, int sect_nr, struct part_ent* entry);
PRIVATE void print_hdinfo(struct hd_info* hdi);
PRIVATE void partition(int device, int style);
PRIVATE void hd_open(int device);
PRIVATE void hd_close(int device);
PRIVATE void hd_rdwt(MESSAGE *p);
PRIVATE void hd_ioctl(MESSAGE *p);

PRIVATE u8 hd_status;
PRIVATE u8 hdbuf[SECTOR_SIZE * 2];
PRIVATE struct hd_info hd_info[1];

//根据来访问的次设备号，计算出主设备号
//若为主分区，则除以 NR_PRIM_PER_DRIVE(5)来计算主设备号
//若为逻辑分区,则除以NR_SUB_PER_DRIVE(16*4)来计算主设备号
#define DRV_OF_DEV(dev)  (dev <= MAX_PRIM ? \
        dev / NR_PRIM_PER_DRIVE : \
        (dev - MINOR_hd1a) / NR_SUB_PER_DRIVE)


/*
 * task_hd 
 *
 * Main loop of HD driver
 */ 
PUBLIC void task_hd()
{
    MESSAGE msg;

    init_hd();


    while(1)
    {
        send_recv(RECEIVE, ANY, &msg);
        int src = msg.source;

        switch(msg.type)
        {
        case DEV_OPEN:
            //hd_identify(0);
            hd_open(msg.DEVICE);
            break;
        case DEV_CLOSE:
            hd_close(msg.DEVICE);
            break;
        case DEV_READ:
        case DEV_WRITE:
            hd_rdwt(&msg);
            break;
        case DEV_IOCTL:
            hd_ioctl(&msg);
            break;
        default:
            dump_msg("HD driver:: unknown msg", &msg);
            spin("FS::main loop (invalid msg.type)");
            break;
        }
        send_recv(SEND, src, &msg);
    }
}

/*
 * init_hd
 *<Ring 1> check hard driver, set IRQ handler, enable IRQ and initalize data structures
 */ 
PRIVATE void init_hd()
{
    int i;
   //Get the number of drivers from the BIOS data area 
   u8 *pNrDrives = (u8*)(0x475);
   printf("NrDrives: %d.\n", *pNrDrives);
   assert(*pNrDrives);

   for(i = 0; i < (sizeof(hd_info) / sizeof(hd_info[0])); i++)
   {
       memset(&hd_info[i], 0, sizeof(hd_info[0]));
       hd_info[0].open_cnt = 0;
   }

   put_irq_handler(AT_WINI_IRQ, hd_handler);
   enable_irq(CASCADE_IRQ);
   enable_irq(AT_WINI_IRQ);
}


/*
 * hd_identify
 *
 * <Ring 1> Get the disk information 
 *
 * @param drive  drive Nr. 
 */ 
PRIVATE void hd_identify(int drive)
{
    struct hd_cmd cmd;
    cmd.device = MAKE_DEVICE_REG(0, drive, 0);
    cmd.command = ATA_IDENTIFY;
    hd_cmd_out(&cmd);
    interrupt_wait();
    port_read(REG_DATA, hdbuf, SECTOR_SIZE);

    print_identify_info((u16*)hdbuf);
    //printf("here.\n");  while(1){}
}

/*
 * print_identify_info
 *
 * <Ring 1> print the hdinfo retrieved via ATA_IDENTIFY command
 *
 * @param hdinfo the buffer read from the disk i/o port 
 */ 
PRIVATE void print_identify_info(u16* hdinfo)
{
    int i, k;
    char s[64];

    struct iden_info_assii
    {
        int idx;
        int len;
        char *desc;
    } iinfo[] = { {10, 20, "HD SN"}, /*serial number in ascii*/
                  {27, 40, "HD model"} /*Model number in ascii*/};

    for(k = 0; k < sizeof(iinfo)/sizeof(iinfo[0]); k++)
    {
        char *p = (char*)&hdinfo[iinfo[k].idx];
        for(i = 0; i < iinfo[k].len/2; i++)
        {
            //此处需要注意hdinfo是 u16* 类型，
            s[i*2+1] = *p++;
            s[i*2] = *p++;
        }
        s[i*2] = 0;
        printl("%s: %s\n", iinfo[k].desc, s);
    }

    int capablities = hdinfo[49];
    printl("LBA supported: %s\n", (capablities &0x0200) ? "Yes" : "No");

    int sectors = ((int)hdinfo[61] << 16) + hdinfo[60];
    printl("HD size: %d MB\n", sectors * 512 / 1000000);
}

/*
 * hd_cmd_out
 *
 * <Ring 1> Output a command to HD controller.
 *
 * @param cmd The command struct ptr 
 */ 
PRIVATE void hd_cmd_out(struct hd_cmd* cmd)
{
    /*
     * for all commands the host must first check if BSY = 1,
     * and should proceed no further unless and until BSY = 0
     */ 
    if(!waitfor(STATUS_BSY, 0, HD_TIMEOUT))
    {
        panic("hd error");
    }

    //activate the interrupt Enable(nIEN) bit
    out_byte(REG_DEV_CTRL, 0);
    //load required parameters in the command block registers 
    out_byte(REG_FEATURES, cmd->features);
    out_byte(REG_NSECTOR, cmd->count);
    out_byte(REG_LBA_LOW, cmd->lba_low);
    out_byte(REG_LBA_MID, cmd->lba_mid);
    out_byte(REG_LBA_HIGH, cmd->lba_high);
    out_byte(REG_DEVICE, cmd->device);
    //write the command code to the command 
    out_byte(REG_CMD, cmd->command);
}

/*
 * interrupt_wait 
 *
 * <Ring 1> Wait until a disk interrupt occurs 
 */ 
PRIVATE void interrupt_wait()
{
    MESSAGE msg;
    send_recv(RECEIVE, INTERRUPT, &msg);
}

/*
 * waitfor 
 *
 * <Ring 1> Wait for a certain status. 
 *
 * @param mask status mask 
 * @param val Required status 
 * @param timeout Timeout in milliseconds 
 *
 * @return One if success, zero if timeout. 
 */ 
PRIVATE int waitfor(int mask, int val, int timeout)
{
    int t = get_ticks();

    while(((get_ticks() - t) * 1000 / HZ) < timeout)
    {
        if((in_byte(REG_STATUS) & mask) ==  val)
            return 1;

    }
    return 0;
}


/*
 * hd_handler
 *
 * <Ring 0> Interrupt handler 
 *
 * @param irq IRQ nr of the disk interrupt.
 */ 
PUBLIC void hd_handler(int irq)
{
    /*
     * Interrupts are cleared when the host:
     * --reads the status register 
     * --issues a reset,
     * --writes to the command register. 
     */ 
    hd_status = in_byte(REG_STATUS);
    inform_int(TASK_HD);
}


/*
 * <Ring 1> This routine handles DEV_OPEN message. It identify the drive 
 * of the given device and read the partition table of the drive if it 
 * has not been read.
 *
 * @param device The device to be opened.
 */ 
PRIVATE void hd_open(int device)
{
    int drive = DRV_OF_DEV(device);
     assert(drive == 0); //only one drive 

     hd_identify(drive);

     if(hd_info[drive].open_cnt++ == 0)
     {
         partition(drive * (NR_PART_PER_DRIVE + 1), P_PRIMARY);
         print_hdinfo(&hd_info[drive]);
     }
}


/*
 * hd_close 
 *
 * <Ring 1> This routine handlers DEV_CLOSE message
 *
 * @param device The device to be opened 
 */ 
PRIVATE void hd_close(int device)
{
    int drive = DRV_OF_DEV(device);
    assert(0 == drive);

    //此处是针对硬件设备的操作没有句柄的概念，只存在计数
    hd_info[drive].open_cnt--;
}

/*
 * hd_rdwt
 *
 * <Ring 1> This routine handles DEV_READ and DEV_WRITE message
 *
 * @param p Messag ptr.
 */ 
PRIVATE void hd_rdwt(MESSAGE *p)
{
    int drive = DRV_OF_DEV(p->DEVICE);

    u64 pos = p->POSITION;//此位置记录的要读内容在磁盘中的当前分区字节序号
    assert((pos >> SECTOR_SIZE_SHIFT) < (1 << 31));//misunderstand为何要做此判断？？？？
    //We only allow to R/W from a sectot boundary 
    assert((pos & 0x1FF));//确保按照一个扇区一个扇区读取

    //此处计算出的扇区序号是在当前分区中的扇区序号
    u32 sect_nr = (u32)(pos >> SECTOR_SIZE_SHIFT);

    //DEVICE为次设备号，即分区编号，若为逻辑分区，由于逻辑分区编号从 MINOR_HD1A(16)开始
    //故需要减去基数
    //由于硬盘按照驱动来记录 hd_info， 故当前磁盘上的逻辑分区索引
    //logidx还需要整除 NR_SUB_PER_DRIVE, 
    int logidx = (p->DEVICE - MINOR_hd1a) % NR_SUB_PER_DRIVE;

    //判断次设备号是主分区还是逻辑分区，找到当前分区的开始扇区
    sect_nr += p->DEVICE < MAX_PRIM ?
        hd_info[drive].primary[p->DEVICE].base :
        hd_info[drive].logical[logidx].base;

    struct hd_cmd cmd;
    cmd.features = 0;
    cmd.count = (p->CNT + SECTOR_SIZE - 1) / SECTOR_SIZE;
    cmd.lba_low = sect_nr & 0xFF;
    cmd.lba_mid = (sect_nr >> 8) & 0xFF;
    cmd.lba_high = (sect_nr >> 16) & 0xFF;
    cmd.device = MAKE_DEVICE_REG(1, drive, (sect_nr >> 24) & 0xF);
    cmd.command = (p->type == DEV_READ) ? ATA_READ : ATA_WRITE;
    hd_cmd_out(&cmd);

    int bytes_left = p->CNT;
    void *la = (void*)va2la(p->PROC_NR, p->BUF);

    while(bytes_left)
    {
        int bytes = min(SECTOR_SIZE, bytes_left);
        if(DEV_READ == p->type)
        {
            interrupt_wait();
            port_read(REG_DATA, hdbuf, SECTOR_SIZE);
            phys_copy(la, (void*)va2la(TASK_HD, hdbuf), bytes);
        }
        else
        {
            if(!waitfor(STATUS_DRQ, STATUS_DRQ, HD_TIMEOUT))
            {
                panic("hd writing error.");
            }
            port_write(REG_DATA, la, bytes);
            interrupt_wait();
        }
        bytes_left -= SECTOR_SIZE;
        la += SECTOR_SIZE;
    }
}

/*
 * hd_ioctl
 *
 * <Ring 1> This routine handles the DEV_IOCTL message
 *
 * @param p Ptr to the MESSAGE
 */ 
PRIVATE void hd_ioctl(MESSAGE *p)
{
    int device = p->DEVICE;
    int drive = DRV_OF_DEV(device);

    struct hd_info *hdi = &hd_info[drive];

    if(p->REQUEST == DIOCTL_GET_GEO)
    {
        void *dst = va2la(p->PROC_NR, p->BUF);
        void *src = va2la(TASK_HD,
                device < MAX_PRIM ?
                &hdi->primary[device] :
                &hdi->logical[(device - MINOR_hd1a) % NR_SUB_PER_DRIVE]);
        phys_copy(dst, src, sizeof(struct part_info));
    }
    else 
    {
        assert(0);
    }
}

/*
 * get_part_table 
 *
 * <Ring 1> Get a partition table of a drive 
 *
 * @param drive Drive_nr (0 for the 1st disk, 1 for the 2nd, ...)
 * @param sect_nr The sector at which the partition table is located 
 * @param entry Ptr to part_ent struct 
 */ 
PRIVATE void get_part_table(int drive, int sect_nr, struct part_ent* entry)
{
    struct hd_cmd cmd;
    cmd.features = 0;
    cmd.count = 1;
    cmd.lba_low = sect_nr & 0xFF;
    cmd.lba_mid = (sect_nr >> 8) & 0xFF;
    cmd.lba_high = (sect_nr >> 16) & 0xFF;
    cmd.device = MAKE_DEVICE_REG(1, //LBA model 
            drive,
            (sect_nr >> 24) & 0xF);
    cmd.command = ATA_READ;
    hd_cmd_out(&cmd);
    interrupt_wait(); //此时若产生其它中断了将如何处理呢？

    //读取指定扇区(512 bytes)
    port_read(REG_DATA, hdbuf, SECTOR_SIZE);
    memcpy(entry, 
            hdbuf + PARTITION_TABLE_OFFSET, //在指定扇区的 0x1be 以后是分区表结构体
            sizeof(struct part_ent) * NR_PART_PER_DRIVE);
}

/*
 * partition 
 *
 * <Ring 1> This routine is called when a device is opened, it reads the 
 * partition table(s) and fills the hd_info struct.
 *
 * @param device  Device nr. 
 * @param style P-PRIMARY or P-EXTENDED
 */ 
PRIVATE void partition(int device, int style)
{
    int i;
    int drive = DRV_OF_DEV(device);
    struct hd_info * hdi = &hd_info[drive];
    struct part_ent part_tbl[NR_SUB_PER_DRIVE];
    if(P_PRIMARY == style)
    {
        get_part_table(drive, drive, part_tbl);

        int nr_prim_parts = 0;
        for(i = 0; i < NR_PART_PER_DRIVE; i++)
        {
            if(part_tbl[i].sys_id == NO_PART)
                continue;
            nr_prim_parts++;
            int dev_nr = i + 1;
            hdi->primary[dev_nr].base = part_tbl[i].start_sect;
            hdi->primary[dev_nr].size = part_tbl[i].nr_sects;

            //若存在逻辑分区，则主分区表的最后一项一定是扩展分区
            if(part_tbl[i].sys_id == EXT_PART)
            {
                partition(device + dev_nr, P_EXTENDED);
            }
        }
        assert(nr_prim_parts != 0);
    }
    else if(P_EXTENDED == style)
    {
        int j = device % NR_PRIM_PER_DRIVE;
        //主分区最后一个分区即为扩展分区，扩展分区的开始扇区即为第一个逻辑分区的开始扇区
        int ext_start_sect = hdi->primary[j].base;
        int s = ext_start_sect;
        //第一个逻辑分区的编号，从前j-1个主分区可能存在的逻辑分区的下一个开始
        int nr_1st_sub = (j - 1) * NR_SUB_PER_PART;
        
        printf("j=%d, s= %d, nr_1st_sub=%d\n", j, s, nr_1st_sub);
        //逻辑分区的分区表位于扩展分区的开始位置
        for(i = 0; i < NR_SUB_PER_PART; i++)
        {
            int dev_nr = nr_1st_sub + i; //每个扩展分区最多可能16个逻辑分区
            get_part_table(drive, s, part_tbl);

            //逻辑分区的分区表由一个普通分区表项和一个扩展分区表项组成
            hdi->logical[dev_nr].base = s + part_tbl[0].start_sect;
            hdi->logical[dev_nr].size = part_tbl[0].nr_sects;

            //下一个逻辑分区开始扇区为主分区中扩展分区的开始扇区 ext_start_sect
            //加上当前扩展分区的 起始lba
            s = ext_start_sect + part_tbl[1].start_sect;

            //no more logical partitions 
            //in the extended partition 
            if(part_tbl[1].sys_id == NO_PART)
                break;
        }

    }
    else
    {
        assert(0);
    }
}


/*
 * print_hdinfo
 *
 * <Ring 1> Print disk info 
 *
 * @param hdi ptr to struct hd_info 
 */ 
PRIVATE void print_hdinfo(struct hd_info* hdi)
{
    int i;
    for(i = 0; i < NR_PART_PER_DRIVE + 1; i++)
    {
        printl("%sPART_%d: base %d(0x%x), size %d(0x%x) (in sector)\n",
            i == 0 ? "  " : "      ",
            i,
            hdi->primary[i].base,
            hdi->primary[i].base,
            hdi->primary[i].size,
            hdi->primary[i].size);
    }

    for(i = 0; i < NR_SUB_PER_DRIVE; i++)
    {
        if(hdi->logical[i].size == 0)
            continue;

        printl("          "
                "%d: base %d(0x%x), size %d(0x%x) (in sector)\n",
                i,
                hdi->logical[i].base,
                hdi->logical[i].base,
                hdi->logical[i].size,
                hdi->logical[i].size);
    }
}
