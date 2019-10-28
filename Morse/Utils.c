#include "Utils.h"

void process_word(char *word, char *architecture)
{
	if(!strcmp(architecture, "x86_64\n"))
	{
		x86_64_process_mcode(word);
	}
	else if(!strcmp(architecture, "armv7l\n"))
	{
		arm_process_mcode(word);
	}
}

void arm_process_mcode(char *word)
{
	const char *LEDBrightness = "/sys/class/leds/beaglebone:green:usr3/brightness";
	FILE *file_descriptor = fopen(LEDBrightness, "r+");

	for(int i = 0; i < strlen(word); i++)
	{
		int ascii_code = (int)word[i];
		char *mcode_letter = (char *)mcodestring(ascii_code);
		
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

	fclose(file_descriptor);
}

void x86_64_process_mcode(char *word)
{
	for(int i = 0; i < strlen(word); i++)
	{
		int ascii_code = (int)word[i];
		char *mcode_letter = (char *)mcodestring(ascii_code);
		
		for(int i = 0; i < strlen(mcode_letter); i++)
		{
			if(mcode_letter[i] == '.')
			{
				fprintf(stdout, "Dot ");
				fflush(stdout);
				usleep(DotTimeInMicroSec);
			}
			else if(mcode_letter[i] == '-')
			{
				fprintf(stdout, "Dash ");
				fflush(stdout);
				usleep(DashTimeInMicroSec);
			}

			usleep(BetweenCharacterTimeInMicroSec);
		}

		fprintf(stdout, "\t");
		usleep(BetweenLetterTimeInMicroSec);
	}
}
