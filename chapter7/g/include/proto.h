#ifndef ORANGESPROTO_H_
#define ORANGESPROTO_H_


PUBLIC void out_byte(u16 port, u8 value);
PUBLIC u8   in_byte(u16 port);
PUBLIC void disp_str(char *info);
PUBLIC void disp_color_str(char* info, int color);
PUBLIC void init_prot();
PUBLIC void init_8259A();
PUBLIC void disp_int(int num);
PUBLIC void disable_int();
PUBLIC void enable_int();

PUBLIC void clock_handler(int irq);
PUBLIC void put_irq_handler(int irq, irq_handler handler);
PUBLIC void spurious_irq(int irq);
PUBLIC void init_keyboard();
PUBLIC void init_clock();

PUBLIC void in_process(u32 key);

PUBLIC void task_tty();


//与系统调用相关
PUBLIC int sys_get_ticks();

PUBLIC void sys_call();
PUBLIC int get_ticks();


PUBLIC void milli_delay(int milli_sec);

PUBLIC void schedule();

void TestA();
void TestB();
void TestC();



#endif
