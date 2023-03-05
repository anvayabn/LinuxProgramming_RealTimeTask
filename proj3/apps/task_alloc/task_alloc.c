#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE_SIZE 1024
#define MAX_TASKS 10000
#define MAX_CPUS 100

typedef struct {
    char name[21];
    unsigned int C;
    unsigned int T;
} task_t;


int main(int argc, char **argv){
    if (argc != 2){
        printf("Usage: %s input_file\n", argv[0]);
        exit(1);
    }

    FILE *input_file = fopen(argv[1], "r");
    if(input_file == NULL){
        printf("Error: Cannot open input file %s\n", argv[1]);
        exit(1);
    }

    char line[MAX_LINE_SIZE];
    unsigned int B =0, N = 0; 
    char heuristic[4]; 
    task_t tasks[MAX_TASKS]; 

    // parse input file
    if(fgets(line, MAX_LINE_SIZE, input_file) != NULL){
        sscanf(line, "%u,%3s", &B, heuristic);
    }
    if(fgets(line, MAX_LINE_SIZE, input_file) != NULL){
        sscanf(line, "%u", &N);
    }
    for (int i = 0; i < N; i++){
        if(fgets(line, MAX_LINE_SIZE, input_file) != NULL){
            sscanf(line, "%20[^,],%u,%u", tasks[i].name, &tasks[i].C, &tasks[i].T);
        }
    }

    fclose(input_file);
    // Debug the input 
    printf("B = %u, heuristic = %s, N = %u\n", B, heuristic, N);
    for (int i = 0; i < N; i++) {
        printf("Task %u: name = %s, C = %u, T = %u\n", i+1, tasks[i].name, tasks[i].C, tasks[i].T);
    }
    if(heuristic == "BWF"){
        for(i=0;i<N;i++){
            
        }

    }else if(heuristic == "WFD"){

    }else if(heuristic == "FFD"){

    }else {
        printf("Error!! Does not execute this heuristic\n");
        return -1;
    }


    return 0;
}