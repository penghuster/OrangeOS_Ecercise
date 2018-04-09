#ifndef ORANGESCONST_H_
#define ORANGESCONST_H_

#define ASSERT
#ifdef ASSERT
void assertion_failure(char *exp, char *file, char *base_file, int line);
#define assert(exp) if (exp); \
    else assertion_failure(#exp, __FILE__, __BASE_FILE__, __LINE__)
#else
#define assert(exp)
#endif


//Color
//e.g. MAKE_COLOR(BLUE, RED)
//  MAKE_COLOR()
#define BLACK 0x0 
#define WHITE 0x7
#define RED   0x4 
#define GREEN   0x2 
#define BLUE    0x1 
#define FLASH   0x80 
#define BRIGHT  0x08 
#define MAKE_COLOR(x, y) ((x<<4) | y)

//函数类型
#define PUBLIC 
#define PRIVATE static 
#define EXTERN extern 

#define STR_DEFAULT_LEN   1024

#define TRUE 1
#define FALSE 0

//GDT 和 IDT 中描述符的个数
#define GDT_SIZE 128
#define IDT_SIZE 256

//最大允许进程数
//#define NR_TASKS 1

//权限
#define PRIVILEGE_KRNL  0
#define PRIVILEGE_TASK  1
#define PRIVILEGE_USER  3

#define RPL_KRNL    SA_RPL0
#define RPL_TASK    SA_RPL1 
#define RPL_USER    SA_RPL3

//8259A interrupt controller ports
#define INT_M_CTL 0x20 //I/O port for interrupt controller <Master>
#define INT_M_CTLMASK 0x21  //setting bits in this port disables ints <master>
#define INT_S_CTL 0xA0   // I/O port for second interrupt controller <slave>
#define INT_S_CTLMASK 0xA1  //setting bits int this port disables ints <slave>

//hardware interrupts
#define NR_IRQ      16  //number of irqs 
#define CLOCK_IRQ   0   
#define KEYBOARD_IRQ   1 
#define CASCADE_IRQ     2  //cascade enable for 2nd AT controller
#define ETHER_IRQ       3  //default ethernet interrupt vector
#define SECONDARY_IRQ  3  //RS232 interrupt vector for port 2
#define RS232_IRQ       4  //RS232 interrupt vector for port 1
#define XT_WINI_IRQ     5  //xt winchester
#define FLOPPY_IRQ      6  // floppy disk 
#define PRINTER_IRQ    7  
#define AT_WINI_IRQ   14   //at winchester

// system call
#define NR_SYS_CALL 3


//8253/8254 PIT Programmable interval Timer
#define TIMER0      0x40  //I/O port for timer chinnal 0
#define TIMER_MODE  0x43  //00-11-010-0; LSB then MSB
#define RATE_GENERATOR  0x34
#define TIMER_FREQ  1193182L  
#define HZ  100   //clock freq 

//AT keyboard 8024 ports 
#define KB_DATA    0x60  
#define KB_CMD      0x64
#define LED_CODE  0xED
#define KB_ACK  0xFA

//VGA
/* VGA */
#define CRTC_ADDR_REG    0x3D4/* CRT Controller Registers - Addr Register */
#define CRTC_DATA_REG    0x3D5/* CRT Controller Registers - Data Register */
#define START_ADDR_H     0xC/* reg index of video mem start addr (MSB) */
#define START_ADDR_L     0xD/* reg index of video mem start addr (LSB) */
#define CURSOR_H         0xE/* reg index of cursor position(MSB) */
#define CURSOR_L         0xF   /* reg index of cursor position (LSB) */
#define V_MEM_BASE      0xB8000/* base of color video memory */
#define V_MEM_SIZE      0x8000/* 32K: B8000H -> BFFFFH */        

//TTY
#define NR_CONSOLES     3

//task 
//注意 TASK_XXX 的定义要与 global.c 中对应
#define INVALID_DRIVER  -20
#define INTERRUPT   -10
#define TASK_TTY     0
#define TASK_SYS    1
#define TASK_HD     2
#define TASK_FS     3
#define TASK_MM     4

#define ANY     (NR_TASKS + NR_PROCS + 10)
#define NO_TASK (NR_TASKS + NR_PROCS + 20)

//ipc 
#define SEND    1
#define RECEIVE 2
#define BOTH   3

//process 
#define SENDING   0x02   // set when proc trying to send 
#define RECEIVING 0x04  // set when proc trying to receive

//magic chars used by 'printx'
#define MAG_CH_PANIC   '\002'
#define MAG_CH_ASSERT   '\003'


enum msgtype
{
    HARD_INT = 1,
    //sys task
    GET_TICKS,

    //message type for drivers
    DEV_OPEN = 1001,
    DEV_CLOSE,
    DEV_READ,
    DEV_WRITE,
    DEV_IOCTL

};

#define RETVAL u.m3.m3i1


#define DIOCTL_GET_GEO   1

//Hard drive 
#define SECTOR_SIZE   512
#define SECTOR_BITS   (SECTOR_SIZE * 8)
#define SECTOR_SIZE_SHIFT   9

//major device numbers(corresponding to kernel/global.c::dd_map[])
#define NO_DEV      0
#define DEV_FLOPPY  1 
#define DEV_CDROM   2
#define DEV_HD      3
#define DEV_CHAR_TTY    4
#define DEV_SCSI        5

//make device number from major and minor numbers
#define MAJOR_SHIFT     8
#define MAKE_DEV(a,b)   ((a << MAJOR_SHIFT) | b)


#endif 
