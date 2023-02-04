#include <linux/module.h>
#include <linux/kallsyms.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/syscalls.h>
#include "ftrace_helper.h"
//#define __NR_count_rt_tasks 449
#define SYSCALL_TO_REPLACE __NR_count_rt_tasks
MODULE_LICENSE("GPL");
//EXPORT_SYMBOL_GPL(sys_count_rt_tasks); 
char *sym_name = "sys_call_table";
int index = 0;
unsigned long original_cr0;
unsigned long *sys_call_table;
typedef asmlinkage long (*custom_count_rt_tasks) (int* result);

custom_count_rt_tasks old_count_rt_tasks;



#define PTREGS_SYSCALL_STUBS 1

#ifdef PTREGS_SYSCALL_STUBS

static asmlinkage long my_count_rt_tasks(int* result)
{
    printk("I'm the syscall which overwrittes the previous !\n");
       if (!result)
        return -EINVAL;
    int count = 0;
    struct task_struct *task;
    for_each_process(task)
    {
        if (task->rt_priority > 50)
            count++;
    }
    *result = count;
    return 0; 
}
#endif
static struct ftrace_hook hooks[] = {
    HOOK("sys_count_rt_tasks", my_count_rt_tasks, &old_count_rt_tasks),
};
static int __init hello_init(void)
{	
    int err;
    err = fh_install_hooks(hooks, ARRAY_SIZE(hooks));
    if(err)
        return err;

    printk(KERN_INFO "my_count_rt_tasks: loaded\n");
    return 0; 
}

static void __exit hello_exit(void)
{
    fh_remove_hooks(hooks, ARRAY_SIZE(hooks));
    printk(KERN_INFO "my_count_rt_tasks: unloaded\n");
}

module_init(hello_init);
module_exit(hello_exit);

