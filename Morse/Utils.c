#include "Utils.h"

void process_word(char *word)
{
	int i;
	for(i = 0; i < strlen(word); i++)
	{
		int ascii = (int)word[i];
		char *mcode_letter = mcodestring(ascii);
		printf("%s\n", mcode_letter);
		// process_mcode_letter(mcode_letter);
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
