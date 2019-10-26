#ifndef UTILS_H
#define UTILS_H

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "McodeMod.c"

extern char * mcodestring(int asciicode);

void process_mcode_letter(char *mcode_letter);
void process_word(char *word);

#endif