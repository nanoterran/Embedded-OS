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

/**
 * The empty string, follwed by 26 letter codes,
 * followed by the 10 numeral codes, followed by the comma,
 * period, and question mark.
 */
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

static int            major_number;        ///< Stores the device number -- determined automatically
static struct class  *morse_class = NULL;  ///< The device-driver class struct pointer
static struct device *morse_device = NULL; ///< The device-driver device struct pointer

// The prototype functions for the character driver -- must come before the struct definition
static int          dev_open(struct inode *, struct file *);
static int          dev_release(struct inode *, struct file *);
static ssize_t      dev_read(struct file *, char *, size_t, loff_t *);
static ssize_t      dev_write(struct file *, const char *, size_t, loff_t *);
static long         dev_ioctl(struct file *, unsigned int, unsigned long);
static void         set_led_callback(unsigned long value);
static inline char *mcodestring(int asciicode);

/** @brief Devices are represented as file structure in the kernel. The file_operations structure 
 *  from /linux/fs.h lists the callback functions that you wish to associated with your file operations
 *  using a C99 syntax structure. char devices usually implement open, read, write and release calls
 */
static struct file_operations file_operations_t =
{
  .open           = dev_open,
  .read           = dev_read,
  .write          = dev_write,
  .release        = dev_release,
  .unlocked_ioctl = dev_ioctl
};

/**
 * Device Drivers global variables
 */
static struct timer_list timer;
static int               led_state;
static char              message[256] = {0};  ///< Memory for the string that is passed from userspace
static short             size_of_message;     ///< Used to remember the size of the string stored
static int               number_of_opens = 0; ///< Counts the number of times the device is opened

/** @brief The LKM initialization function
 *  The static keyword restricts the visibility of the function to within this C file. The __init
 *  macro means that for a built-in driver (not a LKM) the function is only used at initialization
 *  time and that it can be discarded and its memory freed up after that point.
 *  @return returns 0 if successful
 */
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

  // Initialize the mutex lock dynamically at runtime
  mutex_init(&morse_mutex);

  led_state = 0;

  // Way to initialize a timer for older kernel version
  init_timer(&timer);
  timer.expires = jiffies + HZ;      // Expires every one second
  timer.data = NULL;
  timer.function = set_led_callback;
  add_timer(&timer);                 // Registers the timer

  return 0;
}

/** @brief The device open function that is called each time the device is opened
 *  This will only increment the numberOpens counter in this case.
 *  @param inode_ptr A pointer to an inode object (defined in linux/fs.h)
 *  @param file_ptr A pointer to a file object (defined in linux/fs.h)
 */
static int dev_open(struct inode *inode_ptr, struct file *file_ptr)
{
  /**
   * Try to acquire the mutex (i.e., put the lock on/down)
   * returns 1 if successful and 0 if there is contention
   */
  if(!mutex_trylock(&testchar_mutex)){
    printk(KERN_ALERT "TestChar: Device in use by another process\n");

    return -EBUSY;
  }
  printk(KERN_INFO "TestChar: Driver have been opened\n");

  number_of_opens++;

  return 0;   // Successfully opened opened
}

/** @brief The device release function that is called whenever the device is closed/released by
 *  the userspace program
 *  @param inode_ptr A pointer to an inode object (defined in linux/fs.h)
 *  @param file_ptr A pointer to a file object (defined in linux/fs.h)
 */
static int dev_release(struct inode *inode_ptr, struct file *file_ptr)
{
  return 0;   // Sucessfully released
}

/** @brief This function is called whenever device is being read from user space i.e. data is
 *  being sent from the device to the user. In this case is uses the copy_to_user() function to
 *  send the buffer string to the user and captures any errors.
 *  @param file_ptr A pointer to a file object (defined in linux/fs.h)
 *  @param user_buffer The pointer to the buffer to which this function writes the data
 *  @param buffer_size The length of the b
 *  @param offset_ptr The offset if required
 */
static ssize_t dev_read(struct file *file_ptr, char *user_buffer, size_t buffer_size, loff_t *offset_ptr)
{
  return 0;
}

/** @brief This function is called whenever the device is being written to from user space i.e.
 *  data is sent to the device from the user. The data is copied to the message[] array in this
 *  LKM using the copy_from_user() function along with the length of the string.
 *  @param file_ptr A pointer to a file object
 *  @param user_buffer The buffer to that contains the string to write to the device
 *  @param buffer_size The length of the array of data that is being passed in the const char buffer
 *  @param offset_ptr The offset if required
 */
static ssize_t dev_write(struct file *file_ptr, const char *user_buffer, size_t buffer_size, loff_t *offset_ptr)
{
  unsigned long bytes_not_copied;

  if(buffer_size <= 0)
  {
    return -1;
  }
  printk(KERN_INFO "TestChar: Received %lu characters from the user\n", data_size);

  bytes_not_copied = copy_from_user(message, user_buffer, buffer_size);
  if(bytes_not_copied > 0)
  {
    printk(KERN_INFO "TestChar: Error while writing\n");
    return -1;
  }
  size_of_message = strlen(message);

  return size_of_message;
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

/** @brief The LKM cleanup function
 *  Similar to the initialization function, it is static. The __exit macro notifies that if this
 *  code is used for a built-in driver (not a LKM) that this function is not required.
 */
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