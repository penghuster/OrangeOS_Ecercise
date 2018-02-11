

#ifdef GLOBAL_VARIABLES_HERE

#undef EXTERN
#define EXTERN

#endif


EXTERN int disp_pos;
EXTERN u8 gdt_ptr[6];
EXTERN DESCRIPTOR gdt[GDT_SIZE];
EXTERN u8 idt_ptr[6];
EXTERN GATE idt[IDT_SIZE];

EXTERN u32 k_reenter;

EXTERN TSS tss;
EXTERN PROCESS* p_proc_ready;

EXTERN PROCESS proc_table[];
EXTERN char     task_stack[];

EXTERN TASK task_table[];

EXTERN irq_handler irq_table[];

EXTERN system_call sys_call_table[];

EXTERN int ticks;
