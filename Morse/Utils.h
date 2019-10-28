#ifndef UTILS_H
#define UTILS_H

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "McodeMod.h"

enum
{
  DotTimeInMicroSec =              500000,
  DashTimeInMicroSec =             1500000,
  BetweenCharacterTimeInMicroSec = 250000,
  BetweenLetterTimeInMicroSec =    2000000
};

void x86_64_process_mcode(char *word);
void arm_process_mcode(char *word);
void process_word(char *word, char *architecture);

#endif