#ifndef DATA_STRUCTURES_H
#define DATA_STRUCTURES_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/*** text & line ***/

#define TAB_SIZE 8 // spaces for identation

typedef struct line{
	char *raw;
	int raw_len;
	char *rendered;
	int rendered_len;
} line;

typedef struct text{
	line *lines;
	int lines_num;
} text;

/*
 * Appends new raw_line and render_line to text.
 */
int append_line(text *txt, char *str, int len);

/*
*
*/
int insert_char(line* ln, int i, char ch);

/*
 *
 */
int update_rendered(line *ln);

/*
 *
 */
void free_text(text *txt);


/*** dynamic buffer ***/
#define INIT_DYNAMIC_BUFFER {NULL, 0}

struct dynamic_buffer{
	char *bytes;
	int len;
};

/*
 *
 */
int append_buffer(struct dynamic_buffer *arr, const char *str, int len);

/*
 *
 */
void free_buffer(struct dynamic_buffer *arr);


/*** auxiliar ***/

/*
*
*/
char* strdup(const char* str);

#endif /* DATA_STRUCTURES_H */