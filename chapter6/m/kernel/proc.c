#include "type.h"
#include "const.h"
#include "protect.h"
#include "proto.h"
#include "proc.h"
#include "global.h"

PUBLIC int sys_get_ticks()
{
//    disp_str("+");
    return ticks;
}
