#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

int main(int argc, char *argv[])
{
    int loop = 4;
    int intv_time = 1;
    int ret;
    char wdata = 0;
    char rdata;
    int fd = open("/dev/dev_beep", O_RDWR);
    printf("open fd=%d, errno=%d\n", fd, errno);
    while (loop > 0) {
        ret = write(fd, &wdata, sizeof(wdata));
        printf("write data=%d, ret=%d errno=%d\n", wdata, ret, errno);

        ret = read(fd, &rdata, sizeof(rdata));
        printf("read data=%d, ret=%d errno=%d\n", rdata, ret, errno);

        usleep(intv_time * 50000);
        loop--;
        wdata = !wdata;
        printf("\n");
    }
    close(fd);
    return 0;
}