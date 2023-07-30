
#include "data_structures.h"


/*** text & line structures ***/

int append_line(text *txt, char *str, int len){
    // Re-allocate space for new line
    if((txt->lines = (line *)realloc(txt->lines, sizeof(line) * (txt->lines_num + 1))) == NULL){
        return -1;
    }
    int line_i = txt->lines_num;

    // RAW line
    // allocate and store str content in .raw
    if ((txt->lines[line_i].raw = (char *)malloc(sizeof(char) * (len + 1))) == NULL){
        return -1;
    }
    txt->lines[line_i].raw_len = len;
    memcpy(txt->lines[line_i].raw, str, len);
    txt->lines[line_i].raw[len] = 0; // end-of-line

    // RENDERED line from RAW
    int err;
    if ((err = update_rendered(&(txt->lines[line_i]))) != 0){
        return err;
    }

    //
    txt->lines_num++;
    return 0;
}

int insert_char(line* ln, int i, char ch){
    if (i < 0 || i > ln->raw_len){
        i = ln->raw_len;
    }
    if ((ln->raw = (char*)realloc(ln->raw, sizeof(char)*(ln->raw_len+2))) == NULL){
        return -1;
    }
    memmove(&(ln->raw[i+1]), &(ln->raw[i]), ln->raw_len-i+1); // handles overlapping memory regions
    ln->raw_len++;
    ln->raw[i] = ch;

    update_rendered(ln);
    return 0;
}

int update_rendered(line* ln){
    int tabs = 0;
    int j, raw_len = ln->raw_len;
    // count tabs
    for (j = 0; j < raw_len; j++){
        if (ln->raw[j] == '\t'){
            tabs++;
        }
    }

    // re-allocate rendered
    free(ln->rendered);
    if ((ln->rendered = (char*)malloc(sizeof(char) * (raw_len + tabs * (TAB_SIZE - 1) + 1))) == NULL){
        return -1;
    }
    // render tabs according to TAB_SIZE
    int ind = 0;
    for (j = 0; j < raw_len; j++)
    {
        char raw_byte;
        if ((raw_byte = ln->raw[j]) == '\t'){
            ln->rendered[ind++] = ' '; // <space>
            while (ind % TAB_SIZE){
                ln->rendered[ind++] = ' '; // <space>
            }
        } else{
            ln->rendered[ind++] = raw_byte;
        }
    }

    ln->rendered[ind] = 0; // end-of-line
    ln->rendered_len = ind;
    return 0;
}

char* text_to_data(text* txt, int text_size){
	char *data;

	if ((data = (char *)malloc(text_size)) == NULL){
		return NULL;
	}

	char *p = data;
	for (int line_i = 0; line_i < txt->lines_num; line_i++){
		int curr_line_len = txt->lines[line_i].raw_len;
		memcpy(p, txt->lines[line_i].raw, sizeof(char)*curr_line_len);
		p += curr_line_len;
		*(p++) = '\n';
	}

	return data;
}

int compute_text_size(text* txt){
	int text_size = 0;
	for (int i = 0; i < txt->lines_num; i++){
		text_size += txt->lines[i].raw_len + 1; // including newline
	}
	return text_size*sizeof(char);
}

unsigned long compute_text_crc32(text *txt, size_t size, int* err){
    char* data = text_to_data(txt, size);
    if (data == NULL){
        *err = -1; // error converting text to char*
        return 0L;
    }
    
    unsigned long crc = compute_crc32(data, size);
    free(data);

    *err = 0;
    return crc;
}

void free_text(text *txt){
    for (int i = 0; i < txt->lines_num; i++){
        free(txt->lines[i].raw);
        free(txt->lines[i].rendered);
    }
    free(txt->lines);
}


/*** dynamic buffer structure***/

int append_buffer(struct dynamic_buffer *buf, const char *str, int len){
    char *new;
    if ((new = (char*)realloc(buf->bytes, sizeof(char)*(buf->len + len))) == NULL)
        return -1;

    memcpy(&new[buf->len], str, len);
    buf->bytes = new;
    buf->len += len;

    return 0;
}

void free_buffer(struct dynamic_buffer *buf){
    free(buf->bytes);
}


/*** auxiliar ***/

unsigned long compute_crc32(const char* data, size_t size){
	unsigned long crc = crc32(0L, Z_NULL, 0);
	crc = crc32(crc, (const Bytef*)data, size);
	return crc;
}

// had a problem compiling with strdup from string.h
char* strdup(const char* str) {
    if (str == NULL) {
        return NULL;
    }
    size_t len = strlen(str);
    char* allocated_str = (char*)malloc((len + 1) * sizeof(char));
    if (allocated_str != NULL) {
        strcpy(allocated_str, str);
    }
    return allocated_str;
}