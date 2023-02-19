#include <unistd.h> 
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]){
    int pid;
    long ret;


    pid = atoi(argv[1]);

    ret = syscall(451, pid);
    if (ret == 0){
        printf("task completed check the dmesgs.\n");
    }else {
        printf("Error occured.\n");
        exit(EXIT_FAILURE);
    }

    return 0;
}