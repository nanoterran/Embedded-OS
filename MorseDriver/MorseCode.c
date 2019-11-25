#include "MorseCode.h"

#define DEVICE_NAME "MorseCode"
#define CLASS_NAME  "Morse"

MODULE_AUTHOR("Javier Vega");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("A Morse Code Driver support to blink USR0 LED.");
MODULE_VERSION("1.0");


static int           dev_open(struct inode *, struct file *);
static int           dev_release(struct inode *, struct file *);
static ssize_t       dev_read(struct file *, char *, size_t, loff_t *);
static ssize_t       dev_write(struct file *, const char *, size_t, loff_t *);
static long          dev_ioctl(struct file *, unsigned int, unsigned long);

static void          display_morse_code_message(unsigned long value);
static void          display_morse_code_character(char character);
static inline char * ascii_to_morsecode(int asciicode);
static void          convert_message_to_morsecode(char *message, size_t size);
static void          turn_on_led(void);
static void          turn_off_led(void);
static void          set_display_time(uint32_t milli_seconds);
static void          set_timer_callback(void);
static void          set_timer_data(unsigned long data);
static uint8_t       done_displaying_message(void);
static void          set_device_state(uint8_t state);
static uint8_t       get_device_state(void);

static morse_character_data * get_character_data(char character);

static struct file_operations file_operations_t =
{
  .open           = dev_open,
  .read           = dev_read,
  .write          = dev_write,
  .release        = dev_release,
  .unlocked_ioctl = dev_ioctl
};

static                          DEFINE_MUTEX(morse_mutex);
static struct class            *morse_class = NULL;
static struct device           *morse_device;
static int                      major_number;
static struct timer_list        timer;
static struct morse_code_device morse;
static uint8_t                  number_of_opens = 0;

static const struct morse_character_data morse_character_table[] =
{
  { '.', DotTimeInMilliSec,                 turn_on_led },
  { '-', DashTimeInMilliSec,                turn_on_led },
  { ' ', IntraCharacterSpaceTimeInMilliSec, turn_off_led },
  { '#', InterCharacterSpaceTimeInMilliSec, turn_off_led },
  { '$', WordSpaceTimeInMilliSec,           turn_off_led }
};



/** @brief The LKM initialization function
 *  The static keyword restricts the visibility of the function to within
 *  this C file. The __init macro means that for a built-in driver (not a LKM)
 *  the function is only used at initialization time and that it can be 
 *  discarded and its memory freed up after that point.
 *  @return returns 0 if successful
 */
static int __init morse_init(void)
{
  printk(KERN_INFO "MorseCode: Initializing the MorseCode LKM\n");

  major_number = register_chrdev(0, DEVICE_NAME, &file_operations_t);
  if(major_number < 0)
  {
    printk(KERN_ALERT "MorseCode: failed to register a major number\n");

    return major_number;
  }
  printk(KERN_INFO "MorseCode: Registerd Correctrly %d\n", major_number);

  morse_class = class_create(THIS_MODULE, CLASS_NAME);
  if(IS_ERR(morse_class))
  {
    unregister_chrdev(major_number, DEVICE_NAME);

    printk(KERN_ALERT "MorseCode: Failed to register device class\n");

    return PTR_ERR(morse_class);
  }
  printk(KERN_INFO "MorseCode: device class registered correctly\n");

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

  mutex_init(&morse_mutex);

  // Way to initialize a timer for older kernel version
  init_timer(&timer);

  int i;
  for(i = 0; i < MAX_SIZE; i++)
  {
    morse.message[i] = 0;
  }

  morse.message_length = 0;
  morse.iterator = -1;
  set_device_state(STATE_IDLE);
 
  return 0;
}

/** @brief The device open function that is called each time the device is 
 *  opened. This will only increment the numberOpens counter in this case.
 *  @param inode_ptr A pointer to an inode object (defined in linux/fs.h)
 *  @param file_ptr A pointer to a file object (defined in linux/fs.h)
 */
static int dev_open(struct inode *inode_ptr, struct file *file_ptr)
{
  if(!mutex_trylock(&morse_mutex)){
    printk(KERN_ALERT "MorseCode: Device in use by another process\n");

    return -EBUSY;
  }
  printk(KERN_INFO "MorseCode: Driver have been opened\n");

  number_of_opens++;

  return 0;   // Successfully opened
}

/** @brief The device release function that is called whenever the device is 
 *  closed/released by the userspace program.
 *  @param inode_ptr A pointer to an inode object (defined in linux/fs.h)
 *  @param file_ptr A pointer to a file object (defined in linux/fs.h)
 */
static int dev_release(struct inode *inode_ptr, struct file *file_ptr)
{
  if(get_device_state() == STATE_DONE)
  {
    mutex_unlock(&morse_mutex);
  }
  return 0;
}

/** @brief This function is called whenever device is being read from
 *  user space i.e. data is being sent from the device to the user.
 *  In this case is uses the copy_to_user() function to send the buffer string
 *  to the user and captures any errors.
 *  @param file_ptr A pointer to a file object (defined in linux/fs.h)
 *  @param user_buffer The pointer to the buffer to write the data
 *  @param buffer_size The length of the b
 *  @param offset_ptr The offset if required
 */
static ssize_t dev_read(struct file *file_ptr, char *user_buffer, size_t buffer_size, loff_t *offset_ptr)
{
  return 0;
}

/** @brief This function is called whenever the device is being written
 *  to from user space i.e. data is sent to the device from the user.
 *  The data is copied to the message[] array in this LKM using the 
 *  copy_from_user() function along with the length of the string.
 *  @param file_ptr A pointer to a file object
 *  @param user_buffer The buffer with string from the user program
 *  @param buffer_size The length of the user program buffer
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
  printk(KERN_INFO "MorseCode: Received %lu characters from user\n", buffer_size);

  bytes_not_copied = copy_from_user(message, user_buffer, buffer_size);
  if(bytes_not_copied > 0)
  {
    printk(KERN_INFO "MorseCode: Error while writing\n");
    return -1;
  }
  size_of_message = strlen(message);

  convert_message_to_morsecode(message, size_of_message);

  set_display_time(SOONEST_POSSIBLE);
  set_timer_data(NULL);
  set_timer_callback();
  add_timer(&timer);

  set_device_state(STATE_BUSY);

  return size_of_message;
}

/**
 * Allows the user to set mode of the driver
 */
static long dev_ioctl(struct file *file_ptr, unsigned int command, unsigned long arg)
{
  return 0;
}

static void set_device_state(uint8_t state)
{
  morse.state = state;
}

static void turn_on_led(void)
{
  volatile void *gpio1_address;
  unsigned long *set_data;

  gpio1_address = ioremap(GPIO1_BASE_START_ADDRES, GPIO1_SIZE);
  set_data = (long)gpio1_address + GPIO1_DATAOUT_REGISTER_OFFSET;

  *set_data = *set_data | USR0_LED;
}

static void turn_off_led(void)
{
  volatile void *gpio1_address;
  unsigned long *clear_data;

  gpio1_address = ioremap(GPIO1_BASE_START_ADDRES, GPIO1_SIZE);
  clear_data = (long)gpio1_address + GPIO1_CLEAR_DATAOUT_REGISTER_OFFSET;

  *clear_data = *clear_data | USR0_LED;
}

static void set_timer_callback()
{
  timer.function = display_morse_code_message;
}

static void set_timmer_data(unsigned long data)
{
  timer.data = data;
}

static void set_display_time(uint32_t milli_seconds)
{
  uint64_t number_of_jiffies;

  if(milli_seconds > 0)
  {
    number_of_jiffies = milli_seconds * HZ;
    do_div(number_of_jiffies, 1000);
    timer.expires += number_of_jiffies;
  }
  else
  {
    timer.expires = jiffies;
  }
}

static uint8_t done_displaying_message()
{
  return morse.iterator >= morse.message_length;
}

static void convert_message_to_morsecode(char *message, size_t message_size)
{
  int i;
  int j;
  char *morse_code_char;

  morse.iterator = 0;

  for(i = 0; i < message_size; i++)
  {
    morse_code_char = (char *)ascii_to_morsecode((int)message[i]);

    printk(KERN_INFO "MorseCode: Char =  %s\n", morse_code_char);

    if(!strcmp(morse_code_char, ""))
    {
      morse.message[morse.iterator - 1] = '$';
    }
    else
    {
      for(j = 0; j < strlen(morse_code_char); j++)
      {
        morse.message[morse.iterator] = morse_code_char[j];
        morse.iterator++;
        morse.message[morse.iterator] = ' ';
        morse.iterator++;
      }

      morse.message[morse.iterator - 1] = '#';
    }
  }

  morse.message[morse.iterator - 1] = '\0';
  morse.message_length = strlen(morse.message);
  morse.iterator = 0;

  printk(KERN_INFO "MorseCode: Morse Message %s\n", morse.message);
  printk(KERN_INFO "MorseCode: Message Length %i\n", morse.message_length);
}

static morse_character_data * get_character_data(char character)
{
  int i;
  struct morse_character_data *current_character;

  current_character = morse_character_table;

  for(i = 0; i < CHARACTER_OPTIONS; i++)
  {
    if(current_character->character == character)
    {
      break;
    }
    current_character++;
  }

  return current_character;
}

static void display_morse_code_character(char character)
{
  struct morse_character_data *character_data;
  character_data = get_character_data(character);

  set_timer_callback();
  set_display_time(character_data->millisec_time);
  add_timer(&timer);
  character_data->display();
}

static void display_morse_code_message(unsigned long value)
{
  char current_morse_character;

  if(done_displaying_message())
  {
    turn_off_led();
    set_device_state(STATE_DONE);
    mutex_unlock(&morse_mutex);
    return;
  }

  current_morse_character = morse.message[morse.iterator++];
  display_morse_code_character(current_morse_character);
}

static inline char *ascii_to_morsecode(int asciicode)
{
   char *mc;

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
      mc = morse_code_lookup_table[37];  // 36 + 1
   else
      mc = morse_code_lookup_table[CQ_DEFAULT];

   return mc;
}

/** @brief The LKM cleanup function
 *  Similar to the initialization function, it is static. The __exit macro 
 *  notifies that if this code is used for a built-in driver (not a LKM) that 
 *  this function is not required.
 */
static void __exit morse_exit(void)
{
  device_destroy(morse_class, MKDEV(major_number, 0));
  class_unregister(morse_class);
  class_destroy(morse_class);
  unregister_chrdev(major_number, DEVICE_NAME);
  mutex_destroy(&morse_mutex);
  del_timer(&timer);

  printk(KERN_INFO "MorseCode: Goodbye from the Morse Code Device Driver!\n");
}

module_init(morse_init);
module_exit(morse_exit);