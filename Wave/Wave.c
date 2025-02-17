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

/**
 * Sets the terminal back into canonical mode.
 */ 
void disable_raw_mode()
{
  tcsetattr(STDIN_FILENO, TCSANOW, &oldTerminalInterface);
}

/**
 * Enables raw mode on the terminal so that input is available
 * character by character. Disables echoing and all processing
 * of terminal input characters.
 */
void enable_raw_mode()
{
  tcgetattr(STDIN_FILENO, &oldTerminalInterface);
  newTerminalInterface = oldTerminalInterface;
  cfmakeraw(&newTerminalInterface);
  tcsetattr(STDIN_FILENO, TCSANOW, &newTerminalInterface);
}

/**
 * Checks if a key has been pressed by watching stdin to see
 * when input becomes available.
 */
int key_pressed()
{
  struct timeval timeout;
  timeout.tv_usec = timeout.tv_sec = 0;

  FD_ZERO(&fileDescriptorSet);
  FD_SET(STDIN_FILENO, &fileDescriptorSet);
  select(STDIN_FILENO + 1, &fileDescriptorSet, 0, 0, &timeout);

  return FD_ISSET(STDIN_FILENO, &fileDescriptorSet);
}

/**
 * Discards any data by clearing the stdin buffer.
 */
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
    if(key_pressed())
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