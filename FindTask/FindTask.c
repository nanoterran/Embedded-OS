#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/string.h>
#include <linux/sched/signal.h>   //Includes for_each_process()

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Javier Vega");
MODULE_DESCRIPTION("Finds the pid of a process given its name.");
MODULE_VERSION("1.00");

static char *name = "none";
module_param(name, charp, S_IRUGO);
MODULE_PARM_DESC(name, "The name of the process to find");  

static int __init find_task_init(void) {
  struct task_struct *current_task;
  int found = 0;
  pid_t pid = 0; 

  // Traverses the list of tasks until we find the process we are looking for.
  for_each_process(current_task) {
    if(!strcmp(name, current_task->comm))
    {
      // printk(KERN_WARNING "Found process %s with pid %d", current_task->comm, current_task->pid);
      pid = current_task->pid;
      found = 1;
    }
  }

  if(found)
    printk("Found process %s with pid %d", name, pid);
  else
    printk("Not Found process %s", name);

  printk(KERN_INFO "");

  return 0;
}
static void __exit find_task_exit(void) {
  printk(KERN_INFO "Removing FindTask module\n");
}

module_init(find_task_init);
module_exit(find_task_exit);