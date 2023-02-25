#include <unistd.h> 
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]){
    int pid;
    long ret;

    pid = atoi(argv[1]);


    ret = syscall(452, pid);
    if (ret == 0) {
        printf("print_rtmon successful.\n");
    } else if (ret == -1) {
        printf("Error: timer does not exist.\n");
    } else {
        printf("Error: %ld\n", ret);
        exit(EXIT_FAILURE);
    }

    return 0;
}