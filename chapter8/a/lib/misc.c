#include "type.h"
#include "const.h"
#include "protect.h"
#include "string.h"
#include "proc.h"
#include "tty.h"
#include "global.h"
#include "keyboard.h"
#include "proto.h"



PUBLIC void assertion_failure(char *exp, char *file, char *base_file, int line)
{
   printl("%c  assert(%s) failed: file: %s, base_file: %s, line: %d",
            MAG_CH_ASSERT, exp, file, base_file, line);
   // printl("%c%s%s", MAG_CH_ASSERT, exp, base_file);
   //printl("%c %d", MAG_CH_ASSERT, 12);
    //If assertion fails in a task, the system will halt before printl return 
    //if it happens in a user process, printl will return like a conmmon routine
    //and arrive here.
    

    //we use a forever loop to prevent the proc from going on 
    spin("assertion_failure()");

    //should never arrive here
    __asm__ __volatile__("ud2");
}

PUBLIC void spin(char* func_name)
{
    printl("\nspinning in %s...\n", func_name);
    while(1){}
}
