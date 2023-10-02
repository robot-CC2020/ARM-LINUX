#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <poll.h>
#include <fcntl.h>
#include <signal.h>

int non_block_test(void)
{
    int loop = 5;
    int ret;
    char rdata = 0;
    int fd = open("/dev/my_key", O_RDWR | O_NONBLOCK); // 打开时使用 flags O_NONBLOCK
    printf("open fd=%d, errno=%d\n", fd, errno);

    while (loop > 0) {
        printf("start read : \n");
        ret = read(fd, &rdata, sizeof(rdata));
        printf("read data=%d, ret=%d errno=%d\n", rdata, ret, errno);
        sleep(1);
        loop--;
    }
    printf("close fd\n");
    close(fd);
    return 0;
}

int block_test(void)
{
    int loop = 5;
    int ret;
    char rdata = 0;
    int fd = open("/dev/my_key", O_RDWR);
    printf("open fd=%d, errno=%d\n", fd, errno);

    while (loop > 0) {
        printf("start read : \n");
        ret = read(fd, &rdata, sizeof(rdata));
        printf("read data=%d, ret=%d errno=%d\n", rdata, ret, errno);
        loop--;
    }
    printf("close fd\n");
    close(fd);
    return 0;
}

int poll_test(void)
{

    int fd;//要监视的文件描述符
    char buf[32] = {0};   
    struct pollfd fds[1];
    int ret;
    int loopCnt = 5;
    fd = open("/dev/my_key", O_RDWR);  //打开/dev/test设备，阻塞式访问
    if (fd < 0) {
        perror("open error \n");
        return fd;
    }

    fds[0] .fd =fd;
    fds[0].events = POLLIN;

    printf("before loop \n");
    while (loopCnt > 0) {
        printf("loop cnt: %d \n", loopCnt);
        int nfds = poll(fds, 1, 2000); // 2000 毫秒
        if (nfds < 0) {
            printf("poll error !!\n");
        }
        if(nfds == 0) {
            printf("time out !!\n");
        }
        if (nfds == 1 && fds[0].revents == POLLIN) {
            ret = read(fd, buf, sizeof(buf));  // 从文件读取数据
            printf("read %d bytes!\n", ret);
        }
        loopCnt--;
    }
    printf("after loop\n");

    close(fd);     //关闭文件
    return 0;
}


static int g_fd;
static unsigned char g_buf[32] = {0};   
static void sigal_handler(int signum)
{
    int ret;
    ret = read(g_fd, g_buf, sizeof(g_buf));
    printf("recv signal %d, read ret = %d\n", signum, ret);
}

int sig_test(void)  
{
    int ret;
    int flags;
    int loopCnt = 5;
    g_fd = open("/dev/my_key", O_RDWR);
    if (g_fd < 0) {
        perror("open error \n");
        return -1;
    }
    // 设置 信号处理函数
    signal(SIGIO, sigal_handler);

    // 设置能接收这个信号的进程
    fcntl(g_fd, F_SETOWN, getpid());
    
    // 文件描述符设置 标志位 FASYNC
    flags = fcntl(g_fd, F_GETFD);
    fcntl(g_fd, F_SETFL, flags | FASYNC); // 开启信号驱动IO

    while (loopCnt > 0) {
        printf("loop cnt: %d \n", loopCnt);
        sleep(5);
        loopCnt--;
    }
    
    close(g_fd);     //关闭文件
    return 0;
}

int main(int argc, char *argv[])
{
    int ret;
    int select = 3;
    switch (select) {
        case 0:
            ret = block_test();
            break;
        case 1:
            ret = non_block_test();
            break;
        case 2:
            ret = poll_test();
            break;
        case 3:
            ret = sig_test();
            break;
    }
    return ret;
}