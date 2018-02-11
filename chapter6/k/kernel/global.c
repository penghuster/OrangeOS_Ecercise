#define GLOBAL_VARIABLES_HERE

#include "type.h"
#include "const.h"
#include "protect.h"
#include "proto.h"
#include "proc.h"
#include "global.h"


EXTERN int disp_pos;
EXTERN u8 gdt_ptr[6];
EXTERN DESCRIPTOR gdt[GDT_SIZE];
EXTERN u8 idt_ptr[6];
EXTERN GATE idt[IDT_SIZE];
EXTERN PROCESS proc_table[NR_TASKS];
EXTERN char task_stack[STACK_SIZE_TOTAL];
EXTERN TASK task_table[NR_TASKS] = {{TestA, TASK_SIZE_TESTA, "TestA"},
                                    {TestB, STACK_SIZE_TESTB, "TestB"},
                                    {TestC, STACK_SIZE_TESTC, "TestC"}};
EXTERN irq_handler  irq_table[NR_IRQ];
