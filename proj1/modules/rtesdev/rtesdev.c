#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/fs.h>
#include <linux/time.h>
#include <linux/miscdevice.h>
#include <linux/ioctl.h>
#include <linux/kdev_t.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/err.h>
#define IOCTL_PRINT_HELLO _IO(0, 0)
#define IOCTL_GET_TIME_NS _IO(0, 1)
#define DEVICE_NAME "rtesdev" //device name
static int rtesdev_open(struct inode *inode, struct file *file)
{
    return 0;
}
static int rtesdev_release(struct inode *inode, struct file *file)
{
    return 0;
}
static long int rtesdev_read(struct file *file, char __user *buf, long unsigned int len, long long int *offset)
{
    char task_info[256];
    int len_of_data = 0;
    struct task_struct *task;
    printk(KERN_INFO "%-10s %-10s %-10s %-20s\n", "tid", "pid", "pr", "name");
    for_each_process(task) {
        if (task->rt_priority > 0) {
            printk(KERN_INFO "%-10d %-10d %-10d %-20s\n", task->tgid, task->pid, task->rt_priority, task->comm);
        }
    }
        if (len_of_data == 0) {
            return 0;
    }
    if (len < len_of_data) {
        len = len_of_data;
    }
    if (copy_to_user(buf, task_info, len_of_data)) {
        return -EFAULT;
    }
    return len_of_data;
}
static long int ioctl_funcs(struct file *filp,unsigned int cmd, unsigned long arg) { 
    switch(cmd) {
        case IOCTL_PRINT_HELLO: 
            printk(KERN_INFO "Hello world! team10 in kernel space\n");
            break;      
        case IOCTL_GET_TIME_NS: 
            *(ktime_t*)arg =  ktime_get();
            break;
        default:
            printk(KERN_INFO "Wrong command");
            return 1;
    }
    return 0;
}
static struct file_operations rtesdev_fops = {
    .owner = THIS_MODULE,
    .open = rtesdev_open,
    .unlocked_ioctl = ioctl_funcs,
    .release = rtesdev_release,
    .read = rtesdev_read,
};
static struct miscdevice rtesdev_misc = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = DEVICE_NAME,
    .fops = &rtesdev_fops,
};
static int __init rtesdev_init(void)
{
    int ret;
    ret = misc_register(&rtesdev_misc);
    if (ret) {
        printk(KERN_ERR "Unable to register %s misc device\n", DEVICE_NAME);
        return ret;
    }
    printk(KERN_INFO "rtesdev: registered misc device %s\n", DEVICE_NAME);
    return 0;
}
static void __exit rtesdev_exit(void)
{
    misc_deregister(&rtesdev_misc);
    printk(KERN_INFO "rtesdev: unregistered misc device %s\n", DEVICE_NAME);
}
module_init(rtesdev_init);
module_exit(rtesdev_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("team10");
MODULE_DESCRIPTION("Misc Devise Driver");

