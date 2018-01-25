#include "const.h"
#include "type.h"
#include "proto.h"
//itoa
//数字前面的0不被显示出来，比如0000B800显示为 B800
PUBLIC char* itoa(char* str, int num)
{
    char* p = str;
    char ch;
    int i;
    int flag = 0;

    *p++ = '0';
    *p++ = 'x';

    if(0 == num)
    {
        *p++ = '0';
    }
    else
    {
        for(i=28; i>=0; i-=4)
        {
            ch = (num >> i) & 0xf;
            if(flag || (ch > 0))
            {
                flag = 1;
                ch += '0';
                if(ch > '9')
                {
                    ch += 7;
                }
                *p++ = ch;
            }
        }
    }
    
    *p=0;
    return str;
}


PUBLIC void disp_int(int input)
{
    char output[16];
    itoa(output, input);
    disp_str(output);
}
