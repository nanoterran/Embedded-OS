#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#define COMMAND_ROTATE 0
#define COMMAND_STOP 1
#define ROTATE_LEFT 100
#define ROTATE_RIGHT 101

int main(int argc, char *argv[])
{
  int ret, file_descriptor;
  char choice;

  printf("Starting motor test code example...\n");

  // Open the device driver with read/write access
  file_descriptor = open("/dev/motor1", O_RDWR);
  if(file_descriptor < 0)
  {
    perror("Failed to open the device...");
    return errno;
  }

  // Issues a command to rotate the motor to the left
  ret = ioctl(file_descriptor, COMMAND_ROTATE, ROTATE_LEFT);
  if(ret < 0)
  {
    perror("Failed to start rotating the motor to the left.");
    return errno;
  }
  printf("Motor started to rotate to the left\n");

  // Issues a command to stop the motor
  ret = ioctl(file_descriptor, COMMAND_STOP, NULL);
  if(ret < 0)
  {
    perror("Failed to stop the motor.");
    return errno;
  }
  printf("Motor has been stoped\n");

  printf("Done\n");
  close(file_descriptor);

  return 0;
}