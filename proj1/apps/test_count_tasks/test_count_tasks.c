#include <unistd.h> 
int main(){
    int count; 
    long ret = syscall(449, &count); 
    if (ret == 0) printf("Number of real-time tasks: %d\n", count); 
    else printf("Error: %ld\n", ret);
    return 0;
}