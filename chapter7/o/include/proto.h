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

//与键盘/显示器相关
PUBLIC void in_process(TTY* p_tty, u32 key);
PUBLIC void task_tty();
PUBLIC void out_char(CONSOLE* p_con, char ch);
PUBLIC int is_current_console(CONSOLE* p_con);


PUBLIC int printf(const char* fmt, ...);
PUBLIC int vsprintf(char *buf, const char *fmt, va_list args);



//与系统调用相关
PUBLIC void sys_call();

PUBLIC int sys_get_ticks();
PUBLIC int sys_write(char *buf, int len, PROCESS* p_proc);

PUBLIC int get_ticks();
PUBLIC void write(char* buf, int len);


PUBLIC void milli_delay(int milli_sec);

PUBLIC void schedule();

void TestA();
void TestB();
void TestC();



#endif
