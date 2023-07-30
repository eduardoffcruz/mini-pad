#ifndef DATA_STRUCTURES_H
#define DATA_STRUCTURES_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <zlib.h>
#include <unistd.h>

#define TAB_SIZE 8 // spaces for identation

#define INIT_DYNAMIC_BUFFER {NULL, 0}

/*** text & line ***/

typedef struct line{
	char *raw;
	int raw_len;
	
	char *rendered;
	int rendered_len;
} line;

typedef struct text{
	// for modification verification
	size_t saved_size;
	unsigned long saved_crc;
	
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
char* text_to_data(text* txt, int text_size);

/*
*
*/
int compute_text_size(text* txt);

/*
*
*/
unsigned long compute_text_crc32(text *txt, size_t size, int* err);

/*
 *
 */
void free_text(text *txt);


/*** dynamic buffer ***/

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
unsigned long compute_crc32(const char* data, size_t size);

/*
*
*/
unsigned long compute_text_crc32(text *txt, size_t size, int* err);

/*
*
*/
char* strdup(const char* str);

#endif /* DATA_STRUCTURES_H */