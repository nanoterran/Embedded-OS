#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>

enum
{
  RequiredNumberOfArguments = 3,
  DelayTimeInMicroSec = 1000000
};

void set_mode(int want_key)
{
	static struct termios old, new;
	if (!want_key) {
		tcsetattr(STDIN_FILENO, TCSANOW, &old);
		return;
	}
 
	tcgetattr(STDIN_FILENO, &old);
	new = old;
	new.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(STDIN_FILENO, TCSANOW, &new);
}

int get_key()
{
	int c = 0;
	struct timeval tv;
	fd_set fs;
	tv.tv_usec = tv.tv_sec = 0;
 
	FD_ZERO(&fs);
	FD_SET(STDIN_FILENO, &fs);
	select(STDIN_FILENO + 1, &fs, 0, 0, &tv);
 
	if (FD_ISSET(STDIN_FILENO, &fs)) {
		c = getchar();
		set_mode(0);
	}
	return c;
}

int main(int argc, char *argv[])
{

	// while(1) {
	// 	set_mode(1);
	// 	while (!(c = get_key()))
  //   {
  //     printf("0\n");
  //     usleep(10000);
  //   }
	// 	printf("key %d\n", c);
	// }

  if(argc < RequiredNumberOfArguments)
  {
    printf("[-] ERROR: Insufficient number of arguments.");
    printf("[!] Usage: %s <output file> <number of samples>\n", argv[0]);
    return 1;
  }
  int c;
  char *outputFilename = argv[1];
  int numberOfSamples = atoi(argv[2]);

  FILE *outputFile = fopen(outputFilename, "a");

  printf("Writing to file...\n");
  while(numberOfSamples > 0)
  {
    set_mode(1);

    if(!(c = get_key()))
    {
      fprintf(outputFile, "0\n");
      // numberOfSamples--;
      // usleep(1000);
    }
    else
    {
      fprintf(outputFile, "1\n");
    }
    numberOfSamples--;
    usleep(DelayTimeInMicroSec);
  }

  fclose(outputFile);

  return 0;
}