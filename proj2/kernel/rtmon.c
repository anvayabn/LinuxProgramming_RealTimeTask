#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/sched.h>
#include <linux/hrtimer.h>
#include <linux/math.h>


static ktime_t ET;
static ktime_t Period;

spinlock_t my_lock;

static enum hrtimer_restart my_hrtimer_callback(struct hrtimer *timer){

    // printk("Timer Callback function Called\n");
    struct task_struct *t = container_of(timer,struct task_struct, hrtimer);
    wake_up_process(t);
    if (spin_trylock(&my_lock)){
        if (t != NULL || (t->__state == TASK_RUNNING ||  t->__state == TASK_INTERRUPTIBLE || t->__state == TASK_UNINTERRUPTIBLE)){
            // printk("task exists\n");
            t->cmt = ktime_set(0,0);
            hrtimer_forward_now(timer,t->T);
            spin_unlock(&my_lock);
            return HRTIMER_RESTART;
        }
        spin_unlock(&my_lock);
        return HRTIMER_NORESTART;   
        
    }
    return HRTIMER_RESTART;

 
}

SYSCALL_DEFINE3(set_rtmon, pid_t, pid, unsigned int, C_ms, unsigned int, T_ms)
{
    ET = ktime_set(0, C_ms * NSEC_PER_MSEC);
    Period = ktime_set(0, T_ms * NSEC_PER_MSEC);
    int R1,R2 = 0;
    struct task_struct *p,*t;
    //check the parameter contraints 
    if(C_ms <=10000 && T_ms <= 10000 && C_ms >= 1 && T_ms >= 1){  //maximum allowed is 10000 not 1000 
        for_each_process_thread(p,t){
            if(t->pid == pid){
                if(t->C != 0 && t->T !=0){
                    printk("The task Timing parameters are non-zero\n");
                    return -1;

                }else{
                    R1 = ET;
                    while (R1 != R2 && R2 <= Period){
                        R1 = R2;
                        R2 = ET;
                        for_each_process_thread(p,t){
                            if (t->C > 0 && t->T > 0 && pid_alive(t)){
                                if (t->T < Period){
                                    R2 += ((((R1) + (t->T) -1)/(t->T)) * t->C);

                                }

                            }

                        }
                        
                    }if(R1 == R2){
                        for_each_process_thread(p,t){
                            if(t->pid == pid){
                                t->C = ET;
                                t->T = Period;

                                // printk("The Values are added to the tcb %lld, %lld , %d\n", t->C, t->T, t->pid);
                                hrtimer_init(&t->hrtimer, CLOCK_MONOTONIC, HRTIMER_MODE_ABS_PINNED);
                                printk("Timer Initialized\n");
                                t->hrtimer.function = &my_hrtimer_callback;
                                printk("Timer is starting\n");
                                hrtimer_start(&t->hrtimer, Period, HRTIMER_MODE_ABS_PINNED);  
                                return 0;
                            }
                        }
                    }
                    printk("Doesnot Satify Responsiveness Test\n");
                    return 0;    


                }

            }

        }
        printk("The Pid Does not Exist\n");
        return -1;
        
    }else {
        printk("The Values provided are invalid\n");
        return -1;
    }
}

SYSCALL_DEFINE1(cancel_rtmon, pid_t, pid)
{
    struct task_struct *p,*t;
    for_each_process_thread(p,t){
        if(t->pid == pid){
            if(t->C !=0 && t->T !=0){
                t->C = ktime_set(0,0);
                t->T = ktime_set(0,0);
                t->cmt = ktime_set(0,0);
                hrtimer_cancel(&t->hrtimer);
                wake_up_process(t);
                printk("Pids are set to zero\n"); 
                return 0;              
            }else{
                printk("The timing parameters are already zero\n");
                return -1;
            }

        }
        }
        printk("The PID doesnot Exist\n");
        return -1;
}

SYSCALL_DEFINE1(print_rtmon, pid_t, pid)
{
    struct task_struct *p,*t;
    if(pid >= 0){
        for_each_process_thread(p,t){
            if(t->pid == pid){

                ktime_t C_ms = (t->C/1000000);
                ktime_t T_ms = (t->T/1000000);
                ktime_t cmt = t->cmt;
                printk("print_rtmon: PID %d, C %lld ms, T %lld ms, usage %lld ms\n", t->pid, C_ms, T_ms, (cmt/1000000));
                return 0;

            }
            }
            printk("Task Doesnot Exist\n");
        }

    if(pid == -1){
        for_each_process_thread(p,t){
            ktime_t C_ms = (t->C/1000000);
            ktime_t T_ms = (t->T/1000000);
            printk("print_rtmon: PID %d, C %lld ms, T %lld ms, usage %lld ms\n", t->pid, C_ms, T_ms, (t->cmt/1000000));            
            printk("print_rtmon: PID %d, C %lld ms, T %lld ms\n", t->pid, C_ms, T_ms);
        }
    }else{
        printk("Error!!!!!");
        return -1;
    }

    return 0;
}
SYSCALL_DEFINE0(wait_until_next_period)
{
printk("syscall 453!\n");
    
    struct task_struct *t = current;
    if(t != NULL){
    // printk("Find tcb and Hit it! pid: %d, C: %lld, T:%lld\n", t->pid, t->C/1000000, t->T/1000000);
    if(t->C !=0 && t->T != 0){
        // printk("Find tcb and Hit it! pid: %d, C: %lld, T:%lld\n", t->pid, t->C/1000000, t->T/1000000);
        //suspend the task 
        t->__state = TASK_INTERRUPTIBLE;
        schedule();
        return 0;
    }
    printk("Hit it!!\n");
    return -1;
    }
    printk("Hit it!\n");
    return -1;
}