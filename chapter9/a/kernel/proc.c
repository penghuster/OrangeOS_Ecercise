#include "type.h"
#include "const.h"
#include "string.h"
#include "protect.h"
#include "proc.h"
#include "tty.h"
#include "proto.h"
#include "global.h"


PRIVATE int msg_receive(struct s_proc* current, int src, MESSAGE* m);
PRIVATE int msg_send(struct s_proc* current, int dest, MESSAGE* m);

PUBLIC void schedule()
{
    PROCESS* p;
    int greatest_ticks = 0;
    while(!greatest_ticks)
    {
        for(p = proc_table; p < proc_table + NR_TASKS + NR_PROCS; p++)
        {
            if(p->p_flags == 0)
            {
                if(p->ticks > greatest_ticks)
                {
                    greatest_ticks = p->ticks;
                    p_proc_ready = p;
                }
            }
        }

        if(!greatest_ticks)
        {
            for(p = proc_table; p < proc_table + NR_TASKS + NR_PROCS; p++)
            {
                if(p->p_flags == 0)
                    p->ticks = p->priority;
            }
        }
    }
}


PUBLIC int sys_get_ticks()
{
//    disp_str("+");
    return ticks;
}

//the core routine of system call 'sendrec()'
PUBLIC int sys_sendrec(int function, int src_dest, MESSAGE* m, struct s_proc* p)
{
    assert(k_reenter == 0);
    assert((src_dest >= 0 && src_dest < NR_TASKS + NR_PROCS)
            || src_dest == ANY 
            || src_dest == INTERRUPT);

    int ret = 0;
    int caller = proc2pid(p);
    MESSAGE* mla = (MESSAGE*)va2la(caller, m);
    mla->source = caller;

    assert(mla->source != src_dest);

    //事实上，还有一种消息类型，BOTH，然而，此类型不允许被直接传输到内核，
    //内核完全不知道，它被转换为一个SEND　并紧跟一个　RECIEVE
    if(SEND == function)
    {
        ret = msg_send(p, src_dest, m);
        if(0 != ret)    return ret;
    }
    else if(RECEIVE == function)
    {
        ret = msg_receive(p, src_dest, m);
        if(0 != ret)  return ret;
    }
    else
    {
        panic("{sys_sendrec} invalid. function: "
                "%d (SEND:%d,   RECEIVE:%d) ",function, SEND, RECEIVE);
    }

}


//根据给定的 PCB 计算某个段的线性地址
PUBLIC int ldt_seg_linear(struct s_proc* p, int idx)
{
    struct s_descriptor* d = &p->ldts[idx];
    return d->base_high << 24 | d->base_mid << 16 | d->base_low;
}

PUBLIC void* va2la(int pid, void* va)
{
    struct s_proc* p = &proc_table[pid];

    u32 seg_base = ldt_seg_linear(p, INDEX_LDT_RW);
    u32 la = seg_base + (u32)va;

    if(pid < NR_TASKS + NR_PROCS)
    {
        assert(la == (u32)va);
    }

    return (void*)la;
}

/*************************************************************
 * reset_msg
 *************************************************************/ 
/**
 * <Ring 0-3> Clear up a MESSAGE by setting eath byte to 0
 */ 
PUBLIC void reset_msg(MESSAGE *p)
{
    memset(p, 0, sizeof(MESSAGE));
}

/*
 * @function block 
 *
 * <Ring 0> This routine is called after 'p_flags' has been set(!=0), 
 * it calls 'schedule()' to choose another proc as the 'proc_ready'
 *
 * @attention This routine does not change 'p_flags'. make sure the 
 * 'p_flags' of the proc to be blocked has been set properly.
 *
 * @param p the proc to be blocked 
 */ 
PRIVATE void block(struct s_proc* p)
{
    assert(p->p_flags);
    schedule();
}

/*
 * @function unblock 
 *
 * <Ring 0> This is a dummy routine, it does nothing acturally. when it is called,
 *  the 'p_flags' should have been cleaned (==0)
 *
 *  @param p the unblocked proc 
 */ 
PRIVATE void unblock(struct s_proc* p)
{
    assert(p->p_flags == 0);
}

/*
 * deadlock
 *
 * <Ring 0> Check whether it is safe to a message from src to dest.
 * the routine will detect if the messaging graph contains a cycle.
 * for instance, if we have procs trying to send a message like this: 
 * A -> B -> C -> A , then a deadlock occurs, because all of them will
 * wait forever. if no cycles detected, it is considered as safe. 
 *
 * @param src Who wants to send message
 * @param dest To whom the message is sent 
 *
 * @return 0 if success.
 */ 
PRIVATE int deadlock(int src, int dest)
{
    struct s_proc* p = proc_table + dest;
    while(1)
    {
        //若目标进程处于待发送状态,则需判断是否可能死锁
        if(p->p_flags & SENDING)
        {
            //首先,查看是否存在发送目标为src的进程,若存在,则必然出现死锁
            if(p->p_sendto == src)
            {
                //print the chain
                p = proc_table + dest;
                printl("=_=%s", p->p_name);
                do 
                {
                    assert(p->p_msg);
                    p = proc_table + p->p_sendto;
                    printl("->%s", p->p_name);
                }while(p != proc_table + src);
                printl("=_=");
                
                return 1;
            }
            p = proc_table + p->p_sendto;
        }
        else
        {
            break;
        }
    }
    return 0;
}

/*
 * @function msg_send
 *
 * <Ring 0> Send a message to the dest proc. if dest is blocked waiting for the 
 * message, copy the message to it and unblock dest. otherwise the caller will be 
 * blocked and appended to the dest's send queue.
 *
 * @param current the caller, the sender 
 * @param dest To whom the message is sent 
 * @param m The message
 *
 * @return Zero if success
 */ 

PRIVATE int msg_send(struct s_proc* current, int dest, MESSAGE* m)
{
    struct s_proc* sender = current;
    struct s_proc* p_dest = proc_table + dest; //proc dest 
    assert(proc2pid(sender) != dest);

    //check for deadlock here 
    if(deadlock(proc2pid(sender), dest))
    {
        panic(">>DEADLOCK<< %s->%s", sender->p_name, p_dest->p_name);
    }

    if((p_dest->p_flags & RECEIVING) && //dest is waiting for a message
       (p_dest->p_recvfrom == proc2pid(sender)) ||
       (p_dest->p_recvfrom == ANY))
    {
        assert(p_dest->p_msg);
        assert(m);
        phys_copy(va2la(dest, p_dest->p_msg), va2la(proc2pid(sender), m),
                sizeof(MESSAGE));
        p_dest->p_msg = 0;
        p_dest->p_flags &= ~RECEIVING;  //dest has recieved the msg 
        p_dest->p_recvfrom = NO_TASK;
        unblock(p_dest);

        assert(p_dest->p_flags == 0);
        assert(p_dest->p_msg == 0);
        assert(p_dest->p_recvfrom == NO_TASK);
        assert(p_dest->p_sendto == NO_TASK);
        assert(sender->p_flags == 0);
        assert(sender->p_msg == 0);
        assert(sender->p_recvfrom == NO_TASK);
        assert(sender->p_sendto == NO_TASK);
    }
    else //dest is not waiting for the message
    {
        sender->p_flags |= SENDING;
        assert(sender->p_flags == SENDING);
        sender->p_sendto = dest;
        sender->p_msg = m;

        //append to the sending queue
        struct s_proc* p;
        if(p_dest->q_sending)
        {
            p = p_dest->q_sending;
            //移动到最后一个节点
            while(p->next_sending)
                p = p->next_sending;
            p->next_sending = sender;
        }
        else
        {
            p_dest->q_sending = sender;
        }
        sender->next_sending = 0;
        block(sender);

        assert(sender->p_flags == SENDING);
        assert(sender->p_msg != 0);
        assert(sender->p_recvfrom == NO_TASK);
        assert(sender->p_sendto == dest);
    }
    return 0;
}

/*
 * @function msg_receive
 *
 * <Ring 0> Try to get a message from the src proc. if src is blocked sending 
 * the message. copy the message from it and unblock src. otherwise the caller 
 * will be blocked
 *
 * @param current The caller, the proc who wanna recieve 
 * @param src From whom the message will be recieve 
 * @param m The message ptr to accept the message.
 *
 * @return  Zero if success
 */ 
PRIVATE int msg_receive(struct s_proc* current, int src, MESSAGE* m)
{
    //the name is a little bit wierd. but it makes me think clearly, so i keep it.
    struct s_proc* p_who_wanna_recv = current;
    struct s_proc* p_from = 0;
    struct s_proc* prev = 0;
    int copyok = 0;

    assert(proc2pid(p_who_wanna_recv) != src);

    if((p_who_wanna_recv->has_int_msg) &&
      ((src == ANY) || (src == INTERRUPT)))
    {
        //There is an interrupt needs p_who_wanna_recv's handling and 
        //p_who_wanna_recv is ready to handling it. 
        MESSAGE msg;
        reset_msg(&msg);
        msg.source = INTERRUPT;
        msg.type = HARD_INT;
        assert(m);
        phys_copy(va2la(proc2pid(p_who_wanna_recv), m), &msg,
            sizeof(MESSAGE));

        p_who_wanna_recv->has_int_msg = 0;

        assert(p_who_wanna_recv->p_flags == 0);
        assert(p_who_wanna_recv->p_msg == 0);
        assert(p_who_wanna_recv->p_sendto == NO_TASK);
        assert(p_who_wanna_recv->has_int_msg == 0);

        return 0;
    }
    //Arrives here if no interrupt for p_who_wanna_recv
    if(src == ANY)
    {
        //p_who_wanna_recv is ready to receive messages from any proc. we will 
        //check the sending queue and pick the first proc in it.
        if(p_who_wanna_recv->q_sending)
        {
            p_from = p_who_wanna_recv->q_sending;
            copyok = 1;

            assert(p_who_wanna_recv->p_flags == 0);
            assert(p_who_wanna_recv->p_msg == 0);
            assert(p_who_wanna_recv->p_recvfrom == NO_TASK);
            assert(p_who_wanna_recv->p_sendto == NO_TASK);
            assert(p_who_wanna_recv->q_sending != 0);
            assert(p_from->p_flags == SENDING);
            assert(p_from->p_msg != 0);
            assert(p_from->p_recvfrom == NO_TASK);
            assert(p_from->p_sendto == proc2pid(p_who_wanna_recv));
        }
    }
    else if(src >= 0 && src < NR_TASKS + NR_PROCS)
    {
        //p_who_wanna_recv wants to recieve a message from 
        //a certain proc: src 
        p_from = &proc_table[src];

        if((p_from->p_flags & SENDING) &&
           (p_from->p_sendto == proc2pid(p_who_wanna_recv)))
        {
            //perfect, src is sending a message to p_who_wanna_recv.
            copyok = 1;

            struct s_proc* p = p_who_wanna_recv->q_sending;
            //p_from must have been appended to the queue, so the queue
            //must not be NULL.
            assert(p);

            while(p)
            {
                assert(p_from->p_flags & SENDING);
                if(proc2pid(p) == src)
                {
                    p_from = p;
                    break;
                }
                prev = p;
                p = p->next_sending;
            }

            assert(p_who_wanna_recv->p_flags == 0);
            assert(p_who_wanna_recv->p_msg == 0);
            assert(p_who_wanna_recv->p_recvfrom == NO_TASK);
            assert(p_who_wanna_recv->p_sendto == NO_TASK);
            assert(p_who_wanna_recv->q_sending != 0);
            assert(p_from->p_flags == SENDING);
            assert(p_from->p_msg != 0);
            assert(p_from->p_recvfrom == NO_TASK);
            assert(p_from->p_sendto == proc2pid(p_who_wanna_recv));
        }
    }

    if(copyok)
    {
        //It's determined from which proc the message will be copied.
        //Note that this proc must have been waiting for this moment in the queue,
        //so we should remove it from the queue.
        if(p_from == p_who_wanna_recv->q_sending) // the 1st one 
        {
            assert(prev == 0);
            p_who_wanna_recv->q_sending = p_from->next_sending;
            p_from->next_sending = 0;
        }
        else
        {
            assert(prev);
            prev->next_sending = p_from->next_sending;
            p_from->next_sending = 0;
        }

        assert(m);
        assert(p_from->p_msg);

        //copy the message
        phys_copy(va2la(proc2pid(p_who_wanna_recv), m),
                va2la(proc2pid(p_from), p_from->p_msg),
                sizeof(MESSAGE));
        p_from->p_msg = 0;
        p_from->p_sendto = NO_TASK;
        p_from->p_flags &= ~SENDING;

        unblock(p_from);
    }
    else
    {
        //nobody's sending any msg, Set p_flags so that p_who_wanna_recv will not 
        //be scheduled until it is unblocked.
        p_who_wanna_recv->p_flags |= RECEIVING;
        p_who_wanna_recv->p_msg = m;
        p_who_wanna_recv->p_recvfrom = src;
        block(p_who_wanna_recv);

        assert(p_who_wanna_recv->p_flags == RECEIVING);
        assert(p_who_wanna_recv->p_msg != 0);
        assert(p_who_wanna_recv->p_recvfrom != NO_TASK);
        assert(p_who_wanna_recv->p_sendto == NO_TASK);
        assert(p_who_wanna_recv->has_int_msg == 0);
    }

    return 0;
}

/*
 * @function 
 *
 * <Ring 1 - 3> IPC syscall
 *
 * it is an encapsulation of 'sendrec' invoking 'sendrec' directly should be  avoided
 *
 * @param function SEND, RECEIVE or BOTH
 * @param src_dest the caller's proc_nr 
 * @param msg pointer to the MESSAGE struct 
 *
 * @return always 0.
 */ 
PUBLIC int send_recv(int function, int src_dest, MESSAGE *msg)
{
    int ret = 0;
    if(function == RECEIVE)
        memset(msg, 0, sizeof(MESSAGE));
    
    switch(function)
    {
    case BOTH: 
        ret = sendrec(SEND, src_dest, msg);
        if(0 == ret)
            ret = sendrec(RECEIVE, src_dest, msg);
        break;
    case SEND:
    case RECEIVE:
        ret = sendrec(function, src_dest, msg);
        break;
    default:
        assert((function == BOTH) ||
                (function == SEND) ||
                (function == RECEIVE));
        break;
            
    }
    return ret;
}

/*
 * inform_int 
 *
 * <Ring 0> Inform a proc that an interrupt has occured 
 *
 * @param task_nr the task which will be informed 
 */ 
PUBLIC void inform_int(int task_nr)
{
    PROCESS* p = proc_table + task_nr;

    if((p->p_flags & RECEIVING) &&
            ((p->p_recvfrom == INTERRUPT) || (p->p_recvfrom = ANY)))
    {
        p->p_msg->source = INTERRUPT;
        p->p_msg->type = HARD_INT;
        p->p_msg = 0;
        p->has_int_msg = 0;
        p->p_flags &= ~RECEIVING;
        p->p_recvfrom = NO_TASK;
        assert(p->p_flags == 0);
        unblock(p);

        assert(p->p_msg == 0);
        assert(p->p_recvfrom == NO_TASK);
        assert(p->p_sendto == NO_TASK);
    }
    else 
    {
        p->has_int_msg = 1;
    }
}

PUBLIC void dump_proc(struct s_proc* p)
{
    char info[STR_DEFAULT_LEN];
    int i;
    int text_color = MAKE_COLOR(GREEN, RED);
    int dump_len = sizeof(struct s_proc);

    out_byte(CRTC_ADDR_REG, START_ADDR_H);
    out_byte(CRTC_DATA_REG, 0);
    out_byte(CRTC_ADDR_REG, START_ADDR_L);
    out_byte(CRTC_DATA_REG, 0);

    sprintf(info, "byte dump of proc_table[%d]:\n", p - proc_table);
    disp_color_str(info, text_color);

    for(i = 0; i < dump_len; i++)
    {
        sprintf(info, "%x.", ((unsigned char *)p)[i]);
        disp_color_str(info, text_color);
    }

    disp_color_str("\n\n", text_color);
    sprintf(info, "ANY: 0x%x.\n", ANY);
    disp_color_str(info, text_color);

    sprintf(info, "NO_TASK: 0x%x.\n", NO_TASK);
    disp_color_str(info, text_color);

    disp_color_str("\n", text_color);

    sprintf(info, "ldt_sel: 0x%s.  ", p->ldt_sel);
    disp_color_str(info, text_color);

    sprintf(info, "ticks: 0x%x.   ", p->ticks);
    disp_color_str(info, text_color);

    sprintf(info, "priority: 0x%x.  ", p->priority);
    disp_color_str(info, text_color);

    sprintf(info, "pid: 0x%x.  ", p->pid);
    disp_color_str(info, text_color);

    sprintf(info, "name: %s.  ", p->p_name);
    disp_color_str(info, text_color);

    disp_color_str("\n", text_color);

    sprintf(info, "p_flags: 0x%x.   ", p->p_flags);
    disp_color_str(info, text_color);

    sprintf(info, "p_recvfrom: 0x%x.  ", p->p_recvfrom);
    disp_color_str(info, text_color);

    sprintf(info, "p_sendto: 0x%x.   ", p->p_sendto);
    disp_color_str(info, text_color);

    sprintf(info, "nr_tty: 0x%x.   ", p->nr_tty);
    disp_color_str(info, text_color);
    
    disp_color_str("\n", text_color);

    sprintf(info, "has_int_msg: 0x%x.   ", p->has_int_msg);
    disp_color_str(info, text_color);
}

/*
 * dump_msg
 */ 
PUBLIC void dump_msg(const char* title, MESSAGE* m)
{
    int packed = 0;
    printl("{%s}<0x%x>{%ssrc:%s(%d),%stype:%d,%s(0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x)%s}%s", \
            title,
            (int)m,
            packed ? "  " : "\n    ",
            proc_table[m->source].p_name,
            m->source,
            packed ? "  " : "\n    ",
            m->type,
            packed ? "  " : "\n    ",
            m->u.m3.m3i1,
            m->u.m3.m3i2,
            m->u.m3.m3i3,
            m->u.m3.m3i4,
            (int)m->u.m3.m3p1,
            (int)m->u.m3.m3p2,
            packed ? "  " : "\n",
            packed ? "  " : "\n"
          );
}
