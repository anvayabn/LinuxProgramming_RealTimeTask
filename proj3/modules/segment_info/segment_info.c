#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/slab.h>
#include <linux/mm_types.h>

#define DEVICE_NAME "segment_info"

  
static int segment_info_open(struct inode *inode, struct file *file)
{
    // This function is called when the device file is opened
    return 0;
}

static ssize_t segment_info_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
    // This function is called when the user writes to the device file
    pid_t pid;
    const char *p; // for converting the string into to integer
    int val; 
    struct task_struct *task;
    struct mm_struct *mm;
    char *kbuf;

    if (count > 20) {
        printk(KERN_ALERT "segment_info: error: Input too long.\n");
        return -1;
    }

    // Allocate kernel buffer for the input
    kbuf = kmalloc(count + 1, GFP_KERNEL);
    if (!kbuf) {
        printk(KERN_ALERT "segment_info: error: Memory allocation failed.\n");
        return -1;
    }

    // Copy user input to kernel buffer
    if (copy_from_user(kbuf, buf, count)) {
        kfree(kbuf);
        return -1;
    }
    
    //input type 4603 > /dev/vm_area
    p = strstrip(kbuf);
    if (kstrtoint(p, 0, &val)){
        printk("error got val : %i\n", val);
        kfree(kbuf);
        return -1; 
    }
    pid = val ;

    task = pid_task(find_vpid(pid), PIDTYPE_PID);
    if (task == NULL){
        kfree(kbuf);
        return -1 ; 
    }

    printk("[Memory segment address of process %d]\n", pid);

    mm = task->mm;
    if (!mm) {
        printk(KERN_ALERT "segment_info: error: Cannot retrieve memory info for PID %d.\n", pid);
        kfree(kbuf);
        return -1;
    }
    if (mm->start_code && mm->end_code) {
        printk(KERN_INFO "%08lx - %08lx: code segment (%lu bytes)\n",
        mm->start_code, mm->end_code, mm->end_code - mm->start_code);
    }
    if (mm->start_data && mm->end_data) {
        printk(KERN_INFO "%08lx - %08lx: data segment (%lu bytes)\n",
        mm->start_data, mm->end_data, mm->end_data - mm->start_data);
    }
    kfree(kbuf);
    return count;
}

static const struct file_operations segment_info_fops = {
    .owner = THIS_MODULE,
    .open = segment_info_open,
    .write = segment_info_write,
};

static struct miscdevice segment_info_miscdev = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = DEVICE_NAME,
    .fops = &segment_info_fops,
};
static int __init segment_info_init(void)
{
    int ret = misc_register(&segment_info_miscdev);
    if (ret) {
        printk(KERN_ERR "Unable to register %s misc device\n", DEVICE_NAME);
        return ret;
    }
    printk(KERN_INFO "rtesdev: registered misc device %s\n", DEVICE_NAME);
    return 0;
}
static void __exit segment_info_exit(void)
{
    misc_deregister(&segment_info_miscdev);
    printk(KERN_INFO "rtesdev: unregistered misc device %s\n", DEVICE_NAME);
}

module_init(segment_info_init);
module_exit(segment_info_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Team10");
MODULE_DESCRIPTION("A misc device driver for retrieving memory segment info of a process");