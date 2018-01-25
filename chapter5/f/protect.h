#ifndef ORANGESPROTECT_H_
#define ORANGESPROTECT_H_

#include "type.h"

typedef struct s_descriptor 
{
    u16 limit_low;  //limit
    u16 base_low;   //base 
    u8  base_mid;   //base
    u8  attr1;      // P-1, DPL-2 DT-1  TYPE-4
    u8  limit_high_attr2;  //G-1  D-1 O-1  AVL-1  LimitHigh--4
    u8  base_high;   // Base
} DESCRIPTOR;

#endif 
