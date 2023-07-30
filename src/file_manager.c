#include "file_manager.h"

/*** file operations ***/

int read_file_to_text(const char *filename, struct text *txt){
    FILE *fp; 
    if ((fp = fopen(filename, "r")) == NULL){
        return -1;  
    }

    char *str = NULL;
    size_t allocated_size = 0; // getline allocates space
    ssize_t raw_len;
    size_t text_size = 0;
    int err;

    while((raw_len = getline(&str, &allocated_size, fp)) != -1){
        while (raw_len > 0 && ((str[raw_len-1] == '\n') | (str[raw_len-1] == '\r'))){
            raw_len--; // strip '\r\n' from line
        }

        if ((err = append_line(txt, str, raw_len)) != 0){
            return err;
        }
        text_size += raw_len+1;
    }

    free(str);
    fclose(fp);
    
    txt->saved_size = text_size;
    txt->saved_crc = compute_text_crc32(txt, text_size, &err);

    return 0;
}


int save_text_to_file(text* txt, size_t size, const char *filename){
    // check if file was modified since last save! usar CRC32!
    // TODO: IMPROVE BY WRITING TO NEW TEMPORARY FILE AND THEN RENAME IT TO THE ACTUAL FILE THE USER WANTS TO OVERWRITE, AND CAREFULLY CHECK FOR ERRORS!
    if (filename == NULL){
        return -1; // no filename specified
    }

    char *data = text_to_data(txt, size);
    if (data == NULL){
        return -2;
    }

    // create temporary file
    char tmp_filename[256];
    snprintf(tmp_filename, sizeof(tmp_filename), ".tmp_%s", filename);
    
    int tmp_fd=-1;
    if((tmp_fd = open(tmp_filename, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IROTH)) == -1){
        free(data);
        return -3;
    }

    // write the data to the temporary file
    if (write(tmp_fd, data, size) != (ssize_t)size){
        close(tmp_fd);
        free(data);
        return -4;
    }

    // close tmp file and replace original file for tmp file
    close(tmp_fd);
    if (rename(tmp_filename, filename) == -1){
        free(data);
        return -5;
    }

    free(data);

    return 0;
}