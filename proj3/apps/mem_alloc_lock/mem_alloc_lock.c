# include<time.h>
#include <unistd.h>
int main(int argc, char*argv[]) {
 int n = atoi(argv[1]);/* parsed from the input */
 char *buf = malloc(n);
 if(mlock(buf, n)){
    printf("mlock failed!\n");
    return -1;
 }
 /* check memory access time */
 struct timespec t1, t2; 
 clock_gettime(CLOCK_MONOTONIC, &t1);
 for (int i = 0; i < n; i += 4096) /* write one byte in each page (4096 bytes) */
 buf[i] = 1;
 clock_gettime(CLOCK_MONOTONIC, &t2);
//  print current PID and the total memory access time (t2 – t1) in nsec
printf("PID %d, %d ns\n", getpid(), (t2.tv_sec-t1.tv_sec) * 1000000000 + (t2.tv_nsec-t1.tv_nsec));
 pause(); /* wait here; can be terminated by ‘Ctrl-C’ */
 return 0;
}