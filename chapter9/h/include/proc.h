#include "type.h"

typedef struct s_stackframe
{
    //pushed by save()
    u32 gs;
    u32 fs;
    u32 es;
    u32 ds;
    u32 edi;
    u32 esi;
    u32 ebp;
    u32 kernel_esp; //popad will ignore it 
    u32 ebx;
    u32 edx;
    u32 ecx;
    u32 eax;

    u32 retaddr; //return addr for kernel.asm::save()
    
    //pushed by cpu during interrupt
    u32 eip;
    u32 cs;
    u32 eflags;
    u32 esp;
    u32 ss;
}STACK_FRAME;


typedef struct s_proc
{
    STACK_FRAME regs;  //process registers saved in stack frame
    u16 ldt_sel;  //gdt selector giving ldt base and limit
    DESCRIPTOR ldts[LDT_SIZE];  //local descriptor for code and data 
    int ticks;//remained ticks
    int priority;
    u32 pid;  //process id passed in from MM
    char p_name[16]; //name of the process
    
    int p_flags;  //process flags A proc is runnable if p_flags == 0
    MESSAGE* p_msg;
    int p_recvfrom;
    int p_sendto;

    int has_int_msg; //nonzero if an INTERRUPT occurred when the task is not 
    //ready to deal with it.
    
    struct s_proc *q_sending; //queue of procs sending messages to this proc 
    struct s_proc * next_sending; //next procs in the sending queue(q_sending)
    int nr_tty;

    struct file_desc *filp[NR_FILES];
}PROCESS;

typedef struct s_task
{
    task_f initial_eip;
    int stacksize;
    char name[32];
}TASK;

#define proc2pid(x) ((x) - proc_table)

#define NR_TASKS   4
#define NR_PROCS   3
#define TASK_SIZE_TESTA  0x8000
#define STACK_SIZE_TESTB 0x8000
#define STACK_SIZE_TESTC 0x8000
#define STACK_SIZE_TTY   0x8000
#define STACK_SIZE_SYS   0x8000
#define STACK_SIZE_HD    0x8000 
#define STACK_SIZE_FS    0x8000
#define STACK_SIZE_TOTAL (TASK_SIZE_TESTA + STACK_SIZE_TESTB + STACK_SIZE_TESTC \
        + STACK_SIZE_TTY + STACK_SIZE_SYS + STACK_SIZE_HD + STACK_SIZE_FS)
