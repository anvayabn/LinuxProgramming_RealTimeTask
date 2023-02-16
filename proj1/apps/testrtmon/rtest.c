#include <unistd.h> 
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]){
    int pid, c_ms, t_ms;
    long ret, ret2;

    if (argc != 4) {
        fprintf(stderr, "Usage: %s <pid> <C_ms> <T_ms>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    pid = atoi(argv[1]);
    c_ms = atoi(argv[2]);
    t_ms = atoi(argv[3]);

    // Call the set_rtmon system call with the provided arguments
    ret = syscall(450, pid, c_ms, t_ms);
    if (ret == 0) {
        printf("set_rtmon successful.\n");
    } else {
        printf("Error: %ld\n", ret);
        exit(EXIT_FAILURE);
    }

    // Call the cancel_rtmon system call with the provided pid
    ret2 = syscall(451, pid);
    if (ret2 == 1) {
        printf("cancel_rtmon successful.\n");
    } else if (ret2 == 0) {
        printf("Error: timer does not exist.\n");
    } else {
        printf("Error: %ld\n", ret2);
        exit(EXIT_FAILURE);
    }

    return 0;
}