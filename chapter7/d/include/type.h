#ifndef ORANGESTYPE_H_
#define ORANGESTYPE_H_


typedef unsigned int u32; 
typedef unsigned short u16; 
typedef unsigned char u8;

typedef void (*int_handler)();
typedef void (*irq_handler)();

typedef void (*task_f) ();

typedef void* system_call;

#endif
