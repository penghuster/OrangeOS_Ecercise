#ifndef ORANGESCONST_H_
#define ORANGESCONST_H_

//函数类型
#define PUBLIC 
#define PRIVATE static 
#define EXTERN extern 

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

#define NR_SYS_CALL 1


//8253/8254 PIT Programmable interval Timer
#define TIMER0      0x40  //I/O port for timer chinnal 0
#define TIMER_MODE  0x43  //00-11-010-0; LSB then MSB
#define RATE_GENERATOR  0x34
#define TIMER_FREQ  1193182L  
#define HZ  100   //clock freq 

#endif 