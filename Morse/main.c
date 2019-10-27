#include <stdio.h>
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
	process_word(raw_word);
	printf("\n");

	return 0;
}
