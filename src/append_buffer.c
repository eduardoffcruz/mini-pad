
#include "append_buffer.h"

int buffer_append(struct abuf *buffer, const char *str, int len) {
  char *new;
  if ((new = realloc(buffer->b, buffer->len + len)) == NULL) 
    return -1;

  memcpy(&new[buffer->len], str, len);
  buffer->b = new;
  buffer->len += len;

  return 0;
}

void free_buffer(struct abuf *buffer) {
  free(buffer->b);
}