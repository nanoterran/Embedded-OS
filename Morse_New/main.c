#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "McodeMod.h"

enum
{
  DotTimeInMicroSec = 500000,
  DashTimeInMicroSec = 1500000,
  BetweenCharacterTimeInMicroSec = 250000,
  BetweenLetterTimeInMicroSec = 2000000
};

char * extract_word_from_arguments(int arg_count, char *arg_values[])
{
	// Find the word on the argument options
	for(int i = 0; i < arg_count; i++)
	{
		if(!strcmp(arg_values[i], "-w"))
		{
			return arg_values[i + 1];
		}
	}
}

/*
 * Process a word depending on the system architecture. If running
 * on x86_64 it will print the Morse Code to the terminal, and
 * if running on the BeagleBone Black it will flash LED3
 */
void display_word_in_morse_code(FILE *file_descriptor, char *word)
{
	// Process the entire word character by character
	for(int i = 0; i < strlen(word); i++)
	{
		int ascii_code = (int)word[i];
		char *mcode_letter = (char *)ascii_to_morse_code(ascii_code);

		// Represents each character in Morse Code
		for(int i = 0; i < strlen(mcode_letter); i++)
		{
			if(mcode_letter[i] == '.')
			{
				fprintf(file_descriptor, "1");
				fflush(file_descriptor);
				usleep(DotTimeInMicroSec);
			}
			else if(mcode_letter[i] == '-')
			{
				fprintf(file_descriptor, "1");
				fflush(file_descriptor);
				usleep(DashTimeInMicroSec);
			}

			fprintf(file_descriptor, "0");
			fflush(file_descriptor);
			usleep(BetweenCharacterTimeInMicroSec);
		}

		usleep(BetweenLetterTimeInMicroSec);
	}
}

int main(int argc, char *argv[])
{
	int MaxNumberOfArguments = 3;

	if(argc < MaxNumberOfArguments)
	{
		fprintf(stderr, "[-] ERROR: Insufficient number of arguments.");
		fprintf(stderr, "[!] Usage: %s -w <Word>\n", argv[0]);
		return 1;
	}

	char *raw_word = extract_word_from_arguments(argc, argv);

	const char *LEDBrightness = "/sys/class/leds/beaglebone:green:usr3/brightness";
	FILE *file_descriptor = fopen(LEDBrightness, "r+");;

	display_word_in_morse_code(file_descriptor, raw_word);
	fclose(file_descriptor);

	return 0;
}
