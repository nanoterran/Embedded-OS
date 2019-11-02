#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Javier Vega");
MODULE_DESCRIPTION("Finds a task.");
MODULE_VERSION("1.00");

static int __init find_task_init(void) {
  
  printk(KERN_INFO "Module has been Loaded!");

  return 0;
}
static void __exit find_task_exit(void) {
  printk(KERN_INFO "Unloading FindTask module\n");
}

module_init(find_task_init);
module_exit(find_task_exit);