#include <linux/module.h>
#include <linux/kernel.h> 
#include <linux/kthread.h> 
#include <linux/sched.h> 
#include <linux/time.h> 
 
int init_module(void){
    printk("Hello world! team10 in kernel space\n");
    return 0;  
}
void cleanup_module(void){ 
    printk("BYE\n"); 
} 
MODULE_LICENSE("GPL");