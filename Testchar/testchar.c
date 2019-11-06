
#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>

#define DEVICE_NAME "testchar"
#define CLASS_NAME  "test"

MODULE_AUTHOR("Javier Vega");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("A simple Linux char driver");
MODULE_VERSION("0.1");

static int    majorNumber;                   ///< Stores the device number -- determined automatically
static char   message[256] = {0};            ///< Memory for the string that is passed from userspace
static short  size_of_message;               ///< Used to remember the size of the string stored
static int    numberOpens = 0;               ///< Counts the number of times the device is opened

static struct class *testcharClass = NULL;   ///< The device-driver class struct pointer
static struct device *testcharDevice = NULL; ///< The device-driver device struct pointer

// The prototype functions for the character driver -- must come before the struct definition
static int     dev_open(struct inode *, struct file *);
static int     dev_release(struct inode *, struct file *);
static ssize_t dev_read(struct file *, char *, size_t, loff_t *);
static ssize_t dev_write(struct file *, const char *, size_t, loff_t *);

/** @brief Devices are represented as file structure in the kernel. The file_operations structure 
 *  from /linux/fs.h lists the callback functions that you wish to associated with your file operations
 *  using a C99 syntax structure. char devices usually implement open, read, write and release calls
 */
static struct file_operations file_operations_t = {
  .open = dev_open,
  .read = dev_read,
  .write = dev_write,
  .release = dev_release,
};

static int __init testchar_init(void){
  printk(KERN_INFO "TestChar: Initializing the TestChar LKM\n");

  // Try to dynamically allocate a major number for the device -- more difficult but worth it
  majorNumber = register_chrdev(0, DEVICE_NAME, &file_operations_t);
  if(majorNumber < 0)
  {
    printk(KERN_ALERT "TestChar failed to register a major number\n");

    return majorNumber;
  }
  printk(KERN_INFO "TestChar: registered correctly with major number %d\n", majorNumber);

  // Register the device class
  testcharClass = class_create(THIS_MODULE, CLASS_NAME);
  if(IS_ERR(testcharClass))
  {
    unregister_chrdev(majorNumber, DEVICE_NAME);

    printk(KERN_ALERT "Failed to register device class\n");

    return PTR_ERR(testcharClass);
  }
  printk(KERN_INFO "TestChar: device class registered correctly\n");

  // Register the device driver
  testcharDevice = device_create(testcharClass, NULL, MKDEV(majorNumber, 0), NULL, DEVICE_NAME);
  if (IS_ERR(testcharDevice))
  {
    // Repeated code but the alternative is goto statements
    class_destroy(testcharClass);
    unregister_chrdev(majorNumber, DEVICE_NAME);

    printk(KERN_ALERT "Failed to create the device\n");

    return PTR_ERR(testcharDevice);
  }
  printk(KERN_INFO "TestChar: device class created correctly\n");

  return 0;
}

/**
 * Allows users to open the device driver
 */
static int dev_open(struct inode *inode_ptr, struct file *file_ptr)
{

}

/**
 * Allows users to close the device driver.
 */
static int dev_release(struct inode *inode_ptr, struct file *file_ptr)
{

}

/**
 * Allows the device driver to send data to user programs.
 */
static ssize_t dev_read(struct file *file_ptr, char *data, size_t data_size, loff_t *offset_ptr)
{

}

/**
 * Allows user programs to write data to the device driver.
 */
static ssize_t dev_write(struct file *file_ptr, const char *data, size_t data_size, loff_t *offset_ptr)
{

}

static void __exit testchar_exit(void){
  device_destroy(testcharClass, MKDEV(majorNumber, 0));     // remove the device
  class_unregister(testcharClass);                          // unregister the device class
  class_destroy(testcharClass);                             // remove the device class
  unregister_chrdev(majorNumber, DEVICE_NAME);             // unregister the major number

  printk(KERN_INFO "TestChar: Goodbye from the LKM!\n");
}