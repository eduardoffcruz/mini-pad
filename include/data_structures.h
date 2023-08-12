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
	unsigned long raw_len;
	
	char *rendered;
	unsigned long rendered_len;
} line;

typedef struct text{
	// for modification verification
	char modified;
	
	line *lines;
	unsigned long lines_num;
} text;

/*
*
*/
text* new_text(void);

/*
 * Appends new raw_line and render_line to text.
 */
int append_line(text *txt, unsigned long line_i, char *str, unsigned long len);

/*
*
*/
int insert_char(line* ln, unsigned long j, char ch);

/*
*
*/
void delete_char(line* ln, unsigned long j);

/*
*
*/
int merge_lines(text *txt, unsigned long at, unsigned long to);

/*
 *
 */
int update_rendered(line *ln);

/*
*
*/
char* text_to_data(text* txt, size_t text_size);

/*
*
*/
size_t compute_text_size(text* txt);

/*
*
*/
unsigned long compute_text_crc32(text *txt, size_t size, int* err);

/*
 *
 */
void free_text(text *txt);

/*
 *
 */
void free_line(line *ln);


/*** dynamic buffer ***/

struct dynamic_buffer{
	char *bytes;
	int len;
};

/*
 *
 */
int append_buffer(struct dynamic_buffer *arr, const char *str, unsigned int len);

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