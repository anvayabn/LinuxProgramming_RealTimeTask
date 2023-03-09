#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_CPUS 100
#define MAX_TASKS 10000
#define MAX_NAME_LEN 20

// Define data structures for tasks and CPUs
typedef struct {
    char name[MAX_NAME_LEN];
    double C;
    double T;
} Task;

typedef struct {
    double capacity;
    double utilization;
    Task *tasks;
    unsigned int num_tasks;
} CPU;

// int compare_cpu(const void* a, const void* b) {
//     const CPU* cpu_a = (const CPU*) a;
//     const CPU* cpu_b = (const CPU*) b;
//     if (cpu_a->capacity < cpu_b->capacity) {
//         return -1;
//     } else if (cpu_a->capacity > cpu_b->capacity) {
//         return 1;
//     } else {
//         return 0;
//     }
// }

/*The cpus here will be initialized to  */
// Define bin-packing heuristics: BFD, WFD, FFD
int BFD(CPU *cpus, Task task, unsigned int num_cpus) {
    unsigned int i, j;
    int available_cpu = num_cpus;
    double task_utilization = task.C/task.T; 
    double cpu_utilization = task.C/task.T;

    // Start allocation from CPU0
    for (i=0; i<num_cpus; i++){
        // printf("the value of i %u\n", num_cpus);
        // printf("The current capacity of the cpu is %f\n", cpus[i].capacity);
        if (cpus[i].capacity >= task_utilization){
            if (available_cpu == num_cpus || cpus[i].capacity < cpus[available_cpu].capacity ){
                available_cpu = i ; 
            }
        }
    }
    if (available_cpu == num_cpus){
        return -i ; 
    }

    cpus[available_cpu].capacity -= task_utilization;
    // printf("capacity %f\n", cpus[available_cpu].capacity);
    cpus[available_cpu].utilization += task_utilization;
    // printf("utilization %f\n", cpus[available_cpu].utilization);
    cpus[available_cpu].tasks[cpus[available_cpu].num_tasks++] = task;
    return available_cpu;
} 
int WFD(CPU *cpus, Task task, unsigned int num_cpus) {

    unsigned int i, j;
    int available_cpu = num_cpus;
    double task_utilization = task.C/task.T; 
    double cpu_utilization = task.C/task.T;

    // Start allocation from CPU0
    for (i=0; i<num_cpus; i++){
        // printf("the value of i %u\n", num_cpus);
        // printf("The current capacity of the cpu is %f\n", cpus[i].capacity);
        if (cpus[i].capacity >= task_utilization){
            if (available_cpu == num_cpus || cpus[i].capacity > cpus[available_cpu].capacity ){
                available_cpu = i ; 
            }
        }
    }
    if (available_cpu == num_cpus){
        return -i ; 
    }

    cpus[available_cpu].capacity -= task_utilization;
    // printf("capacity %f\n", cpus[available_cpu].capacity);
    cpus[available_cpu].utilization += task_utilization;
    // printf("utilization %f\n", cpus[available_cpu].utilization);
    cpus[available_cpu].tasks[cpus[available_cpu].num_tasks++] = task;
    return available_cpu;
} 
int FFD(CPU *cpus, Task task, unsigned int num_cpus) {
    unsigned int i, j;
    int available_cpu = num_cpus;
    double task_utilization = task.C/task.T; 
    double cpu_utilization = task.C/task.T;

    // qsort(cpus, num_cpus, sizeof(CPU), compare_cpu);
    // Start allocation from CPU0
    for (i=0; i<num_cpus; i++){
        // printf("the value of i %u\n", num_cpus);
        // printf("The current capacity of the cpu is %f\n", cpus[i].capacity);
        if (cpus[i].capacity >= task_utilization){
                available_cpu = i ; 
                break;
        }
    }
    if (available_cpu == num_cpus){
        return -i ; 
    }

    cpus[available_cpu].capacity -= task_utilization;
    // printf("capacity %f\n", cpus[available_cpu].capacity);
    cpus[available_cpu].utilization += task_utilization;
    // printf("utilization %f\n", cpus[available_cpu].utilization);
    cpus[available_cpu].tasks[cpus[available_cpu].num_tasks++] = task;
    return available_cpu;
} 
int compare_tasks(const void *a, const void *b) {
    Task *task1 = (Task *) a;
    Task *task2 = (Task *) b;
    
    double util1 = (double) task1->C / task1->T;
    double util2 = (double) task2->C / task2->T;

    if (util1 < util2) {
        return 1;
    } else if (util1 > util2) {
        return -1;
    } else {
        return 0;
    }
}
int main(int argc, char *argv[]) {
    FILE *fp;
    unsigned int num_cpus, num_tasks, i, j, k, bin_capacity;
    char heuristic[4];
    Task *tasks;
    CPU *cpus;

    // Parse command-line arguments and input file
    if (argc != 2) {
        printf("Usage: %s input_file\n", argv[0]);
    }
    // open file and check if not null 
    fp = fopen(argv[1], "r");
    if (fp == NULL) {
        printf("Error: cannot open input file %s\n", argv[1]);
        exit(1);
    }

    fscanf(fp, "%u,%3s\n", &num_cpus, heuristic);
    // if heuristic is FFD bin_capacity is max number of tasks 
    // if (strcmp(heuristic, "FFD") == 0) {
    //     bin_capacity = MAX_TASKS;
    // } else {
    // bin_capacity = MAX_TASKS / num_cpus;
    // }

    cpus = (CPU *) malloc(num_cpus * sizeof(CPU));
    for (i = 0; i < num_cpus; i++) {
        cpus[i].capacity = 1;
        cpus[i].utilization = 0;
        cpus[i].tasks = (Task *) malloc(MAX_TASKS * sizeof(Task));
        cpus[i].num_tasks = 0;
    }

    fscanf(fp, "%u\n", &num_tasks);

    tasks = (Task *) malloc(num_tasks * sizeof(Task));

    for (i = 0; i < num_tasks; i++) {
    fscanf(fp, "%[^,],%le,%le\n", tasks[i].name, &tasks[i].C, &tasks[i].T);
    }

    //close the file 
    fclose(fp);

    // Sort tasks in decreasing order of utilization
    qsort(tasks, num_tasks, sizeof(Task), compare_tasks);
    // debug sort 
    // for (i=0; i < num_tasks; i++ ){
    //     printf("%s,%f,%f\n", tasks[i].name, tasks[i].C, tasks[i].T);
    // }
    // Allocate tasks to CPUs using selected bin-packing heuristic
    int cpu_index;

    for (i = 0; i < num_tasks; i++) {
        if (strcmp(heuristic, "BFD") == 0) {
            cpu_index = BFD(cpus, tasks[i], num_cpus);
        } else if (strcmp(heuristic, "WFD") == 0) {
            cpu_index = WFD(cpus, tasks[i], num_cpus);
        } else if (strcmp(heuristic, "FFD") == 0) {
            cpu_index = FFD(cpus, tasks[i], num_cpus);
        } else {
            printf("Error: unsupported bin-packing heuristic %s\n", heuristic);
            exit(1);
        }

        if (cpu_index < 0) {
            printf("Failure\n");
            exit(1);
        }
    }

    // Print allocation results
    printf("Success\n");

    for (i = 0; i < num_cpus; i++) {
        printf("CPU%u", i);
    
        for (j = 0; j < cpus[i].num_tasks; j++) {
            printf(",%s", cpus[i].tasks[j].name);
        }
    
        printf("\n");
    }

    // Free memory and exit
    for (i = 0; i < num_cpus; i++) {
        free(cpus[i].tasks);
    }

    free(cpus);
    free(tasks);

    return 0;
}