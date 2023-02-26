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
    struct task_struct *t = container_of(timer,struct task_struct, hrtimer);
    if (t == NULL){
        // printk("task pointer is NULL \n");
        return HRTIMER_NORESTART;
    }
    else if (t != NULL){
        if(t->C == 0 || t->T == 0 || !pid_alive(t)){
            
            // printk("Due to do_exit() set C and T to zero, cancel hrtimer & budget_hrtimer\n");
            return HRTIMER_NORESTART;
        }
    if (spin_trylock(&my_lock)){
        if (pid_alive(t)){
            wake_up_process(t);
            // printk("Refrech hrtimer\n");
            t->cmt = ktime_set(0,0);
            hrtimer_forward_now(timer,t->T);
            hrtimer_start(&t->budget_hrtimer, t->C, HRTIMER_MODE_ABS_PINNED);  
            spin_unlock(&my_lock);
            return HRTIMER_RESTART;
        }else if(t->exit_state == EXIT_DEAD || t->exit_state == EXIT_ZOMBIE || t->__state == __TASK_STOPPED || t->__state == __TASK_TRACED){
            // printk("The task has exited and hrtimer will stopped\n");
            t->C = ktime_set(0,0); 
            t->T = ktime_set(0,0); 
            t->cmt = ktime_set(0,0); 
            int x = 1;
            if(x == 1){
                // printk("Timer was cancelled\n");
                spin_unlock(&my_lock);
                return HRTIMER_NORESTART;
            }else if(x == 0){
                // printk("Timer Tried to cancel\n");
                spin_unlock(&my_lock);
                return HRTIMER_NORESTART;
            }else if (x<0){
                // printk("Timer Cancell Error\n");
                spin_unlock(&my_lock);
                return HRTIMER_NORESTART;
            }             
        }       
    }
    }
    return HRTIMER_NORESTART;
}


static enum hrtimer_restart budget_hrtimer_callback(struct hrtimer *timer){

    // printk("Budget Timer Callback function Called\n");
    struct task_struct *t = container_of(timer,struct task_struct, budget_hrtimer);
    if (t == NULL){
        // printk("Budget Callback: task pointer is NULL \n");
        if (hrtimer_active(timer))
        return HRTIMER_NORESTART;
    }
    if(t->C == 0 || t->T == 0 || !pid_alive(t) || t->exit_state == EXIT_DEAD || t->exit_state == EXIT_ZOMBIE || t->__state == __TASK_STOPPED || t->__state == __TASK_TRACED){
        // printk("do_exit() set C and T to zero, cancel hrtimer & budget_hrtimer\n");
            t->C = ktime_set(0,0); 
    t->T = ktime_set(0,0); 
    t->cmt = ktime_set(0,0); 
        return HRTIMER_NORESTART;
    }
    if (spin_trylock(&my_lock)){
    
    long long remain_time = t->C - t->cmt;
    if(t->sot <= t->sit){
        remain_time  = remain_time - (ktime_get_ns() - t->sit);
    }
    if(remain_time < 0){
    // printk("Run out of Budget: Interrupt\n");
    t->__state = TASK_INTERRUPTIBLE;
    set_tsk_need_resched(t);
    spin_unlock(&my_lock);
    return HRTIMER_NORESTART;
    }
    else if(remain_time == 0){
    // printk("We finished!\n");
    t->__state = TASK_INTERRUPTIBLE;
    set_tsk_need_resched(t);
    spin_unlock(&my_lock);
    return HRTIMER_NORESTART;
    }
    else{
        // printk("Budget is efficient, extend the expiration\n");
        hrtimer_forward_now(timer, remain_time);
        spin_unlock(&my_lock);
        return HRTIMER_RESTART;
    }
    }
    return HRTIMER_NORESTART; 
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
                    // printk("The task Timing parameters are non-zero\n");
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
                                hrtimer_init(&t->budget_hrtimer, CLOCK_MONOTONIC, HRTIMER_MODE_ABS_PINNED);
                                // printk("Timer Initialized\n");
                                t->hrtimer.function = &my_hrtimer_callback;
                                t->budget_hrtimer.function = &budget_hrtimer_callback;
                                // printk("Timer is starting\n");
                                hrtimer_start(&t->hrtimer, Period, HRTIMER_MODE_ABS_PINNED);  
                                hrtimer_start(&t->budget_hrtimer, ET, HRTIMER_MODE_ABS_PINNED);  
                                return 0;
                            }
                        }
                    }    


                }

            }

        }
        // printk("The Pid Does not Exist\n");
        return -1;
        
    }else {
        // printk("The Values provided are invalid\n");
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
                int x; 
                if (hrtimer_active(&t->hrtimer))
                    x = hrtimer_cancel(&t->hrtimer);
                if (hrtimer_active(&t->budget_hrtimer))
                    x = hrtimer_cancel(&t->budget_hrtimer);
                wake_up_process(t);
                // printk("Pids are set to zero\n"); 
                if (x == 0 || x == 1){
                    // printk("timer is cancelled\n");
                    return 0;
                }else {
                    // printk("Failed to cancel timer\n");
                    return -1;
                }
                          
            }else{
                // printk("The timing parameters are already zero\n");
                return -1;
            }

        }
        }
        // printk("The PID doesnot Exist\n");
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
                if(t->sot <= t->sit){
                     cmt += (ktime_get_ns() - t->sit);
                }
                cmt = cmt / 1000000;
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
// printk("syscall 453!\n");
    
    struct task_struct *t = current;
    if(t != NULL){
    // printk("Find tcb and Hit it! pid: %d, C: %d, T:%d\n", t->pid, t->C/1000000, t->T/1000000);
    if(t->C !=0 && t->T != 0){
        // printk("Find tcb and Hit it! pid: %d, C: %d, T:%d\n", t->pid, t->C/1000000, t->T/1000000);
        //suspend the task 
        t->__state = TASK_INTERRUPTIBLE;
        schedule();
        return 0;
    }
    }
    // printk("Hit it!\n");
    return -1;
}