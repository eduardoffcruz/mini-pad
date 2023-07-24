#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ABUF_INIT {NULL, 0}

struct abuf {
  char *b;
  int len;
};

int buffer_append(struct abuf *buffer, const char *str, int len);

void free_buffer(struct abuf *buffer);