#include "file_manager.h"


int read_file_text(const char *filename, struct text *txt){
    FILE *fp; 
    if ((fp = fopen(filename, "r")) == NULL){
        return -1;  
    }

    char *str = NULL;
    size_t allocated_size = 0; // getline allocates space
    ssize_t raw_len;

    while((raw_len = getline(&str, &allocated_size, fp)) != -1){
        while (raw_len > 0 && ((str[raw_len-1] == '\n') | (str[raw_len-1] == '\r'))){
            raw_len--; // strip '\r\n' from line
        }
        int err;
        if ((err = append_line(txt,str, raw_len))!=0){
            return err;
        }
    }

    free(str);
    fclose(fp);

    return 0;
}