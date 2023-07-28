
#include "data_structures.h"

//
int append_buffer(struct dynamic_buffer *buf, const char *str, int len){
    char *new;
    if ((new = realloc(buf->bytes, buf->len + len)) == NULL)
        return -1;

    memcpy(&new[buf->len], str, len);
    buf->bytes = new;
    buf->len += len;

    return 0;
}

void free_buffer(struct dynamic_buffer *buf){
    free(buf->bytes);
}

//
int append_line(text *txt, char *str, int len){
    // Re-allocate space for new line
    txt->lines = (line *)realloc(txt->lines, sizeof(line) * (txt->lines_num + 1));
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
    if ((err = update_rendered(txt, line_i)) != 0){
        return err;
    }

    //
    txt->lines_num++;
    return 0;
}

int update_rendered(text *txt, int line_i)
{
    int tabs = 0;
    int j, raw_len = txt->lines[line_i].raw_len;
    // count tabs
    for (j = 0; j < raw_len; j++){
        if (txt->lines[line_i].raw[j] == '\t'){
            tabs++;
        }
    }

    // re-allocate rendered
    free(txt->lines[line_i].rendered);
    if ((txt->lines[line_i].rendered = (char *)malloc(sizeof(char) * (raw_len + tabs * (TAB_SIZE - 1) + 1))) == NULL){
        return -1;
    }
    // render tabs according to TAB_SIZE
    int ind = 0;
    for (j = 0; j < raw_len; j++)
    {
        char raw_byte;
        if ((raw_byte = txt->lines[line_i].raw[j]) == '\t'){
            txt->lines[line_i].rendered[ind++] = ' '; // <space>
            while (ind % TAB_SIZE){
                txt->lines[line_i].rendered[ind++] = ' '; // <space>
            }
        } else{
            txt->lines[line_i].rendered[ind++] = raw_byte;
        }
    }

    txt->lines[line_i].rendered[ind] = 0; // end-of-line
    txt->lines[line_i].rendered_len = ind;
    return 0;
}

void free_text(text *txt){
    for (int i = 0; i < txt->lines_num; i++){
        free(txt->lines[i].raw);
        free(txt->lines[i].rendered);
    }
    free(txt->lines);
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