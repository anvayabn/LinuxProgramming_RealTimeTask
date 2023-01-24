#include <stdio.h>
#include <fcntl.h>
#include <linux/ioctl.h>
#define IOCTL_PRINT_HELLO _IO(0, 0)
#define IOCTL_GET_TIME_NS _IO(0, 1)
int main(){
int fd;
long current_time=0;
int ret=0;
fd = open("/dev/rtesdev", O_RDONLY);
        if(fd < 0) {
                printf("Cannot open device file...\n");
                return 0;
        }
ioctl(fd, IOCTL_PRINT_HELLO, &ret);
ioctl(fd, IOCTL_GET_TIME_NS, &current_time);
printf("Current time in ns: %ld\n", current_time);
return 0;
}
