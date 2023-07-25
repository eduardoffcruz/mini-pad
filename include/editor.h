#ifndef EDITOR_H
#define EDITOR_H

#include <stdio.h>
#include <ctype.h>
#include <sys/ioctl.h>
#include "terminal_handler.h"
#include "append_buffer.h"

#define APP_VERSION "1.0"
#define CTRL_KEY(k) ((k) & 0x1f)

enum editor_key {
    ARROW_LEFT = 1000,
    ARROW_RIGHT,
    ARROW_UP,
    ARROW_DOWN
};

struct editor_cfg{
    int cx, cy;
    int screen_rows;
    int screen_cols;
    //struct abuf data_buffer;
};

/*
*
*/
int editor_read_key();

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
void editor_draw_rows(struct abuf *buffer);

/*
* 
*/
int get_window_size(int *rows, int *cols);

/*
* 
*/
void editor_init();

/*
* 
*/
void editor_move_cursor(int key_val);

#endif /* EDITOR_H */