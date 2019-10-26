#include "Utils.h"

void process_word(char *word)
{
	for(int i = 0; i < strlen(word); i++)
	{
		char *mcode_letter = mcodestring(5);
		process_mcode_letter(mcode_letter);
		usleep(100000);
	}
}

void process_mcode_letter(char *mcode_letter)
{
	for(int i = 0; i < strlen(mcode_letter); i++)
	{
		printf("%c ", mcode_letter[i]);
		usleep(50000);
	}
	printf("\t");
}
