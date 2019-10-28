#include "Utils.h"

int main(int argc, char *argv[])
{
	int MaxNumberOfArguments = 3;

	if(argc < MaxNumberOfArguments)
	{
		fprintf(stderr, "[-] ERROR: Insufficient number of arguments.");
		fprintf(stderr, "[!] Usage: %s -w <Word>\n", argv[0]);
		return 1;
	}
	char *raw_word = argv[2];

	FILE *command_fd = popen("uname -m", "r");
	if (command_fd == NULL) {
    printf("Failed to run command\n" );
    exit(1);
  }

	char architecture[10];

	/* Read the output a line at a time - output it. */
  fgets(architecture, sizeof(architecture), command_fd);
	fclose(command_fd);
	process_word(raw_word, architecture);

	return 0;
}
