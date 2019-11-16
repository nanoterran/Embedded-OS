#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/uaccess.h>
#include <linux/ctype.h>
#include <linux/mutex.h>
#include <asm/io.h>

#define DEVICE_NAME "MorseCode"
#define CLASS_NAME  "Morse"
#define CQ_DEFAULT	0

MODULE_AUTHOR("Javier Vega");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("A character device driver for morse code.");
MODULE_VERSION("1.0");

/* the empty string, follwed by 26 letter codes, followed by the 10 numeral codes, followed by the comma,
   period, and question mark.  */

char *morse_code[40] =
  {
    "", ".-","-...","-.-.","-..",".","..-.","--.","....","..",
    ".---","-.-",".-..","--","-.","---",".--.","--.-",".-.",
    "...","-","..-","...-",".--","-..-","-.--","--..","-----",
    ".----","..---","...--","....-",".....","-....","--...",
    "---..","----.","--..--","-.-.-.","..--.."
  };

/**
 * A macro that is used to declare a new mutex that is visible in this file
 * results in a semaphore variable ebbchar_mutex with value 1 (unlocked)
 * DEFINE_MUTEX_LOCKED() results in a variable with value 0 (locked)
 */
static DEFINE_MUTEX(morse_mutex);

static int major_number;                       ///< Stores the device number -- determined automatically
static struct class  *morse_class = NULL;      ///< The device-driver class struct pointer
static struct device *morse_device = NULL;     ///< The device-driver device struct pointer
static struct timer_list timer;
static int led_state;

// The prototype functions for the character driver -- must come before the struct definition
static int     dev_open(struct inode *, struct file *);
static int     dev_release(struct inode *, struct file *);
static ssize_t dev_read(struct file *, char *, size_t, loff_t *);
static ssize_t dev_write(struct file *, const char *, size_t, loff_t *);
static long    dev_ioctl(struct file *, unsigned int, unsigned long);
static void    set_led_callback(unsigned long value);
static inline char *mcodestring(int asciicode);

/** @brief Devices are represented as file structure in the kernel. The file_operations structure 
 *  from /linux/fs.h lists the callback functions that you wish to associated with your file operations
 *  using a C99 syntax structure. char devices usually implement open, read, write and release calls
 */
static struct file_operations file_operations_t = {
  .open =           dev_open,
  .read =           dev_read,
  .write =          dev_write,
  .release =        dev_release,
  .unlocked_ioctl = dev_ioctl
};

static int __init morse_init(void)
{
  printk(KERN_INFO "MorseCode: Initializing the MorseCode LKM\n");

  // Try to dynamically allocate a major number for the device -- more difficult but worth it
  major_number = register_chrdev(0, DEVICE_NAME, &file_operations_t);
  if(major_number < 0)
  {
    printk(KERN_ALERT "MorseCode: failed to register a major number\n");

    return major_number;
  }
  printk(KERN_INFO "MorseCode: registered correctly with major number %d\n", major_number);

  // Register the device class
  morse_class = class_create(THIS_MODULE, CLASS_NAME);
  if(IS_ERR(morse_class))
  {
    unregister_chrdev(major_number, DEVICE_NAME);

    printk(KERN_ALERT "MorseCode: Failed to register device class\n");

    return PTR_ERR(morse_class);
  }
  printk(KERN_INFO "MorseCode: device class registered correctly\n");

  // Register the device driver
  morse_device = device_create(morse_class, NULL, MKDEV(major_number, 0), NULL, DEVICE_NAME);
  if (IS_ERR(morse_device))
  {
    // Repeated code but the alternative is goto statements
    class_destroy(morse_class);
    unregister_chrdev(major_number, DEVICE_NAME);

    printk(KERN_ALERT "MorseCode: Failed to create the device\n");

    return PTR_ERR(morse_device);
  }
  printk(KERN_INFO "MorseCode: device class created correctly\n");

  mutex_init(&morse_mutex);       /// Initialize the mutex lock dynamically at runtime

  led_state = 0;

  init_timer(&timer);

  // Set th timer to expire in one second
  timer.expires = jiffies + HZ;
  timer.data = NULL;
  timer.function = set_led_callback;

  // Registers the timer
  add_timer(&timer);

  return 0;
}

/**
 * Allows users to open the device driver
 */
static int dev_open(struct inode *inode_ptr, struct file *file_ptr)
{
  return 0;   // Successfully opened
}

/**
 * Allows users to close the device driver.
 */
static int dev_release(struct inode *inode_ptr, struct file *file_ptr)
{
  return 0;   // Sucessfully released
}

/**
 * Allows the device driver to send data to user programs.
 */
static ssize_t dev_read(struct file *file_ptr, char *user_buffer, size_t data_size, loff_t *offset_ptr)
{
  return 0;
}

/**
 * Allows user programs to write data to the device driver.
 */
static ssize_t dev_write(struct file *file_ptr, const char *data, size_t data_size, loff_t *offset_ptr)
{
  return 0;
}

/**
 * Allows the user to set mode of the driver
 */
static long dev_ioctl(struct file *file_ptr, unsigned int command, unsigned long arg)
{
  return 0;
}

static void set_led_callback(unsigned long value)
{
  unsigned long gpio1_base_address = 0x4804C000;
  unsigned long gpio1_base_end_address = 0x4804E000;
  unsigned long register_offset = 0x13C;
  unsigned long clear_register_offset = 0x190;

  char *base_address_ptr = ioremap(gpio1_base_address, gpio1_base_end_address - gpio1_base_address);

  if(led_state == 0)
  {
    unsigned long *reg = (long)base_address_ptr + register_offset;
    *reg = *reg | (1<<21);
    printk(KERN_INFO "MorseCode: SetData GPIO1 Register = %d\n", *reg);
    led_state = 1;
  }
  else
  {
    unsigned long *reg = (long)base_address_ptr + clear_register_offset;
    *reg = *reg | (1<<21);
    printk(KERN_INFO "MorseCode: Clear GPIO1 Register = %d\n", *reg);
    led_state = 0;
  }
  
  timer.expires += HZ; // add another delay period
  add_timer(&timer);
}

/**
 * Maps the ascii value to its morse code string.
 */
static inline char *mcodestring(int asciicode)
{
   char *mc;   // this is the mapping from the ASCII code into the mcodearray of strings.

   if (asciicode > 122)  // Past 'z'
      mc = morse_code[CQ_DEFAULT];
   else if (asciicode > 96)  // Upper Case
      mc = morse_code[asciicode - 96];
   else if (asciicode > 90)  // uncoded punctuation
      mc = morse_code[CQ_DEFAULT];
   else if (asciicode > 64)  // Lower Case 
      mc = morse_code[asciicode - 64];
   else if (asciicode == 63)  // Question Mark
      mc = morse_code[39];    // 36 + 3 
   else if (asciicode > 57)  // uncoded punctuation
      mc = morse_code[CQ_DEFAULT];
   else if (asciicode > 47)  // Numeral
      mc = morse_code[asciicode - 21];  // 27 + (asciicode - 48) 
   else if (asciicode == 46)  // Period
      mc = morse_code[38];  // 36 + 2 
   else if (asciicode == 44)  // Comma
      mc = morse_code[37];   // 36 + 1
   else
      mc = morse_code[CQ_DEFAULT];
   return mc;
}

static void __exit morse_exit(void)
{
  device_destroy(morse_class, MKDEV(major_number, 0)); // remove the device
  class_unregister(morse_class);                       // unregister the device class
  class_destroy(morse_class);                          // remove the device class
  unregister_chrdev(major_number, DEVICE_NAME);        // unregister the major number
  mutex_destroy(&morse_mutex);                         // destroy the dynamically-allocated mutex
  del_timer(&timer);

  printk(KERN_INFO "MorseCode: Goodbye from the Morse Code Device Driver!\n");
}

module_init(morse_init);
module_exit(morse_exit);