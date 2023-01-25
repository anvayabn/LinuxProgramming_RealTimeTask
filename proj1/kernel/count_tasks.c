#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/syscalls.h>
// #define __NR_count_rt_tasks 449
//EXPORT_SYMBOL(sys_count_rt_tasks);

//SYSCALL_DEFINE1(count_rt_tasks, int*, result);
SYSCALL_DEFINE1(count_rt_tasks, int*, result){
    if (!result)
        return -EINVAL;
    int count = 0;
    struct task_struct *task;
    for_each_process(task)
    {
        printk("task_id: %d, task_pr: %d", count, task->rt_priority);
        if (task->rt_priority > 0)
            count++;
    }
    *result = count;
    return 0;
}

//EXPORT_SYMBOL(sys_count_rt_tasks);
