#include "type.h"
#include "const.h"
#include "protect.h"
#include "proc.h"
#include "proto.h"
#include "string.h"
#include "global.h"
#include "keyboard.h"
#include "keymap.h"


PRIVATE KB_INPUT kb_in;

PUBLIC void keyboard_handler(int irq)
{
    //disp_str("*");
    u8 scan_code = in_byte(KB_DATA);
    //disp_int(scan_code);

    if(kb_in.count < KB_IN_BYTES)
    {
        *(kb_in.p_head) = scan_code;
        kb_in.p_head ++;
        if(kb_in.p_head == kb_in.buf + KB_IN_BYTES)
        {
            kb_in.p_head = kb_in.buf;
        }
        kb_in.count ++;
    }
}

PUBLIC void init_keyboard()
{
    kb_in.count = 0;
    kb_in.p_head = kb_in.p_tail = kb_in.buf;

    put_irq_handler(KEYBOARD_IRQ, keyboard_handler);
    enable_irq(KEYBOARD_IRQ);
}


PUBLIC void keyboard_read()
{
    u8 scan_code;
    char output[2];
    int make; //True:make  False:break
    if(kb_in.count > 0)
    {
        disable_int();
        scan_code = *(kb_in.p_tail);
        kb_in.p_tail++;
        if(kb_in.p_tail == kb_in.buf + KB_IN_BYTES)
        {
            kb_in.p_tail = kb_in.buf;
        }
        kb_in.count--;
        enable_int();
       /* 
        if(0xE1 == scan_code)
        {

        }
        else if(0xE0 == scan_code)
        {
        }
        else
        {
            //首先判断Make code还是break code
            make = (scan_code & FLAG_BREAK ? FALSE : TRUE);

            //如果是Make code就打印，是break code则不做处理
            if(make)
            {
                output[0] = keymap[(scan_code&0x7F) * MAP_COLS];
                disp_str(output);
            }
        }*/
        disp_int(scan_code);
    }
}
