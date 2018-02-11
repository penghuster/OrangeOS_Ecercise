#include "const.h"
#include "type.h"
#include "protect.h"
#include "proc.h"
#include "global.h"

PUBLIC void clock_handler(int irq)
{
    disp_str("#");
    
    if(0 != k_reenter)
    {
        disp_str("!");
        return;
    }

    p_proc_ready++;
    if(p_proc_ready >= proc_table + NR_TASKS)
    {
        p_proc_ready = proc_table;
    }
}
