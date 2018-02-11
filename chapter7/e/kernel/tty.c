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
