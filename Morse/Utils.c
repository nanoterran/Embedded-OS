#include "Utils.h"

/*
 * Process a word depending on the system architecture. If running
 * on x86_64 it will print the Morse Code to the terminal, and
 * if running on the BeagleBone Black it will flash LED3
 */
void display_word_in_morse_code(mcode_configuration_t *configuration)
{
	FILE *file_descriptor = configuration->file_descriptor;
	char *word = configuration->raw_word;

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
				if(!strcmp(configuration->architecture, "x86_64\n"))
				{
					fprintf(file_descriptor, "Dot ");
				}
				else if(!strcmp(configuration->architecture, "armv7l\n"))
				{
					fprintf(file_descriptor, "1");
				}
				
				fflush(file_descriptor);
				usleep(DotTimeInMicroSec);
			}
			else if(mcode_letter[i] == '-')
			{
				if(!strcmp(configuration->architecture, "x86_64\n"))
				{
					fprintf(file_descriptor, "Dash ");
				}
				else if(!strcmp(configuration->architecture, "armv7l\n"))
				{
					fprintf(file_descriptor, "1");
				}

				fflush(file_descriptor);
				usleep(DashTimeInMicroSec);
			}

			if(!strcmp(configuration->architecture, "armv7l\n"))
			{
				fprintf(file_descriptor, "0");
			}
			
			fflush(file_descriptor);
			usleep(BetweenCharacterTimeInMicroSec);
		}

		if(!strcmp(configuration->architecture, "x86_64\n"))
		{
			fprintf(file_descriptor, "\n");
		}

		usleep(BetweenLetterTimeInMicroSec);
	}
}

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