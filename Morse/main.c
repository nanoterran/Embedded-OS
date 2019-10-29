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

	mcode_configuration_t *configuration = malloc(sizeof(configuration));

	// Find the word on the argument options
	for(int i = 0; i < argc; i++)
	{
		if(!strcmp(argv[i], "-w"))
		{
			configuration->raw_word = argv[i + 1];
		}
	}

	// Gets the architecture of the system
	char architecture[10];
	FILE *command_fd = popen("uname -m", "r");
	if (command_fd == NULL) {
    printf("Failed to run command\n" );
    exit(1);
  }
  fgets(architecture, sizeof(architecture), command_fd);
	fclose(command_fd);

	configuration->architecture = architecture;

	if(!strcmp(configuration->architecture, "x86_64\n"))
	{
		configuration->file_descriptor = stdout;
	}
	else if(!strcmp(configuration->architecture, "armv7l\n"))
	{
		const char *LEDBrightness = "/sys/class/leds/beaglebone:green:usr3/brightness";
		configuration->file_descriptor = fopen(LEDBrightness, "r+");;
	}

	process_word(configuration);

	fclose(configuration->file_descriptor);
	free(configuration);

	return 0;
}
