#define TTY_IN_BYTES    256  //tty input queue size 

struct s_console;

typedef struct s_tty
{
    u32 in_buf[TTY_IN_BYTES];  //TTY 输入缓冲区
    u32* p_inbuf_head; //指向缓冲区下一个空闲区
    u32* p_inbuf_tail; //指向键盘任务中应处理的键值
    int inbuf_count; //缓冲区中已经填充了多少

    struct s_console *p_console;
    
}TTY;

typedef struct s_console
{
    unsigned int current_start_addr; //当前显示到什么位置
    unsigned int original_add;  //该控制台显存的初始位置    
    unsigned int v_mem_limit;  //该控制台显存大小
    unsigned int cursor;  //当前光标位置
}CONSOLE;

#define DEFAULT_CHAR_COLOR  0x07
