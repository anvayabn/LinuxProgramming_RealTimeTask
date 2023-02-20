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
                t->C = 0;
                t->T = 0;
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
                printk("print_rtmon: PID %d, C %lld ms, T %lld ms\n", t->pid, C_ms, T_ms);
                return 0;

            }
            }
            printk("Task Doesnot Exist\n");
        }

    if(pid == -1){
        for_each_process_thread(p,t){
            ktime_t C_ms = (t->C/1000000);
            ktime_t T_ms = (t->T/1000000);
            printk("print_rtmon: PID %d, C %lld ms, T %lld ms\n", t->pid, C_ms, T_ms);
        }
    }else{
        printk("Error!!!!!");
        return -1;
    }

    return 0;
}