#ifndef EDITOR_H
#define EDITOR_H

#include <stdio.h>
#include <time.h>
#include <stdarg.h>
#include <ctype.h>
#include "terminal.h"
#include "data_structures.h"
#include "key_mapping.h"
#include "file_manager.h"

#define APP_VERSION "1.0"
#define WELCOME_MSG "Text editor -- version %s"
#define INFO_PERIOD 8 // seconds

#define CTRL_KEY(k) ((k) & 0x1f)


struct editor_state{
    // cursor
    int cursor_y, cursor_x; 
    int render_x; // for rendering tabs
    int prev_x; // for smoother scrolling
    // window dimentions
    int screen_height, screen_width;
    // scroll
    int row_offset, col_offset; // refers to how much the cursor is past the top, left of the screen
    
    char* filename;
    struct text txt;

    char info[64];
    int info_len;
    time_t info_time;    
    
    //struct abuf state_buffer;
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
void editor_render_text(struct dynamic_buffer *buffer);


/*
* 
*/
void editor_init();

/*
* 
*/
void editor_open_file();

/*
* 
*/
void editor_move_cursor(int key_val);

/*
* flow
*/
void editor_line_scroll(void);

/*
* 
*/
int compute_render_x(line *ln, int cx);

/*
*
*/
void manage_cursor_x(void);

/*
*
*/
void editor_page_scroll(int key_val);

/*
*
*/
void editor_render_status_bar(struct dynamic_buffer *buf);

/*
*
*/
void editor_render_info_bar(struct dynamic_buffer *buf);

/*
*
*/
void editor_set_info(const char *fmt, ...);


#endif /* EDITOR_H */