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
#include <asm/div64.h>
#include <linux/jiffies.h>
#include <linux/types.h>

#define DEVICE_NAME "MorseCode"
#define CLASS_NAME  "Morse"
#define MAX_SIZE 256
#define CQ_DEFAULT	0


// static long MULTIPLIER = do_div(var, 1000);

// #define DotTimeInMilliSec (5 * HZ) // = (500 * MULTIPLIER);
// #define DashTimeInMilliSec (1.5 * HZ) // = (1500 * MULTIPLIER);
// #define IntraCharacterSpaceTimeInMilliSec (0.2 * HZ) // = (200 * MULTIPLIER);  // space between dots and dashes
// #define InterCharacterSpaceTimeInMilliSec (0.6 * HZ) // = (600 * MULTIPLIER);  // space between characters of a word
// #define WordSpaceTimeInMilliSec (1.4 * HZ)// = (1400 * MULTIPLIER);  // space between words

MODULE_AUTHOR("Javier Vega");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("A character device driver for morse code.");
MODULE_VERSION("1.0");

/**
 * The empty string, follwed by 26 letter codes,
 * followed by the 10 numeral codes, followed by the comma,
 * period, and question mark.
 */
char *morse_code_lookup_table[40] =
  {
    "", ".-","-...","-.-.","-..",".","..-.","--.","....","..",
    ".---","-.-",".-..","--","-.","---",".--.","--.-",".-.",
    "...","-","..-","...-",".--","-..-","-.--","--..","-----",
    ".----","..---","...--","....-",".....","-....","--...",
    "---..","----.","--..--","-.-.-.","..--.."
  };

enum
{
  DotTimeInMilliSec                 = 500,
  DashTimeInMilliSec                = 1500,
  IntraCharacterSpaceTimeInMilliSec = 200,  // space between dots and dashes
  InterCharacterSpaceTimeInMilliSec = 600,  // space between characters of a word
  WordSpaceTimeInMilliSec           = 1400  // space between words
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
static void         process_morse_code(unsigned long value);
static void         intra_character_space(unsigned long value);
static inline char *ascii_to_morsecode(int asciicode);
static void         convert_message_to_morsecode(char *message, size_t size);

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
static char              morse_code[MAX_SIZE] = {0};
static short             morse_code_length = 0;
static short             morse_code_iterator = -1;
static int               number_of_opens = 0;
static int               space = 0;
static uint64_t          DotTimeInJiffies = DotTimeInMilliSec * HZ;
static uint64_t          DashTimeInJiffies = DashTimeInMilliSec * HZ;
static uint64_t          IntraCharacterSpaceTimeInJiffies = IntraCharacterSpaceTimeInMilliSec * HZ;
static uint64_t          InterCharacterSpaceTimeInJiffies = InterCharacterSpaceTimeInMilliSec * HZ;
static uint64_t          WordSpaceTimeInJiffies = WordSpaceTimeInMilliSec * HZ;

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

  // Way to initialize a timer for older kernel version
  init_timer(&timer);

  do_div(DotTimeInJiffies, 1000);
  do_div(DashTimeInJiffies, 1000);
  do_div(IntraCharacterSpaceTimeInJiffies, 1000);
  do_div(InterCharacterSpaceTimeInJiffies, 1000);
  do_div(WordSpaceTimeInJiffies, 1000);
  // DashTimeInMilliSec = (1500 * HZ);
  // IntraCharacterSpaceTimeInMilliSec = (200 * HZ);  // space between dots and dashes
  // InterCharacterSpaceTimeInMilliSec = (600 * HZ);  // space between characters of a word
  // WordSpaceTimeInMilliSec = (1400 * HZ);  // space between words

  // printk(KERN_INFO "Value = %llu\n", val);
  // printk(KERN_INFO "Rem = %d\n", rem);
  // printk(KERN_INFO "Dot = %l\n", DashTimeInMilliSec);
  // printk(KERN_INFO "Dot = %l\n", IntraCharacterSpaceTimeInMilliSec);
  // printk(KERN_INFO "Dot = %l\n", InterCharacterSpaceTimeInMilliSec);
  // printk(KERN_INFO "Dot = %l\n", WordSpaceTimeInMilliSec);

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
  if(!mutex_trylock(&morse_mutex)){
    printk(KERN_ALERT "MorseCode: Device in use by another process\n");

    return -EBUSY;
  }
  printk(KERN_INFO "MorseCode: Driver have been opened\n");

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
  // Releases the mutex (i.e., the lock goes up)
  mutex_unlock(&morse_mutex);

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
  char message[MAX_SIZE] = {0};
  int size_of_message = 0;

  if(buffer_size <= 0)
  {
    return -1;
  }
  printk(KERN_INFO "MorseCode: Received %lu characters from the user\n", buffer_size);

  bytes_not_copied = copy_from_user(message, user_buffer, buffer_size);
  if(bytes_not_copied > 0)
  {
    printk(KERN_INFO "MorseCode: Error while writing\n");
    return -1;
  }
  size_of_message = strlen(message);

  convert_message_to_morsecode(message, size_of_message);

  // Add the callback function to timer to start displaying morse code
  timer.expires = jiffies;
  timer.data = (unsigned long)NULL;
  timer.function = process_morse_code;
  add_timer(&timer);

  return size_of_message;
}

/** @brief Converts the message writen by the user to morse code string.
 *  @param message The message writen by the user
 *  @param message_size The size of the message
 */
static void convert_message_to_morsecode(char *message, size_t message_size)
{
  int i;
  int j;
  morse_code_iterator = 0;

  for(i = 0; i < message_size; i++)
  {
    char *morse_code_char = (char *)ascii_to_morsecode((int)message[i]);
    printk(KERN_INFO "MorseCode: Char =  %s\n", morse_code_char);

    if(!strcmp(morse_code_char, ""))
    {
      morse_code[morse_code_iterator - 1] = '$';
    }
    else
    {
      for(j = 0; j < strlen(morse_code_char); j++)
      {
        morse_code[morse_code_iterator] = morse_code_char[j];
        morse_code_iterator++;
      }

      morse_code[morse_code_iterator] = '#';
      morse_code_iterator++;
    }
  }

  morse_code[morse_code_iterator - 1] = '\0';
  morse_code_length = strlen(morse_code);
  morse_code_iterator = 0;

  printk(KERN_INFO "MorseCode: Morse Message %s\n", morse_code);
  printk(KERN_INFO "MorseCode: Morse Message Length %i\n", morse_code_length);
}

/**
 * Allows the user to set mode of the driver
 */
static long dev_ioctl(struct file *file_ptr, unsigned int command, unsigned long arg)
{
  return 0;
}

static void intra_character_space(unsigned long value)
{
  printk(KERN_INFO "MorseCode: Space Between Dots and Dashes\n");

  timer.expires += IntraCharacterSpaceTimeInJiffies;
  timer.function = process_morse_code;
  add_timer(&timer);
}

static void process_morse_code(unsigned long value)
{
  // unsigned long gpio1_base_address = 0x4804C000;
  // unsigned long gpio1_base_end_address = 0x4804E000;
  // unsigned long register_offset = 0x13C;
  // unsigned long clear_register_offset = 0x190;

  // char *base_address_ptr = ioremap(gpio1_base_address, gpio1_base_end_address - gpio1_base_address);
  // unsigned long *reg = (long)base_address_ptr + register_offset;
  // *reg = *reg | (1<<21);

  if(morse_code_iterator >= morse_code_length)
  {
    return;
  }

  char current_character = morse_code[morse_code_iterator];

  if(current_character == '.')
  {
    printk(KERN_INFO "MorseCode: Dot\n");

    timer.expires += DotTimeInJiffies;
    timer.function = intra_character_space;
    add_timer(&timer);
  }
  else if(current_character == '-')
  {
    printk(KERN_INFO "MorseCode: Dash\n");

    timer.expires += DashTimeInJiffies;
    timer.function = intra_character_space;
    add_timer(&timer);
  }
  else if(current_character == '#')
  {
    printk(KERN_INFO "MorseCode: Between Letters\n");

    timer.expires += InterCharacterSpaceTimeInJiffies;
    timer.function = process_morse_code;
    add_timer(&timer);
  }
  // else if(current_character == '$')
  // {
  //   printk(KERN_INFO "MorseCode: Between Word\n");

  //   timer.expires += HZ * (WordSpaceTimeInMilliSec / 1000.0);
  //   add_timer(&timer);
  // }
  morse_code_iterator++;

  // if(led_state == 0)
  // {
  //   unsigned long *reg = (long)base_address_ptr + register_offset;
  //   *reg = *reg | (1<<21);
  //   printk(KERN_INFO "MorseCode: SetData GPIO1 Register = %d\n", *reg);
  //   led_state = 1;
  // }
  // else
  // {
  //   unsigned long *reg = (long)base_address_ptr + clear_register_offset;
  //   *reg = *reg | (1<<21);
  //   printk(KERN_INFO "MorseCode: Clear GPIO1 Register = %d\n", *reg);
  //   led_state = 0;
  // }
  
  // timer.expires += HZ; // add another delay period
  // add_timer(&timer);
}

/**
 * Maps the ascii value to its morse code string.
 */
static inline char *ascii_to_morsecode(int asciicode)
{
   char *mc;   // this is the mapping from the ASCII code into the mcodearray of strings.

   if (asciicode > 122)  // Past 'z'
      mc = morse_code_lookup_table[CQ_DEFAULT];
   else if (asciicode > 96)  // Upper Case
      mc = morse_code_lookup_table[asciicode - 96];
   else if (asciicode > 90)  // uncoded punctuation
      mc = morse_code_lookup_table[CQ_DEFAULT];
   else if (asciicode > 64)  // Lower Case 
      mc = morse_code_lookup_table[asciicode - 64];
   else if (asciicode == 63)  // Question Mark
      mc = morse_code_lookup_table[39];    // 36 + 3 
   else if (asciicode > 57)  // uncoded punctuation
      mc = morse_code_lookup_table[CQ_DEFAULT];
   else if (asciicode > 47)  // Numeral
      mc = morse_code_lookup_table[asciicode - 21];  // 27 + (asciicode - 48) 
   else if (asciicode == 46)  // Period
      mc = morse_code_lookup_table[38];  // 36 + 2 
   else if (asciicode == 44)  // Comma
      mc = morse_code_lookup_table[37];   // 36 + 1
   else
      mc = morse_code_lookup_table[CQ_DEFAULT];
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