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
#define DEFAULT_INFO "HELP: Ctrl-S = save | Ctrl-Q = quit"
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
    int default_info_len;  
    
    //struct abuf state_buffer;
};


/*** editor operations ***/

/*
* 
*/
void editor_init(void);

/*
* 
*/
void editor_open_file(const char *filename);

/*
*
*/
void editor_insert_char(char ch);

/*
*
*/
void editor_save_file(void);


/*** rendering ***/

/*
* 
*/
void editor_refresh_screen(void);

/*
* 
*/
void editor_render_text(struct dynamic_buffer *buffer);

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


/*** keypress handling ***/

/*
*
*/
int editor_read_keypress(void);

/*
*
*/
void editor_process_keypress(void);


/*** editor control ***/

/*
* 
*/
void editor_move_cursor(int key_val);

/*
* 
*/
void editor_line_scroll(void);

/*
*
*/
void editor_page_scroll(int key_val);


/*** auxiliar ***/
/*
* 
*/
int compute_render_x(line *ln, int cx);

/*
*
*/
void handle_cursor_x(void);



#endif /* EDITOR_H */