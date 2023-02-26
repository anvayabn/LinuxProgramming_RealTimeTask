#include <unistd.h> 
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]){
    int c_ms, t_ms;
    long ret;

    if (argc != 3) {
        fprintf(stderr, "Usage: %s <pid> <C_ms> <T_ms>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    pid_t pid = getpid();
    c_ms = atoi(argv[1]);
    t_ms = atoi(argv[2]);

    // Call the set_rtmon system call with the provided arguments
    ret = syscall(450, pid, c_ms, t_ms);
    if (ret == 0) {
        printf("set_rtmon successful.\n");
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
        // Wait for next period
        ret = syscall(453);
        if(ret != 0){
            break;
        }
    }
    printf("End of the periodic task\n");
    // // Cancel the hrtimer 
    // ret = syscall(451, pid);
    // if (ret == 0){
    //     printf("task completed check the dmesgs.\n");
    // }
    // else {
    //     printf("Error occured.\n");
    //     exit(EXIT_FAILURE);
    // }
    
    return 0;
}

