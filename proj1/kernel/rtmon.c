#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/sched.h>
#include <linux/hrtimer.h>
#include <linux/math.h>


static ktime_t ET;
static ktime_t Period;

// #ifndef NR_PID_MAX
// #define NR_PID_MAX 32768
// #endif
spinlock_t my_lock;

static enum hrtimer_restart my_hrtimer_callback(struct hrtimer *timer){

    // printk("Timer Callback function Called\n");
    struct task_struct *t = container_of(timer,struct task_struct, hrtimer);
    if (spin_trylock(&my_lock)){
        if (t != NULL){
            // printk("task exists\n");
            t->cmt = ktime_set(0,0);
            hrtimer_forward_now(timer,t->T);
            spin_unlock(&my_lock);
            return HRTIMER_RESTART;
        }else if(t == NULL || t->exit_state == EXIT_DEAD || t->exit_state == EXIT_ZOMBIE || t->__state == __TASK_STOPPED || t->__state == __TASK_TRACED){
            printk("The task has exited and hr timer will stopped\n");
            t->cmt = ktime_set(0,0); 
            int x = hrtimer_cancel(&t->hrtimer);
            if(x == 1){
                printk("Timer was cancelled\n");
                spin_unlock(&my_lock);
                return HRTIMER_NORESTART;
            }else if(x == 0){
                printk("Timer Tried to cancel\n");
                spin_unlock(&my_lock);
                return HRTIMER_NORESTART;
            }else if (x<0){
                printk("Timer Cancell Error\n");
                spin_unlock(&my_lock);
                return HRTIMER_NORESTART;
            }             
        }       
        
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
    if(C_ms <=1000 && T_ms <= 1000 && C_ms >= 1 && T_ms >= 1){
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

                                printk("The Values are added to the tcb %lld, %lld , %d\n", t->C, t->T, t->pid);
                                hrtimer_init(&t->hrtimer, CLOCK_MONOTONIC, HRTIMER_MODE_ABS_PINNED);
                                printk("Timer Initialized\n");
                                t->hrtimer.function = &my_hrtimer_callback;
                                printk("Timer is starting\n");
                                hrtimer_start(&t->hrtimer, Period, HRTIMER_MODE_ABS_PINNED);  
                                return 0;
                            }
                        }
                    }    


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
                int x = hrtimer_cancel(&t->hrtimer);
                printk("Pids are set to zero\n"); 
                if (x == 0 || x == 1){
                    printk("timer is cancelled\n");
                    return 0;
                }else {
                    printk("Failed to cancel timer\n");
                    return -1;
                }
                          
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
                printk("print_rtmon: PID %d, C %lld ms, T %lld ms\n, usage %lld ms", t->pid, C_ms, T_ms, (t->cmt/1000000));
                return 0;

            }
            }
            printk("Task Doesnot Exist\n");
        }

    if(pid == -1){
        for_each_process_thread(p,t){
            ktime_t C_ms = (t->C/1000000);
            ktime_t T_ms = (t->T/1000000);
            printk("print_rtmon: PID %d, C %lld ms, T %lld ms\n, usage %lld ms", t->pid, C_ms, T_ms, (t->cmt/1000000));            
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
    struct task_struct *t = current;
    if(t->C !=0 || t->T != 0){
        //suspend the task 
        t->__state = TASK_INTERRUPTIBLE;
        set_tsk_need_resched(t);
        //wait for the callback function to execute 
        ktime rem = hrtimer_get_remaining(&t->hrtimer);
        while(rem != 0){
            rem = hrtimer_get_remaining(&t->hrtimer);
        }
        // wakeup the process
        wake_up_process(t);
        retuurn 0;
    }
    return -1;


}