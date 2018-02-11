#include "type.h"
#include "const.h"
#include "protect.h"
#include "tty.h"
#include "string.h"
#include "proc.h"
#include "proto.h"
#include "global.h"

//PUBLIC void* memcpy(void* pDst, void* pSrc, int iSize);
//PUBLIC void disp_str(char * pszInfo);

//PUBLIC u8 gdt_ptr[6]; // 0-15:Limit  16-47:base 
//PUBLIC DESCRIPTOR gdt[GDT_SIZE];

PUBLIC void cstart()
{
    disp_str("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n"
            "---------\"cstart\" begins---------\n");
    //将LOADER中 GDT 复制到新的GDT中
    memcpy(&gdt, //NEW GDT 
            (void*)(*((u32*)(&gdt_ptr[2]))), //base of Old GDT 
            *((u16*)(&gdt_ptr[0])) + 1  //Limit of old GDT 
          );

    //gdt_ptr[6] 共6个字节， 0-15：limit  16-17：base，用作sdgt/lgdt的参数
    u16* p_gdt_limit = (u16*)(&gdt_ptr[0]);
    u32* p_dgt_base = (u32*)(&gdt_ptr[2]);
    *p_gdt_limit = GDT_SIZE * sizeof(DESCRIPTOR) - 1;
    *p_dgt_base = (u32)&gdt;


    u16* p_idt_limit = (u16*)(&idt_ptr[0]);
    u32* p_idt_base = (u32*)(&idt_ptr[2]);

    *p_idt_limit = IDT_SIZE * sizeof(GATE) - 1;
    *p_idt_base = (u32)&idt;

    init_prot();
}
