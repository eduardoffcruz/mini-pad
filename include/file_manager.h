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

int read_file_text(const char *filename, struct text *txt);

#endif /* FILE_MANAGER_H */