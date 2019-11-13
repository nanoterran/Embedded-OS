#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/ioctl.h>
#include "commands.h"

#define BUFFER_LENGTH 256               ///< The buffer length (crude but fine)
static char receive[BUFFER_LENGTH];     ///< The receive buffer from the LKM

int main(void)
{
  int ret, file_descriptor;
  char stringToSend[BUFFER_LENGTH];

  unsigned int commands_lookup_table[] =
    {
      TESTCHAR_IOCRESET,
      TESTCHAR_ALLLOWER,
      TESTCHAR_ALLUPPER,
      TESTCHAR_ALLCAPS,
      TESTCHAR_NONE
    };

  printf("Starting device test code example...\n");

  // Open the device driver with read/write access
  file_descriptor = open("/dev/testchar", O_RDWR);
  if(file_descriptor < 0)
  {
    perror("Failed to open the device...");
    return errno;
  }

  char choice;

  printf("Send message to driver? [Y/n] ");
  scanf("%[^\n]%*c", &choice);

  while(choice == 'Y')
  {
    printf("Type in a short string to send to the kernel module:\n");

    // Read in a string (with spaces), changes delimeter to new line.
    scanf("%[^\n]%*c", stringToSend);

    printf("Writing message to the device [%s].\n\n", stringToSend);

    // Send the string to the Loadable Kernel Module
    ret = write(file_descriptor, stringToSend, strlen(stringToSend));
    if(ret < 0)
    {
      perror("Failed to write the message to the device.");
      return errno;
    }

    int command = 4;

    printf("1 - Set message to all lowercase\n");
    printf("2 - Set message to all uppercase\n");
    printf("3 - Set message to all caps\n");
    printf("4 - keep original message format\n");
    printf("Enter a ioctl command: ");
    scanf("%d", &command);
    fflush(stdin);

    ret = ioctl(file_descriptor, commands_lookup_table[command]);
    if(ret < 0)
    {
      perror("Failed to write the message to the device.");
      return errno;
    }

    printf("Press ENTER to read back from the device...\n");
    getchar();
    printf("Reading from the device...\n");
    
    ret = read(file_descriptor, receive, BUFFER_LENGTH);
    if(ret < 0)
    {
      perror("Failed to read the message from the device.");
      return errno;
    }
    printf("The received message is: [%s]\n\n", receive);

    printf("Send message to driver? [Y/n] ");
    scanf("%[^\n]%*c", &choice);
  }
  
  printf("End of the program\n");

  return 0;
}
