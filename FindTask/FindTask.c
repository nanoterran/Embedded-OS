#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/string.h>
#include <linux/sched/signal.h>   //Includes for_each_process()

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Javier Vega");
MODULE_DESCRIPTION("Finds a task.");
MODULE_VERSION("1.00");

static char *name = "none";
module_param(name, charp, S_IRUGO);
MODULE_PARM_DESC(name, "The name of the process to find");  

static int __init find_task_init(void) {
  struct task_struct *task_list;

  // Traverses the list of tasks until we find the process we are looking for.
  for_each_process(task_list) {
    if(!strcmp(name, task_list->comm))
    {
      printk(KERN_INFO "Found process %s with pid %d", task_list->comm, task_list->pid);
      return 0;
    }
  }
  printk(KERN_INFO "Not Found process %s", name);

  return 0;
}
static void __exit find_task_exit(void) {
  printk(KERN_INFO "Unloading FindTask module\n");
}

module_init(find_task_init);
module_exit(find_task_exit);