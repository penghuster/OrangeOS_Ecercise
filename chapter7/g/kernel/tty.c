#include "type.h"
#include "const.h"
#include "protect.h"
#include "proto.h"
#include "string.h"
#include "proc.h"
#include "global.h"
#include "keyboard.h"


PUBLIC void task_tty()
{
    disp_str("tty");
    while(1)
    {
        keyboard_read();
    }
}

PUBLIC void in_process(u32 key)
{
    char output[2]={0};

    if(!(key & FLAG_EXT))
    {
        output[0] = key & 0xFF;
        disp_str(output);
    }
}
