#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/sched/signal.h>   //Includes for_each_process()

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Javier Vega");
MODULE_DESCRIPTION("Finds a task.");
MODULE_VERSION("1.00");

static int __init find_task_init(void) {
  struct task_struct *task_list;

  for_each_process(task_list) {
    printk(KERN_INFO "%s: %d\n", task_list->comm, task_list->pid);
  }
  printk(KERN_INFO "Module Loaded");
  return 0;
}
static void __exit find_task_exit(void) {
  printk(KERN_INFO "Unloading FindTask module\n");
}

module_init(find_task_init);
module_exit(find_task_exit);