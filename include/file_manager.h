#ifndef FILE_MANAGER_H
#define FILE_MANAGER_H

#define _DEFAULT_SOURCE
#define _BSD_SOURCE
#define _GNU_SOURCE

#include <sys/types.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "data_structures.h"


/*** file operations ***/

int read_file_to_text(const char *filename, struct text *txt);

int save_text_to_file(text* txt, size_t size, const char *filename);

#endif /* FILE_MANAGER_H */