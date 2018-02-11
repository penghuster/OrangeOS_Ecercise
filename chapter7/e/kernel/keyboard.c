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
PRIVATE int code_with_E0 = 0;
PRIVATE int shift_l;
PRIVATE int shift_r;
PRIVATE int alt_l;
PRIVATE int alt_r;
PRIVATE int ctrl_l;
PRIVATE int ctrl_r;
PRIVATE int caps_lock;
PRIVATE int num_lock;
PRIVATE int scroll_lock;
PRIVATE int column;

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

    //用一个整型来表示一个键，比如 home， 被按下，则key值将为定义在keyboard.h种的HOME
    u32 key = 0;
    //指向keymap[] 的某一行
    u32 *keyrow;

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
        
        if(0xE1 == scan_code)
        {

        }
        else if(0xE0 == scan_code)
        {
            code_with_E0 = 1;
        }
        else
        {
            //首先判断Make code还是break code
            make = (scan_code & FLAG_BREAK ? FALSE : TRUE);

            //先定位到keymap中的行
            keyrow = &keymap[(scan_code & 0x7f) * MAP_COLS];
            column = 0;
            if(shift_l || shift_r)
            {
                column = 1;
            }

            if(code_with_E0)
            {
                column = 2;
                code_with_E0 = 0;
            }

            key = keyrow[column];

            switch(key)
            {
                case SHIFT_L:
                    shift_l = make;
                    key = 0;
                    break;
                case SHIFT_R:
                    shift_r = make;
                    key = 0;
                    break;
                case CTRL_L:
                    ctrl_l = make;
                    key = 0;
                    break;
                case CTRL_R:
                    ctrl_r = make;
                    key = 0;
                    break;
                case ALT_L:
                    alt_l = make;
                    key = 0;
                    break;
                case ALT_R:
                    alt_r = make;
                    key = 0;
                    break;
                default:
                    if(!make)
                    {
                        key = 0;
                    }
                    break;
            }

            //如果是Make code就打印，是break code则不做处理
            if(key)
            {
                output[0] = key;
                disp_str(output);
            }
        }
        //disp_int(scan_code);
    }
}
