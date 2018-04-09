#ifndef _ORANGES_HD_H_
#define _ORANGES_HD_H_


//I/P Ports used by hard disk controllers 
//slave disk not supported yet, all master registers below

//command block registers
// MACRO   PORT    DESCRIPTION    INPUT/OUTPUT
#define REG_DATA  0x1F0   //data    I/O 
#define REG_FEATURES   0x1F1      //Features   O 
#define REG_ERROR   REG_FEATURES  //Error     I 
/*
 * The contents of this register are valid only when the error bit (ERR) in the 
 * status register is set, except at drive power-up or at the  completion of the 
 * drive's internal diagnostics, when the register contains a status code.
 * when the error bit(ERR) is set, Error register bits are interpreted as such:
 * 0 -- AMNF: Data adress mark not found after correct id field found
 * 1 -- TK0NF: Track 0 not found during execution of recalibrate command
 * 2 -- ABRT: Command aborted due to drive status error or invalid command 
 * 3 --     : not used 
 * 4 -- IDNF: requested sector's ID field not found 
 * 5 --     : not used 
 * 6 -- UNC : uncorrectable data error encountered 
 * 7 -- BRK : bad block mark detected in the requested sector's ID field.
 */ 

#define REG_NSECTOR   0x1F2    //Sector count   I/O 
#define REG_LBA_LOW   0x1F3    //Sector Number / LBA Bits 0-7  I/O 
#define REG_LBA_MID   0x1F4    //Cylinder Low / LBA Bits 8-15   I/O 
#define REG_LBA_HIGH  0x1F5    // Cylinder High / LBA  bit 16-23    I/O 
#define REG_DEVICE    0x1F6    //Drive | head | LBA bits 24-27   I/O 
/*
 * 0 -- HS0 \
 * 1 -- HS1 |If L = 0, Head Select, These four bits select the head number. HS0 is the least significant
 * 2 -- HS2 | If L=1, HS0 to HS3 contain bit 24-27 of the LBA 
 * 3 -- HS3 / 
 * 4 -- DRV Drive, when DRV=0, drive 0(master) is selected
 *                 when DRV = 1, drvie 1(slave) is selected
 * 5 -- 1
 * 6 -- L -- LBA mode. when L=0, addressing is by 'CHS' mode 
 * 7 -- 1 --           when L=1, addressing is by 'LBA' mode 
 */ 

#define REG_STATUS  0x1F7  // status    I 
 /*
  * 0 -- ERR : error, an error occurred
  * 1 -- obsolete
  * 2 -- obsolete 
  * 3 -- DRQ : Data Request (ready to transfer data)
  * 4 -- # :  command dependent.(formerly DSC bit )
  * 5 --DF/SE : Device fault / Stream error 
  * 6 -- DRDY : drive ready
  * 7 -- DSY  : busy if busy=1, no other bits in the register are valid.
  */ 

#define STATUS_BSY 0x80 
#define STATUS_DRDY   0x40 
#define STATUS_DFSE   0x20 
#define STATUS_DSC    0x10 
#define STATUS_DRQ    0x08 
#define STATUS_CORR   0x04 
#define STATUS_IDX    0x02 
#define STATUS_ERR    0x01 

#define REG_CMD   REG_STATUS // command    O 
/*
 * +----------------+---------------------------+---------------------+
 * | Command code   | Command Description       | Parameters Used     
 * |                |                           | PC SC SN CY DH      |
 * +----------------+---------------------------+---------------------+
 * |   ECh          | Identify Drive            |              D      |
 * |   91h          | Initialize Drive Parameter|    V         V      |
 * |   20h          | Read sectors with Retry   |    V  V   V  V      |
 * |   E8h          | Writer Buffer             |              D      |
 * +----------------+---------------------------+---------------------+
 *
 * KEY FOR SYMBOLS IN THE TABLE:
 * ===============================================
 * PC  Register 1F1: Writer precompensation  @ These commands are optional and 
 *                  may not be supported by some drivers 
 * SC Register 1F2: Sector Count  D only Drive Parameter is valid. Head Parameter is ignored
 * SN Register 1F3: sector Number D+ Both drives execute the command regardless of the Drive parameter.
 * CY Register 1F4+1F5: Cylinder low + high V Indicates that the register contains a valid parameter. 
 * DH Register 1F6: Drive / Head 
 */ 

/* Control Block Registers */ 
#define REG_DEV_CTRL 0x3F6   //Device Control     O 
/*
 * 0 -- 
 * 1 -- IEN Interrupt Enable, IEN == 0, and the drive is selected. drive interrupts to the host will be enabled; IEN==1, or drive is not selected, drive interrupts to the host will be disable
 * 2 -- SRST Software Reset. The drive is held reset when RST=1, Setting RST=0 re-enables the drive; the host must be set RST=0 and wait for at least 5 microseconds before setting RST=0, to ensure that the drive recognizes the reset.
 * 3
 * 4
 * 5
 * 6
 * 7 -- HOB  high order byte. defined by 48-bit address feature set.
 */ 

#define REG_ALT_STATUS  REG_DEV_CTRL   //Alternate  status I 
/*
 * This register contains the same information as the status register.
 * The only difference is that reading this register does not imply interrupt acknowledge or clear a pending interrupt.
 */ 

#define REG_DRV_ADDR  0x3F7  // Drive Address I 

/*
 * Definitions
 */ 
#define HD_TIMEOUT 10000 //in millisec
#define PARTITION_TABLE_OFFSET  0x1BE 
#define ATA_IDENTIFY   0xEC 
#define ATA_READ   0x20 
#define ATA_WRITE   0x30 
//for DEVICE register 
#define MAKE_DEVICE_REG(lba, drv, lba_highest) (((lba) << 6) | \
                                               ((drv) << 4)  | \
                                               (lba_highest & 0xF) | 0xA0)



struct hd_cmd
{
    u8 features;
    u8 count;
    u8 lba_low;
    u8 lba_mid;
    u8 lba_high;
    u8 device;
    u8 command;
};



#endif
