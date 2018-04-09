#define GLOBAL_VARIABLES_HERE

#include "type.h"
#include "const.h"
#include "protect.h"
#include "proc.h"
#include "tty.h"
#include "fs.h"
#include "proto.h"
#include "global.h"


EXTERN int disp_pos;
EXTERN u8 gdt_ptr[6];
EXTERN DESCRIPTOR gdt[GDT_SIZE];
EXTERN u8 idt_ptr[6];
EXTERN GATE idt[IDT_SIZE];
EXTERN PROCESS proc_table[NR_TASKS + NR_PROCS];
EXTERN char task_stack[STACK_SIZE_TOTAL];

EXTERN TASK task_table[NR_TASKS] = { {task_tty, STACK_SIZE_TTY, "tty1"},
                                     {task_sys, STACK_SIZE_SYS, "sys task"},
                                     {task_hd, STACK_SIZE_HD, "hd task"},
                                     {task_fs, STACK_SIZE_FS, "fs task"}
                                   };
EXTERN TASK user_proc_table[NR_PROCS] = {{TestA, TASK_SIZE_TESTA, "TestA"},
                                        {TestB, STACK_SIZE_TESTB, "TestB"},
                                        {TestC, STACK_SIZE_TESTC, "TestC"}};
EXTERN irq_handler  irq_table[NR_IRQ];
EXTERN system_call sys_call_table[NR_SYS_CALL] = 
            {sys_get_ticks, sys_printx, sys_sendrec};

EXTERN int ticks;

EXTERN TTY tty_table[NR_CONSOLES];
EXTERN CONSOLE console_table[NR_CONSOLES];

EXTERN int nr_current_console;

EXTERN struct dev_drv_map dd_map[] = {
                                     /*driver nr. major device nr.*/
                                     {INVALID_DRIVER}, /*0 : Unused*/
                                     {INVALID_DRIVER}, /*1 : reserved for floppy driver*/ 
                                     {INVALID_DRIVER}, /*2 : reserved for cdrom driver */
                                     {TASK_HD},
                                     {TASK_TTY}, 
                                     {INVALID_DRIVER}
                                };