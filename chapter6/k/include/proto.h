#ifndef ORANGESPROTO_H_
#define ORANGESPROTO_H_


PUBLIC void out_byte(u16 port, u8 value);
PUBLIC u8   in_byte(u16 port);
PUBLIC void disp_str(char *info);
PUBLIC void disp_color_str(char* info, int color);
PUBLIC void init_prot();
PUBLIC void init_8259A();
PUBLIC void disp_int(int num);

PUBLIC void clock_handler(int irq);
PUBLIC void put_irq_handler(int irq, irq_handler handler);
PUBLIC void spurious_irq(int irq);

void TestA();
void TestB();
void TestC();



#endif
