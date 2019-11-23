#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#define BUFFER_LENGTH 256
const char driver_path = "/dev/MorseCode";

int main(void)
{
  int file_descriptor;
  char choice;

  // Open the device driver with read/write access
  file_descriptor = open(driver_path, O_RDWR);
  if(file_descriptor < 0)
  {
    perror("Failed to open the device...");
    return errno;
  }

  printf("Send morse code message? [Y/n] ");
  scanf("%[^\n]%*c", &choice);

  while(choice == 'Y')
  {
    char stringToSend[BUFFER_LENGTH] = {0};

    printf("Type a message to translate to morse code:\n");
    scanf("%[^\n]%*c", stringToSend);

    printf("Writing message to the device [%s].\n\n", stringToSend);

    // Send the string to the Loadable Kernel Module
    int ret = write(file_descriptor, stringToSend, strlen(stringToSend));
    if(ret < 0)
    {
      perror("Failed to write the message to the device.");
      return errno;
    }

    printf("\n");
    printf("Press ENTER to read back from the device...\n");
    getchar();
    printf("Reading from the device...\n");

    printf("Send message to driver? [Y/n] ");
    scanf("%[^\n]%*c", &choice);
  }

  close(file_descriptor);
  printf("End of the program\n");

  return 0;
}