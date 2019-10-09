#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>

enum
{
  RequiredNumberOfArguments = 3,
  DelayTimeInSeconds = 1
};

static struct termios newTerminalInterface, oldTerminalInterface;
static fd_set fileDescriptorSet;

void disable_raw_mode()
{
  tcsetattr(STDIN_FILENO, TCSANOW, &oldTerminalInterface);
}

void enable_raw_mode()
{
  tcgetattr(STDIN_FILENO, &oldTerminalInterface);
  newTerminalInterface = oldTerminalInterface;
  cfmakeraw(&newTerminalInterface);
  tcsetattr(STDIN_FILENO, TCSANOW, &newTerminalInterface);
}

int input_available()
{
  struct timeval timeout;
  timeout.tv_usec = timeout.tv_sec = 0;

  FD_ZERO(&fileDescriptorSet);
  FD_SET(STDIN_FILENO, &fileDescriptorSet);
  select(STDIN_FILENO + 1, &fileDescriptorSet, 0, 0, &timeout);

  return FD_ISSET(STDIN_FILENO, &fileDescriptorSet);
}

void discard_data()
{
  tcflush(STDIN_FILENO, TCIFLUSH);
}

int main(int argc, char *argv[])
{
  if(argc < RequiredNumberOfArguments)
  {
    printf("[-] ERROR: Insufficient number of arguments.");
    printf("[!] Usage: %s <output file> <number of samples>\n", argv[0]);
    return 1;
  }

  char *outputFilename = argv[1];
  int numberOfSamples = atoi(argv[2]);
  FILE *outputFile = fopen(outputFilename, "w");

  if(outputFilename == NULL)
  {
    printf("[-] ERROR: Could not open %s", outputFilename);
    return 1;
  }

  enable_raw_mode();

  while(numberOfSamples > 0)
  {
    if(input_available())
    {
      discard_data();
      fprintf(outputFile, "1\n");
    }
    else
    {
      fprintf(outputFile, "0\n");
    }

    numberOfSamples--;
    sleep(DelayTimeInSeconds);
  }

  disable_raw_mode();

  fclose(outputFile);
  printf("Done Processing %s\n", outputFilename);

  return 0;
}