#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/string.h>
#include <linux/sched/signal.h>   //Includes for_each_process()
#include <linux/timer.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Javier Vega");
MODULE_DESCRIPTION("Finds the pid of a process given its name.");
MODULE_VERSION("1.00");

static char *name = "none";
module_param(name, charp, S_IRUGO);
MODULE_PARM_DESC(name, "The name of the process to find");  

static struct timer_list timer;

static void find_task_callback(struct timer_list *another_timer)
{
  struct task_struct *current_task;
  int found = 0;
  pid_t pid = 0; 

  // Traverses the list of tasks until we find the process we are looking for.
  for_each_process(current_task) {
    if(!strcmp(name, current_task->comm))
    {
      pid = current_task->pid;
      found = 1;
    }
  }

  if(found)
    printk("Found process %s with pid %d\n", name, pid);
  else
    printk("Not Found process %s\n", name);

  // // timers are one-shot, so we have to set-up the next timer event:
  // timer.expires += HZ; // add another delay period
  // add_timer(&timer);
}

static int __init find_task_init(void) {
  // Intializes a timer
  // init_timers(&timer);

  timer.expires = jiffies + HZ;   // expire in one second

  // Sets up a timer
  timer_setup(&timer, &find_task_callback, 0);

  // Registers the timer
  add_timer(&timer);

  printk(KERN_INFO "FindTaskTimer Initialized\n");

  return 0;
}
static void __exit find_task_exit(void) {
  // Delete the registered timer to give resources back to kernel
  del_timer(&timer);

  printk(KERN_INFO "FindTaskTimer module removed\n");
}

module_init(find_task_init);
module_exit(find_task_exit);