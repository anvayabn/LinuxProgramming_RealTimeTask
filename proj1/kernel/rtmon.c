#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/sched.h>
#include <linux/hrtimer.h>

static ktime_t ET;
static ktime_t Period;


SYSCALL_DEFINE3(set_rtmon, pid_t, pid, unsigned int, C_ms, unsigned int, T_ms)
{
    
    // Check if the provided pid is valid and belongs to a running task PIDTYPE_PID is a constant that is used to specify the type of a process ID in the Linux kernel. Specifically, it is used to indicate that a given process ID is associated with a process and not a thread.
    struct task_struct *task = get_pid_task(find_get_pid(pid), PIDTYPE_PID);
    if (task == NULL){
        printk("Task Doesn't Exist\n");
        return -1;
    }

    // Check if the provided C_ms and T_ms values are within the allowed range
    if (C_ms > 10000 || T_ms > 10000 || C_ms < 1 || T_ms < 1){
        printk("Given Values are wrong\n");
        return -1;
    }

    // Check if the task already has non-zero timing parameters set
    if (task->C > 0 || task->T > 0){
        printk("Task Exists and has non zero parameters\n");
        return -1;
    }
    // Convert the provided C_ms and T_ms values to ktime_t values in nanoseconds
    ET = ktime_set(0, C_ms * NSEC_PER_MSEC);
    Period = ktime_set(0, T_ms * NSEC_PER_MSEC);   

    task->C = ET;
    task->T = Period; 

    printk("The value of Period is : %lld\n", task->T);
    printk("The Value of Execution Time is : %lld\n", task->C);

    return 0;
}

SYSCALL_DEFINE1(cancel_rtmon, pid_t, pid)
{
    //Check if the provided pid is valid and belongs to a running task
    struct task_struct *task = get_pid_task(find_get_pid(pid), PIDTYPE_PID);
    if (task == NULL){
        printk("Task Doesn't Exist\n");
        return -1;
    }
    printk("%lld and %lld\n", task->C, task->T);

    //Check if the task already has zero timing parameters set
    if (task->C == 0 && task->T == 0){
        printk("Task is already cleared\n");
        return -1;
    }
    //Clear the timing parameters in the task's TCB
    task->C = 0;
    task->T = 0;

    return 0;
}
SYSCALL_DEFINE1(print_rtmon, pid_t, pid)
{
    if(pid >= 0){
        struct task_struct *task = get_pid_task(find_get_pid(pid), PIDTYPE_PID);
        if (task == NULL){
            printk("Task Doesn't Exist\n");
            return -1;
        }
        ktime_t C_ms = (task->C/1000000);
        ktime_t T_ms = (task->T/1000000);
        printk("print_rtmon: PID %d, C %lld ms, T %lld ms\n", task->pid, C_ms, T_ms);
        return 0;
    }

    if(pid == -1){
        struct task_struct *task;
        for_each_process(task){
            ktime_t C_ms = (task->C/1000000);
            ktime_t T_ms = (task->T/1000000);
            printk("print_rtmon: PID %d, C %lld ms, T %lld ms\n", task->pid, C_ms, T_ms);
        }
    }else{
        printk("Error!!!!!");
        return -1;
    }
    return 0;
}