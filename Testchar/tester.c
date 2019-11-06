#include<stdio.h>
#include<stdlib.h>
#include<errno.h>
#include<fcntl.h>
#include<string.h>

#define BUFFER_LENGTH 256               ///< The buffer length (crude but fine)
static char receive[BUFFER_LENGTH];     ///< The receive buffer from the LKM

int main(void)
{
  int ret, file_descriptor;
  char stringToSend[BUFFER_LENGTH];

  printf("Starting device test code example...\n");

  // Open the device driver with read/write access
  file_descriptor = open("/dev/testchar", O_RDWR);
  if(file_descriptor < 0)
  {
    perror("Failed to open the device...");
    return errno;
  }

  printf("Type in a short string to send to the kernel module:\n");

  // Read in a string (with spaces), changes delimeter to new line.
  scanf("%[^\n]%*c", stringToSend);

  printf("Writing message to the device [%s].\n", stringToSend);

  // Send the string to the Loadable Kernel Module
  ret = write(file_descriptor, stringToSend, strlen(stringToSend));
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
  printf("The received message is: [%s]\n", receive);
  printf("End of the program\n");

  return 0;
}
