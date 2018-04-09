#ifndef ORANGESPROTO_H_
#define ORANGESPROTO_H_

//kliba.asm
PUBLIC void out_byte(u16 port, u8 value);
PUBLIC u8   in_byte(u16 port);
PUBLIC void disp_str(char *info);
PUBLIC void disp_color_str(char* info, int color);
PUBLIC void init_prot();
PUBLIC void disp_int(int num);
PUBLIC void disable_int();
PUBLIC void enable_int();
PUBLIC void disable_irq(int irq);
PUBLIC void enable_irq(int irq);
PUBLIC void port_read(u16 port, void* buf, int n);
PUBLIC void port_write(u16 port, void* buf, int n);

//string.asm 
PUBLIC char* strcpy(char*dst, const char* src);

//i8259.c
PUBLIC void init_8259A();
PUBLIC void put_irq_handler(int irq, irq_handler handler);
PUBLIC void spurious_irq(int irq);

//clock.c
PUBLIC void clock_handler(int irq);
PUBLIC void init_keyboard();
PUBLIC void init_clock();

//tty.c
//与键盘/显示器相关
PUBLIC void in_process(TTY* p_tty, u32 key);
PUBLIC void task_tty();
PUBLIC void out_char(CONSOLE* p_con, char ch);
PUBLIC int is_current_console(CONSOLE* p_con);
PUBLIC void scroll_screen(CONSOLE* p_con, int direction);
PUBLIC void select_console(int nr_console);
PUBLIC void init_screen(TTY* p_tty);


//printf.c
PUBLIC int printf(const char* fmt, ...);
PUBLIC int vsprintf(char *buf, const char *fmt, va_list args);
PUBLIC int sprintf(char *buf, const char *fmt, ...);
#define printl printf 

//lib/misc.c 
PUBLIC void spin(char* func_name);

//kernel.asm 
PUBLIC void restart();

//与系统调用相关
PUBLIC void sys_call();  //int_handler

//系统调用---系统级
PUBLIC int sys_get_ticks();
PUBLIC int sys_write(char *buf, int len, PROCESS* p_proc);
PUBLIC int sys_printx(int _unused1, int unused2, char* s, struct s_proc *p_proc);
//实现进程间通信的系统调用--系统级
PUBLIC int sys_sendrec(int function, int src_dest, MESSAGE* m, struct s_proc* p);

//系统调用---用户级
PUBLIC int get_ticks();
PUBLIC void write(char* buf, int len);
PUBLIC void printx(char *s);
//实现进程间通信的系统调用--用户级
PUBLIC int sendrec(int function, int src_dest, MESSAGE* p_msg);


PUBLIC void milli_delay(int milli_sec);

//proc.c 
PUBLIC void schedule();
PUBLIC void* va2la(int pid, void *va);
PUBLIC int ldt_seg_linear(struct s_proc* p, int idx);
PUBLIC void reset_msg(MESSAGE* p);
PUBLIC void dump_msg(const char* title, MESSAGE* m);
PUBLIC void dump_proc(struct s_proc* p);
PUBLIC void inform_int(int task_nr);
//进程间通信
PUBLIC int send_recv(int function, int src_dest, MESSAGE *msg);

//main.c
void TestA();
void TestB();
void TestC();
PUBLIC void panic(const char *fmt, ...);

//systask.c
void task_sys();

//kernel/hd.c
void task_hd();
PUBLIC void hd_handler(int irq);

//fs/main.c
void task_fs();
PUBLIC int rw_sector(int io_type, int dev, int pos, int bytes, int proc_nr, void* buf);
int test(int x, int y, int z, int n, int m, void* l);



#endif
