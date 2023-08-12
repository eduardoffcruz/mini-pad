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
#include "kmp_search.h"

#define True 1
#define False 0

#define APP_VERSION "1.0"
#define WELCOME_MSG "minipad editor -- version %s"
#define DEFAULT_INFO "HELP: ^S = save | ^Q = quit | ^F = find"
#define UNSAVED_QUIT_INFO "Unsaved changes: ^S = save & quit | ^Q = quit | <ESC> = cancel"
#define SEARCH_PROMPT "Search: %s \n^→ = next | ^← = prev | <ENTER> = stop | <ESC> = cancel | ^↓ = CS %s"
#define INPUT_PROMPT "Save as: %s \n<ENTER> = submit | <ESC> = cancel"
#define INFO_PERIOD 8 // seconds
#define DEFAULT_CS_SEARCH False

struct editor_state{
    // cursor
    unsigned long cursor_y;
    unsigned long cursor_x; // points to raw_text position
    unsigned long render_x; // for rendering tabs
    unsigned long prev_cx; // for smoother scrolling
    unsigned long prev_cy; // for scrolling
    unsigned int click_x, click_y; // 0 to 255 (relative to screen view)!
    // window dimentions
    unsigned short screen_height, screen_width;
    // scroll
    unsigned long row_offset, col_offset; // refers to how much the cursor is past the top, left of the screen

    char search_cs;
    
    char* filename;
    struct text* txt;

    char info[256];
    unsigned int info_len;
    time_t info_time, curr_time;  
    unsigned short extra_info_lines_num;
    unsigned int default_info_len;  
    
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
void editor_empty_file(const char *filename);

/*
*
*/
void editor_insert_char(char ch);

/*
*
*/
void editor_delete_char(void);

/*
*
*/
void editor_insert_newline(void);

/*
*
*/
void editor_save_file(void);

/*
*
*/
void find_callback(char *query, int key_val, unsigned long* query_len);

/*
*
*/
void editor_find(void);


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
unsigned int prepare_render_info_bar(void);

/*
*
*/
void editor_render_info_bar(struct dynamic_buffer *buf, unsigned int newline_i);

/*
*
*/
void editor_set_info(const char *fmt, ...);

/*
*
*/
void editor_update_text_window_size(void);


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
void editor_line_navigation(void);

/*
*
*/
void editor_scroll(int key_val);

/*
*
*/
void editor_position_mouse_cursor(void);

/*
*
*/
void editor_quit(void);

/*
*
*/
char* editor_input_prompt(char* prompt, size_t* input_len);

/*
*
*/
void editor_search_prompt(char* prompt);

/*
*
*/
void editor_search_navigation(unsigned long** occs, unsigned long query_len, int key_val);

/*** auxiliar ***/
/*
* 
*/
unsigned long rx_to_cx(line *ln, unsigned long rx);

/*
* 
*/
unsigned long cx_to_rx(line *ln, unsigned long cx);

/*
*
*/
void handle_cursor_x(void);



#endif /* EDITOR_H */