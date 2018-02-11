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


#endif 
