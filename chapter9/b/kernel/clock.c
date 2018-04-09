#include "const.h"
#include "type.h"
#include "protect.h"
#include "proc.h"
#include "tty.h"
#include "fs.h"
#include "global.h"
#include "proto.h"

PUBLIC void clock_handler(int irq)
{
//    disp_str("#");

    ticks++;
    p_proc_ready->ticks --;
    
    if(0 != k_reenter)
    {
//        disp_str("!");
        return;
    }

//    p_proc_ready++;
//    if(p_proc_ready >= proc_table + NR_TASKS)
//    {
//        p_proc_ready = proc_table;
//    }
    
    if(p_proc_ready->ticks > 0)
    {
        return;
    }

    schedule();
}

PUBLIC void init_clock()
{
    out_byte(TIMER_MODE, RATE_GENERATOR);
    out_byte(TIMER0, (u8)(TIMER_FREQ/HZ));
    out_byte(TIMER0, (u8)(TIMER_FREQ/HZ) >> 8);

    put_irq_handler(CLOCK_IRQ, clock_handler);
    enable_irq(CLOCK_IRQ);
}

PUBLIC void milli_delay(int milli_sec)
{
    int t = get_ticks();
    while(((get_ticks() - t) * 1000 / HZ) < milli_sec){}
}
