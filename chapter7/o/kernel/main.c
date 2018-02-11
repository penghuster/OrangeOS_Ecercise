#include "type.h"
#include "const.h"
#include "protect.h"
#include "proc.h"
#include "tty.h"
#include "proto.h"
#include "string.h"
#include "global.h"

//void TestA();
//void TestB();

PUBLIC int kernel_main()
{
    disp_str("---------------\"kernel_main\" begin------\n");

    TASK* p_task = task_table;
    PROCESS* p_proc = proc_table;
    char* p_task_stack = task_stack + STACK_SIZE_TOTAL;

    u16 selector_ldt = SELECTOR_LDT_FIRST;

    u8 privilege;
    u8 rpl;
    int eflags;
    int i;
    for(i = 0; i < NR_TASKS + NR_PROCS; i++)
    {
        if(i < NR_TASKS)
        {
            p_task = task_table + i;
            privilege = PRIVILEGE_TASK;
            rpl = RPL_TASK;
            eflags = 0x1202;
        }
        else
        {
            p_task = user_proc_table + (i - NR_TASKS);
            privilege = PRIVILEGE_USER;
            rpl = RPL_USER;
            eflags = 0x202;
        }
        p_proc->nr_tty = 0;
        strcpy(p_proc->p_name, p_task->name);
        p_proc->pid = i;
        p_proc->ldt_sel = selector_ldt;
        memcpy(&p_proc->ldts[0], &gdt[SELECTOR_KERNEL_CS>>3], sizeof(DESCRIPTOR));
        p_proc->ldts[0].attr1 = DA_C | privilege << 5;  //change the DPL 
        memcpy(&p_proc->ldts[1], &gdt[SELECTOR_KERNEL_DS>>3], sizeof(DESCRIPTOR));
        p_proc->ldts[1].attr1 = DA_DRW | privilege << 5;  //change the DPL 

        //此处指定寄存器为ldt中的段
        p_proc->regs.cs = (0 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
        p_proc->regs.ds = (8 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
        p_proc->regs.es = (8 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
        p_proc->regs.fs = (8 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
        p_proc->regs.ss = (8 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
        p_proc->regs.gs = (SELECTOR_KERNEL_GS & SA_RPL_MASK) | rpl;

        p_proc->regs.eip = (u32)p_task->initial_eip;
        p_proc->regs.esp = (u32)p_task_stack;
        p_proc->regs.eflags = eflags;  //IF=1 IOPL=1 bit 2 is always 1.
        
        p_task_stack -= p_task->stacksize;
        p_proc++;
        p_task++;
        selector_ldt += 1<<3;
    }

    proc_table[0].ticks = proc_table[0].priority = 27;
    proc_table[1].ticks = proc_table[1].priority = 9;
    proc_table[2].ticks = proc_table[2].priority = 3;
    proc_table[3].ticks = proc_table[3].priority = 30;

    proc_table[1].nr_tty = 0;
    proc_table[2].nr_tty = 1;
    proc_table[3].nr_tty = 1;

   // put_irq_handler(CLOCK_IRQ, clock_handler);  //设置时钟中断处理函数
    //enable_irq(CLOCK_IRQ);  //让8259A可以接收时钟中断
    init_clock();
    //init_keyboard();

    p_proc_ready = proc_table;
    disp_str("hello\n");
    k_reenter = 0;
    ticks = 0;
    restart();

    while(1)
    {

    }
}

void TestA()
{
    int i = 0;
    while(1)
    {
        disp_str("A");
        //disp_int(i++);
//        disp_int(get_ticks());
    //    disp_str(".");
        milli_delay(2000);
        //delay(1);
        printf("Ticks:%x", get_ticks());
    }
}

void TestB()
{
    int i = 0x1000;
    while(1)
    {
    //    disp_str("B");
//        disp_int(i++);
  //      disp_str(".");
        //delay(1);
        printf("B");
        milli_delay(2000);
    }
}

void TestC()
{
    int i = 0x2000;
    while(1)
    {
 //       disp_str("C");
 //       disp_int(i++);
//        disp_str(".");
       // delay(1);
        printf("C");
        milli_delay(2000);
    }
}
