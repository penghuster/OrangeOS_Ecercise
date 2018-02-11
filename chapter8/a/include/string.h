#ifndef ORANGESSTRING_H_
#define ORANGESSTRING_H_


PUBLIC void memcpy(void* src, void* dest, int size);
PUBLIC void memset(void* src, int zero, int size);
PUBLIC int strlen(char* p_str);


#define phys_copy memcpy
#define phys_set  memset 

#endif
