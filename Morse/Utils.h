#ifndef UTILS_H
#define UTILS_H

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "McodeMod.h"

enum
{
  DotTimeInMicroSec               = 500000,
  DashTimeInMicroSec              = 1500000,
  BetweenCharacterTimeInMicroSec  = 250000,
  BetweenLetterTimeInMicroSec     = 2000000
};

typedef struct mcode_configuration_t
{
  char *architecture;
  FILE *file_descriptor;
  char *raw_word;
} mcode_configuration_t;

void process_word(mcode_configuration_t *configuration);

#endif