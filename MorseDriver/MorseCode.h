#ifndef MORSECODE_H
#define MORSECODE_H

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

#define GPIO1_BASE_START_ADDRES             0x4804C000
#define GPIO1_BASE_END_ADDRESS              0x4804E000
#define GPIO1_DATAOUT_REGISTER_OFFSET       0x13C
#define GPIO1_CLEAR_DATAOUT_REGISTER_OFFSET 0x190

#define USR0_LED   (1<<21)
#define GPIO1_SIZE (GPIO1_BASE_END_ADDRESS - GPIO1_BASE_START_ADDRES)

#define MAX_SIZE          256
#define CQ_DEFAULT        0
#define CHARACTER_OPTIONS 5
#define SOONEST_POSSIBLE  0
#define NO_DATA           0

#define STATE_DONE 0
#define STATE_BUSY 1
#define STATE_IDLE 2

typedef struct morse_character_data
{
  char     character;
  uint32_t millisec_time;
  void     (*display)(void);
} morse_character_data;

typedef struct morse_code_device
{
  char    message[MAX_SIZE];
  uint8_t message_length;
  uint8_t iterator;
  uint8_t state;
} morse_code_device;

enum
{
  DotTimeInMilliSec                 = 500,
  DashTimeInMilliSec                = 1500,
  IntraCharacterSpaceTimeInMilliSec = 200,  // space between dots and dashes
  InterCharacterSpaceTimeInMilliSec = 600,  // space between characters of a word
  WordSpaceTimeInMilliSec           = 1400  // space between words
};

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

#endif
