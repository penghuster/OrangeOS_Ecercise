#include "type.h"
#include "const.h"
#include "protect.h"
#include "proc.h"
#include "proto.h"
#include "string.h"
#include "global.h"

PUBLIC void keyboard_handler(int irq)
{
    disp_str("*");
    u8 scan_code = in_byte(0x60);
    disp_int(scan_code);
}

PUBLIC void init_keyboard()
{
    put_irq_handler(KEYBOARD_IRQ, keyboard_handler);
    enable_irq(KEYBOARD_IRQ);
}
