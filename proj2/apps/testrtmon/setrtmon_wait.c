#include <unistd.h> 
#include <stdio.h>
#include <stdlib.h>
#include <sched.h>
#include <time.h>
// #include "linux/syscalls.h"
void timespec_add_ms(struct timespec *t, long ms)
{
	t->tv_nsec += ms * 1000000;
	if (t->tv_nsec > 1000000000) {
		t->tv_nsec = t->tv_nsec - 1000000000;
		t->tv_sec += 1;
	}
}

void timespec_init(struct timespec *t)
{
	t->tv_sec = 0;
	t->tv_nsec = 0;
}

long long timespec_to_ms(struct timespec t)
{
	return (t.tv_sec * 1000000000 + t.tv_nsec) / 1000000;
}

long long timespec_to_ns(struct timespec t)
{
	return (t.tv_sec * 1000000000 + t.tv_nsec);
}

int main(int argc, char *argv[]){
    int pid, c_ms, t_ms, cpuid;
    long ret, ret2, ret1;

    if (argc != 3) {
        fprintf(stderr, "Usage: %s <C_ms> <T_ms>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    c_ms = atoi(argv[1]);
    t_ms = atoi(argv[2]);
    // cpuid = atoi(argv[3]);
    // Get current task pid
    pid = getpid();
    printf("current task pid: %d\n", pid);
    // cpu_set_t set;
	// CPU_ZERO(&set);
	// CPU_SET(cpuid, &set);
	// if (sched_setaffinity(getpid(), sizeof(set), &set) == -1) {
	// 	perror("sched_setaffinity error");
	// 	return -1;
	// }	
    // Set C, T for current task
    ret = syscall(450, pid, c_ms, t_ms);
    if (ret == 0) {
        printf("set_rtmon %d successful.\n", pid);
    } else {
        printf("Error: %ld\n", ret);
        exit(EXIT_FAILURE);
    }
    
    // Periodic Loop
    int cnt = 0;
    while(1){
        // Do some computation
        printf("Pid %d is being executed! [%d]\n", pid, cnt);
        cnt++;
        struct timespec t1, t2;
		long long t1_ms, t2_ms;
		long long delta = 1; // error margin; currentl 1 ms
		clock_gettime(CLOCK_THREAD_CPUTIME_ID, &t1);
		t1_ms = timespec_to_ms(t1); 
        do {
			clock_gettime(CLOCK_THREAD_CPUTIME_ID, &t2);
			t2_ms = timespec_to_ms(t2);
		} while ((t2_ms - t1_ms + delta) < (c_ms));
        // Wait for next period
        ret = syscall(453);
        if(ret != 0){
            break;
        }
    }

    // struct timespec t1, t2;
    // long long t1_ms, t2_ms;
    // long long delta = 1; // error margin; currentl 1 ms
    // clock_gettime(CLOCK_THREAD_CPUTIME_ID, &t1);
    // t1_ms = timespec_to_ms(t1); 
    // do {
    //     clock_gettime(CLOCK_THREAD_CPUTIME_ID, &t2);
    //     t2_ms = timespec_to_ms(t2);
    // } while ((t2_ms - t1_ms + delta) < 1000);
    // // Wait for next period

    printf("End of the periodic task\n");

    // // Cancel the hrtimer 
    ret = syscall(451, pid);
    if (ret == 0){
        printf("task completed check the dmesgs.\n");
    }
    else {
        printf("Error occured.\n");
        exit(EXIT_FAILURE);
    }
    
    return 0;
}
