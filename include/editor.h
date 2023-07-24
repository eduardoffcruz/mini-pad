#ifndef EDITOR_H
#define EDITOR_H

#include <stdio.h>
#include <ctype.h>
#include <sys/ioctl.h>
#include "terminal_handler.h"
#include "append_buffer.h"

#define APP_VERSION "1.0"
#define CTRL_KEY(k) ((k) & 0x1f)


struct editor_cfg{
    int screen_rows;
    int screen_cols;
    //struct abuf data_buffer;
};

/*
*
*/
char editor_read_key();

/*
*
*/
void editor_process_keypress();

/*
* 
*/
void editor_refresh_screen();

/*
* 
*/
void editor_draw_rows();

/*
* 
*/
int get_window_size(int *rows, int *cols);

/*
* 
*/
void init_editor();

#endif /* EDITOR_H */