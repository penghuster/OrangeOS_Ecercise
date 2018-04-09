#include "type.h"
#include "const.h"
#include "protect.h"
#include "string.h"
#include "proc.h"
#include "tty.h"
#include "fs.h"
#include "global.h"
#include "proto.h"
#include "keyboard.h"

#define TTY_FIRST (tty_table)
#define TTY_END   (tty_table + NR_CONSOLES)

extern void keyboard_read(TTY* p_tty);

PRIVATE void init_tty(TTY *p_tty);
PRIVATE void tty_dev_write(TTY *p_tty);
PRIVATE void tty_dev_read(TTY *p_tty);
PRIVATE void put_key(TTY* p_tty, u32 key);
PRIVATE void tty_do_read(TTY* tty, MESSAGE *msg);
PRIVATE void tty_do_write(TTY* tty, MESSAGE *msg);


PUBLIC void task_tty()
{
    TTY* p_tty;
    MESSAGE msg;

    init_keyboard();

    for(p_tty=TTY_FIRST; p_tty < TTY_END; p_tty++)
    {
        init_tty(p_tty);
    }
    //nr_current_console = 0;
    select_console(0);
    disp_str("tty");
   // assert(0);
    while(1)
    {
        int i = 0;
        for(p_tty = TTY_FIRST; p_tty < TTY_END; p_tty++)
        {
//    if(is_current_console(p_tty->p_console))
//                out_char(p_tty->p_console, 'a');
//              
            do 
            {
                tty_dev_read(p_tty);
                tty_dev_write(p_tty);
            }while(p_tty->inbuf_count);
        }

                printf("tty-------dsdsd\n");

        send_recv(RECEIVE, ANY, &msg);

        int src = msg.source;
        assert(src != TASK_TTY);


        TTY* ptty = &tty_table[msg.DEVICE];

        switch(msg.type)
        {
        case DEV_OPEN:
            reset_msg(&msg);
            msg.type = SYSCALL_RET;
            send_recv(SEND, src, &msg);
            break;
        case DEV_READ:
            tty_do_read(ptty, &msg);
            break;
        case DEV_WRITE:
            tty_do_write(ptty, &msg);
            break;
        case HARD_INT:
            printf("press\n");
            key_pressed = 0;
            continue;
        default:
            dump_msg("TTY::unknown msg", &msg);
            break;
        }
    }
}

PUBLIC void in_process(TTY* p_tty, u32 key)
{
    char output[2]={0};

    if(!(key & FLAG_EXT))
    {
        put_key(p_tty, key);
    }
    else
    {
        int raw_code = key & MASK_RAW;
        switch(raw_code)
        {
        case ENTER:
            put_key(p_tty, '\n');
            break;
        case BACKSPACE:
            put_key(p_tty, '\b');
            break;
        case UP:
            if((key & FLAG_SHIFT_L) || (key & FLAG_SHIFT_R))
            {
                scroll_screen(p_tty->p_console, SCR_DN);
            }
            break;
        case DOWN:
            if((key & FLAG_SHIFT_L) || (key & FLAG_SHIFT_R))
            {
                //shift + down  do nothing
                scroll_screen(p_tty->p_console, SCR_UP);
            }
            break;
        case F1:
        case F2:
        case F3:
        case F4:
        case F5:
        case F6:
        case F7:
        case F8:
        case F9:
        case F10:
        case F11:
        case F12:
            if((key & FLAG_ALT_L) || (key & FLAG_ALT_R))
            {
                select_console(raw_code - F1);
            }
        default:
            break;

        }
    }
}

PRIVATE void put_key(TTY* p_tty, u32 key)
{
    if(p_tty->inbuf_count < TTY_IN_BYTES)
    {
        *(p_tty->p_inbuf_head) = key;
        p_tty->p_inbuf_head++;
        if(p_tty->p_inbuf_head == p_tty->in_buf + TTY_IN_BYTES)
        {
            p_tty->p_inbuf_head = p_tty->in_buf;
        }
        p_tty->inbuf_count++;
    }
}

PRIVATE void init_tty(TTY *p_tty)
{
    p_tty->inbuf_count = 0;
    p_tty->p_inbuf_head = p_tty->p_inbuf_tail = p_tty->in_buf;

    init_screen(p_tty);

}

PRIVATE void tty_dev_read(TTY *p_tty)
{
    if(is_current_console(p_tty->p_console))
    {
                //out_char(p_tty->p_console, 'r');
        keyboard_read(p_tty);

    }
}

PRIVATE void tty_dev_write(TTY *p_tty)
{
    while(p_tty->inbuf_count)
    {
        char ch = *(p_tty->p_inbuf_tail);
                out_char(p_tty->p_console, ch);
        p_tty->p_inbuf_tail++;
        if(p_tty->p_inbuf_tail == p_tty->in_buf + TTY_IN_BYTES)
        {
            p_tty->p_inbuf_tail = p_tty->in_buf;
        }
        p_tty->inbuf_count--;

        if(p_tty->tty_left_cnt)
        {
            if(ch >= '  ' && ch <= '~') //printable
            {
 //               printf("%d ---%c\n", p_tty->p_console, ch);
                out_char(p_tty->p_console, ch);
                void *p = p_tty->tty_req_buf + p_tty->tty_trans_cnt;
                phys_copy(p, (void*)va2la(TASK_TTY, &ch), 1);
                p_tty->tty_trans_cnt++;
                p_tty->tty_left_cnt--;
            }
            else if(ch == '\b' && p_tty->tty_trans_cnt)
            {
                out_char(p_tty->p_console, ch);
                p_tty->tty_trans_cnt--;
                p_tty->tty_left_cnt++;
            }
            else if(ch == '\n' || p_tty->tty_left_cnt == 0)
            {
                out_char(p_tty->p_console, '\n');
                MESSAGE msg;
                msg.type = RESUME_PROC;
                msg.CNT = p_tty->tty_trans_cnt;
                msg.PROC_NR = p_tty->tty_procnr;
                printf("hhh %d\n", p_tty->tty_caller);
                send_recv(SEND, p_tty->tty_caller, &msg);
                p_tty->tty_left_cnt = 0;
            }
        }
    }
}

/**
 * tty_do_read 
 *
 * Invoked when task TTY recieves DEV_READ message
 *
 * @note the routine will return immediately after setting some numbers of 
 * TTY struct, telling FS to suspend the proc who want to read. the real transfer 
 * (tty buffer -> proc buffer) is not done here 
 *
 * @param tty From which TTY the caller proc wants to read.
 * @param msg the MESSAGE just recieved.
 */ 
PRIVATE void tty_do_read(TTY* tty, MESSAGE* msg)
{
    //tell the tty 
    tty->tty_caller = msg->source; //who called, usually FS 
    tty->tty_procnr = msg->PROC_NR; //who wants the chars 
    tty->tty_req_buf = va2la(tty->tty_procnr, msg->BUF); //where the chars should be put
    tty->tty_left_cnt = msg->CNT; //how many chars are request 
    tty->tty_trans_cnt = 0; //how many chars has been transfered 

    msg->type = SUSPEND_PROC;
    msg->CNT = tty->tty_left_cnt;
    
        printf("tty-read---3\n");
        printf("proc = %d\n", tty->tty_caller);
    send_recv(SEND, tty->tty_caller, msg);


}

/**
 * tty_do_write
 *
 * Invoked when task TTY recieves DEV_WRITE message
 *
 * @param tty to which TTY the caller proc is found 
 * @param msg The MESSAGE.
 */ 
PRIVATE void tty_do_write(TTY* tty, MESSAGE *msg)
{
    char buf[TTY_OUT_BUF_LEN];
    char *p = (char*)va2la(msg->PROC_NR, msg->BUF);
    int i = msg->CNT;
    int j;

    while(i)
    {
        int bytes = min(TTY_OUT_BUF_LEN, i);
        phys_copy(va2la(TASK_TTY, buf), (void*)p, bytes);
        for(j = 0; j < bytes; j++)
        {
            out_char(tty->p_console, buf[j]);
        }
        i -= bytes;
        p += bytes;
    }
    
    msg->type = SYSCALL_RET;
    send_recv(SEND, msg->source, msg);
}

PUBLIC void tty_write(TTY* p_tty, char* buf, int len)
{
    char* p = buf;
    int i = len;
    while(i)
    {
        out_char(p_tty->p_console, *p++);
        i--;
    }
}

PUBLIC int sys_write(char* buf, int len, PROCESS* p_proc)
{
    tty_write(&tty_table[p_proc->nr_tty], buf, len);
    return 0;
}

PUBLIC int sys_printx(int _unused1, int unused2, char* s, struct s_proc *p_proc)
{
    const char *p;
    char ch;

    char reenter_err[] = "? k_reenter is incorrect for unkown reason.";
    reenter_err[0] = MAG_CH_PANIC;

    /**
     * @note Code in both Ring 0 and Ring 1-3 may invoke printx(). if this happens in 
     * Ring 0, no linear-physical address mapping is needed.
     *
     * @attention The value of 'k_reenter' is tricky here. when
     * -# printx() is called in Ring 0
     * - k_reenter > 0. when code in Ring 0 calls printx(), an 'interrupt reenter' will
     *   occur (printx() generates a software interrupt). thus 'k_reenter' will be 
     *   increased by 'kernel.asm::save' and be greater than 0.
     *  -# printx() is called in Ring 1-3
     *   - k_reenter == 0.
     */
    if(k_reenter == 0) //printx() called in Ring<1-3>
    {
        p = va2la(proc2pid(p_proc), s);
    }
    else if(k_reenter > 0) //printx() called in Ring 0
        p = s;
    else 
        p = reenter_err;

    /**
     * @note if assertion fails in any TASK, the system will be halted;
     * if it fails in a USER_PROC, it'll return like any normal system call does.
     */ 
    if((*p == MAG_CH_PANIC) ||
            (*p == MAG_CH_ASSERT && p_proc_ready < &proc_table[NR_TASKS]))
    {
        out_char(tty_table[p_proc->nr_tty].p_console, 'H');
        disable_int();
        char *v = (char*)V_MEM_BASE;
        const char *q = p + 1; //skip the magic char 
        
        while(v < (char*)(V_MEM_BASE + V_MEM_SIZE))
        {
            *v++ = *q++;
            *v++ = RED_CHAR;
            if(!*q)
            {
                while(((int)v - V_MEM_BASE) % (SCREEN_WIDTH * 16))
                {
                    v++;
                    *v++ = GRAY_CHAR;
                }
                q = p + 1;
            }
        }

        __asm__ __volatile("sti\n"
                "hlt\n");
    }
    while((ch = *p++) != 0)
    {
        if(MAG_CH_PANIC == ch || MAG_CH_ASSERT == ch)
            continue;

        out_char(tty_table[p_proc->nr_tty].p_console, ch);
    }
    return 0;
}
