#include "type.h"
#include "const.h"
#include "protect.h"
#include "string.h"
#include "proc.h"
#include "tty.h"
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

PRIVATE u8 hd_status;
PRIVATE u8 hdbuf[SECTOR_SIZE * 2];


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
            hd_identify(0);
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
   //Get the number of drivers from the BIOS data area 
   u8 *pNrDrives = (u8*)(0x475);
   printf("NrDrives: %d.\n", *pNrDrives);
   assert(*pNrDrives);

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
    printf("here.\n");  while(1){}
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
    //printl("HD size: %d MB\n", 512);
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
