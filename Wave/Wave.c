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

void set_mode(int want_key)
{
  static struct termios oldTerminalInterface, newTerminalInterface;

  if (!want_key) {
    tcsetattr(STDIN_FILENO, TCSANOW, &oldTerminalInterface);
    return;
  }

  tcgetattr(STDIN_FILENO, &oldTerminalInterface);
  newTerminalInterface = oldTerminalInterface;
  newTerminalInterface.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &newTerminalInterface);
}

int get_key()
{
  int keyValue = 0;
  struct timeval timeout;
  fd_set fileDescriptorSet;
  timeout.tv_usec = timeout.tv_sec = 0;

  FD_ZERO(&fileDescriptorSet);
  FD_SET(STDIN_FILENO, &fileDescriptorSet);
  select(STDIN_FILENO + 1, &fileDescriptorSet, 0, 0, &timeout);

  if (FD_ISSET(STDIN_FILENO, &fileDescriptorSet)) {
    keyValue = getchar();
    set_mode(0);
  }
  return keyValue;
}

int main(int argc, char *argv[])
{
  if(argc < RequiredNumberOfArguments)
  {
    printf("[-] ERROR: Insufficient number of arguments.");
    printf("[!] Usage: %s <output file> <number of samples>\n", argv[0]);
    return 1;
  }

  int keyValue;
  char *outputFilename = argv[1];
  int numberOfSamples = atoi(argv[2]);
  FILE *outputFile = fopen(outputFilename, "w");

  if(outputFilename == NULL)
  {
    printf("[-] ERROR: Could not open %s", outputFilename);
    return 1;
  }
  printf("Processing...\n");

  while(numberOfSamples > 0)
  {
    set_mode(1);

    if(!(keyValue = get_key()))
    {
      fprintf(outputFile, "0\n");
    }
    else
    {
      fprintf(outputFile, "1\n");
    }

    numberOfSamples--;
    sleep(DelayTimeInSeconds);
  }

  fclose(outputFile);
  printf("Done Processing %s\n", outputFilename);

  return 0;
}