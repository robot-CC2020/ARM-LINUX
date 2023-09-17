#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

int main(int argc, char *argv[])
{
    int loop = 50;
    int intv_time = 1;
    int ret;
    char rdata = 0;

    int fd2 = open("/dev/my_key", O_RDWR);
    printf("open fd=%d, errno=%d\n", fd2, errno);

    while (loop > 0) {
        ret = read(fd2, &rdata, sizeof(rdata));
        printf("read data=%d, ret=%d errno=%d\n", rdata, ret, errno);

        usleep(intv_time * 100000);
        loop--;
        printf("\n");
    }
    close(fd2);
    return 0;
}