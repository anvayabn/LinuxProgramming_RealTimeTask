#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/mm.h>
#include <linux/sched.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/slab.h>
#include <linux/mm_types.h>

#define DEVICE_NAME "vm_areas"

static int vm_areas_open(struct inode *inode, struct file *file)
{
    // This function is called when the device file is opened
    return 0;
}

static ssize_t vm_areas_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
    // This function is called when the user writes to the device file
    pid_t pid;
    const char *p; // for converting the string into to integer
    int val; 
    struct task_struct *task;
    struct mm_struct *mm;
    struct vm_areas_struct *vma;
    char *kbuf;

    if (count > 20) {
        printk(KERN_ALERT "vm_areas: error: Input too long.\n");
        return -1;
    }

    // Allocate kernel buffer for the input
    kbuf = kmalloc(count + 1, GFP_KERNEL);
    if (!kbuf) {
        printk(KERN_ALERT "vm_areas: error: Memory allocation failed.\n");
        return -1;
    }

    // Copy user input to kernel buffer
    if (copy_from_user(kbuf, buf, count)) {
        kfree(kbuf);
        return -1;
    }
    
    //input type 4603 > /dev/vm_areas
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
        printk(KERN_ALERT "vm_areas: error: Cannot retrieve memory info for PID %d.\n", pid);
        kfree(kbuf);
        return -1;
    }    
	printk(KERN_INFO "[Memory-mapped areas of process %ld]\n", pid);
    vma = mm.mmap;
    if (!vma){
        printk(KERN_ALERT "vm_areas: error: Cannot retrive vma for PID %d.\n", pid);
        kfree(kbuf);
        return -1;
    }

	for (vma; vma = vma->vm_next) {
		printk(KERN_INFO "%08lx - %08lx: %lu bytes%s\n",
			   vma->vm_start, vma->vm_end,
			   vma->vm_end - vma->vm_start,
			   vma->vm_flags & VM_LOCKED ? " [L]" : "");
	}
    kfree(kbuf);
	return count;    


}
static const struct file_operations vm_areas_fops = {
    .owner = THIS_MODULE,
    .open = vm_areas_open,
    .write = vm_areas_write,
};

static struct miscdevice vm_areas_miscdev = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = DEVICE_NAME,
    .fops = &vm_areas_fops,
};
static int __init vm_areass_init(void)
{
    int ret = misc_register(&vm_areas_miscdev);
    if (ret) {
        printk(KERN_ERR "Unable to register %s misc device\n", DEVICE_NAME);
        return ret;
    }
    printk(KERN_INFO "vm_areas: registered misc device %s\n", DEVICE_NAME);
    return 0;
}
static void __exit vm_areass_exit(void)
{
    misc_deregister(&vm_areas_miscdev);
    printk(KERN_INFO "vm_areas: unregistered misc device %s\n", DEVICE_NAME);
}
module_init(vm_areass_init);
module_exit(vm_areass_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Team10");
MODULE_DESCRIPTION("A misc device driver for retrieving memory segment info of all process in the VM Area");