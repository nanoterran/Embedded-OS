
#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/uaccess.h>
#include "commands.h"

#define DEVICE_NAME "testchar"
#define CLASS_NAME  "test"

MODULE_AUTHOR("Javier Vega");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("A simple Linux char driver");
MODULE_VERSION("0.1");

static int   major_number;                  ///< Stores the device number -- determined automatically
static char  message[256] = {0};            ///< Memory for the string that is passed from userspace
static short size_of_message;               ///< Used to remember the size of the string stored
static int   number_of_opens = 0;           ///< Counts the number of times the device is opened
static int   device_mode = TESTCHAR_NONE;

static struct class  *testchar_class = NULL;   ///< The device-driver class struct pointer
static struct device *testchar_device = NULL;  ///< The device-driver device struct pointer

// The prototype functions for the character driver -- must come before the struct definition
static int     dev_open(struct inode *, struct file *);
static int     dev_release(struct inode *, struct file *);
static ssize_t dev_read(struct file *, char *, size_t, loff_t *);
static ssize_t dev_write(struct file *, const char *, size_t, loff_t *);
static long     dev_ioctl(struct file *, unsigned int, unsigned long);

/** @brief Devices are represented as file structure in the kernel. The file_operations structure 
 *  from /linux/fs.h lists the callback functions that you wish to associated with your file operations
 *  using a C99 syntax structure. char devices usually implement open, read, write and release calls
 */
static struct file_operations file_operations_t = {
  .open =    dev_open,
  .read =    dev_read,
  .write =   dev_write,
  .release = dev_release,
  .unlocked_ioctl =   dev_ioctl
};

static int __init testchar_init(void)
{
  printk(KERN_INFO "TestChar: Initializing the TestChar LKM\n");

  // Try to dynamically allocate a major number for the device -- more difficult but worth it
  major_number = register_chrdev(0, DEVICE_NAME, &file_operations_t);
  if(major_number < 0)
  {
    printk(KERN_ALERT "TestChar failed to register a major number\n");

    return major_number;
  }
  printk(KERN_INFO "TestChar: registered correctly with major number %d\n", major_number);

  // Register the device class
  testchar_class = class_create(THIS_MODULE, CLASS_NAME);
  if(IS_ERR(testchar_class))
  {
    unregister_chrdev(major_number, DEVICE_NAME);

    printk(KERN_ALERT "Failed to register device class\n");

    return PTR_ERR(testchar_class);
  }
  printk(KERN_INFO "TestChar: device class registered correctly\n");

  // Register the device driver
  testchar_device = device_create(testchar_class, NULL, MKDEV(major_number, 0), NULL, DEVICE_NAME);
  if (IS_ERR(testchar_device))
  {
    // Repeated code but the alternative is goto statements
    class_destroy(testchar_class);
    unregister_chrdev(major_number, DEVICE_NAME);

    printk(KERN_ALERT "Failed to create the device\n");

    return PTR_ERR(testchar_device);
  }
  printk(KERN_INFO "TestChar: device class created correctly\n");

  return 0;
}

/**
 * Allows users to open the device driver
 */
static int dev_open(struct inode *inode_ptr, struct file *file_ptr)
{
  printk(KERN_INFO "TestChar: Driver have been opened\n");

  number_of_opens++;

  return 0;   // Successfully opened
}

/**
 * Allows users to close the device driver.
 */
static int dev_release(struct inode *inode_ptr, struct file *file_ptr)
{
  printk(KERN_INFO "TestChar: Device successfully closed\n");

  return 0;   // Sucessfully released
}

/**
 * Allows the device driver to send data to user programs.
 */
static ssize_t dev_read(struct file *file_ptr, char *user_buffer, size_t data_size, loff_t *offset_ptr)
{
  int error_number = 0;

  // copy_to_user has the format ( * to, * from, size) and returns 0 on success
  error_number = copy_to_user(user_buffer, message, size_of_message);

  // if true then have success
  if(error_number == 0)
  {
    printk(KERN_INFO "TestChar: Sent %d characters to the user\n", size_of_message);

    // size_of_message = 0;
    return 0;
  }
  else
  {
    printk(KERN_INFO "TestChar: Could not send %d characters to the user\n", error_number);
    return -EFAULT;
  }
}

/**
 * Allows user programs to write data to the device driver.
 */
static ssize_t dev_write(struct file *file_ptr, const char *data, size_t data_size, loff_t *offset_ptr)
{
  if(data_size <= 0)
  {
    return 0;
  }
  printk(KERN_INFO "TestChar: Received %lu characters from the user\n", data_size);

  sprintf(message, "%s(%lu letters)", data, data_size);
  size_of_message = strlen(message);

  return data_size;
}

static long dev_ioctl(struct file *file_ptr, unsigned int command, unsigned long arg)
{
  switch(command)
  {
    case TESTCHAR_NONE:
      device_mode = TESTCHAR_NONE;
      printk(KERN_INFO "TestChar: Mode Changed to None\n");
      break;
    case TESTCHAR_ALLCAPS:
      device_mode = TESTCHAR_ALLCAPS;
      printk(KERN_INFO "TestChar: Mode Changed to ALLCAPS\n");
      break;
    default:
      return -ENOTTY; 
  }
  return 0;
}

static void __exit testchar_exit(void)
{
  device_destroy(testchar_class, MKDEV(major_number, 0));     // remove the device
  class_unregister(testchar_class);                           // unregister the device class
  class_destroy(testchar_class);                              // remove the device class
  unregister_chrdev(major_number, DEVICE_NAME);               // unregister the major number

  printk(KERN_INFO "TestChar: Goodbye from the Device Driver!\n");
}

module_init(testchar_init);
module_exit(testchar_exit);