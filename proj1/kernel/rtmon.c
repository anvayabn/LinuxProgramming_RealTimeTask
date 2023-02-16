#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/sched.h>
#include <linux/hrtimer.h>

#ifndef NR_PID_MAX
#define NR_PID_MAX 32768
#endif

#ifndef CLOCK_MONOTONIC
#define CLOCK_MONOTONIC 1
#endif

static struct hrtimer timer;


// enum hrtimer_restart my_hrtimer_callback( struct hrtimer ∗timer )
// {
//   printk( "my_hrtimer_callback called (%ld).\n", jiffies );

//   return HRTIMER_NORESTART;
// }
// Define a TCB structure to store timing parameters for a task

struct task_timing {
    ktime_t C; // Execution time
    ktime_t T; // Period
};

// Define an array to store the timing parameters for each task //NR_PID is a static constant 32768.
static struct task_timing task_timings[NR_PID_MAX];

SYSCALL_DEFINE3(set_rtmon, pid_t, pid, unsigned int, C_ms, unsigned int, T_ms)
{
    // Check if the provided pid is valid and belongs to a running task PIDTYPE_PID is a constant that is used to specify the type of a process ID in the Linux kernel. Specifically, it is used to indicate that a given process ID is associated with a process and not a thread.
    struct task_struct *task = get_pid_task(find_get_pid(pid), PIDTYPE_PID);
    if (task == NULL)
        return -1;

    // Check if the provided C_ms and T_ms values are within the allowed range
    if (C_ms > 10000 || T_ms > 10000 || C_ms < 1 || T_ms < 1)
        return -1;

    // Check if the task already has non-zero timing parameters set
    if (task_timings[pid].C != 0 || task_timings[pid].T != 0)
        return -1;

    // Convert the provided C_ms and T_ms values to ktime_t values in nanoseconds
    ktime_t C = ktime_set(0, C_ms * NSEC_PER_MSEC);
    ktime_t T = ktime_set(0, T_ms * NSEC_PER_MSEC);

    // Store the timing parameters in the task's TCB
    task_timings[pid].C = C;
    task_timings[pid].T = T;

    // Schedule a high-resolution timer to periodically check the task's execution time
    //struct hrtimer *timer = kmalloc(sizeof(struct hrtimer), GFP_KERNEL);
    
    hrtimer_init(&timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
    timer.function = HRTIMER_NORESTART;
    timer._softexpires = ktime_add(ktime_get(), T);
    hrtimer_start(&timer, T, HRTIMER_MODE_REL);

    return 0;
}

SYSCALL_DEFINE1(cancel_rtmon, pid_t, pid)
{
    //Check if the provided pid is valid and belongs to a running task
    struct task_struct *task = get_pid_task(find_get_pid(pid), PIDTYPE_PID);
    if (task == NULL)
        return -1;

    //Check if the task already has zero timing parameters set
    if (task_timings[pid].C == 0 && task_timings[pid].T == 0)
        return -1;

    //Clear the timing parameters in the task's TCB
    task_timings[pid].C = 0;
    task_timings[pid].T = 0;

    int ret = hrtimer_cancel(&timer);

    return ret;
}