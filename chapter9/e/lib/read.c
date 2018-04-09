

PUBLIC int read(int fd, void*buf, int count)
{
    MESSAGE msg;
    msg.type = READ;
    msg.FD = fd;
    msg.BUF = buf;
    msg.CNT = count;

    send_recv(BOTH, TASK_FS, &msg);
    return msg.CNT;
}
