#ifndef EDITOR_H
#define EDITOR_H

#include <stdio.h>
#include <time.h>
#include <stdarg.h>
#include <ctype.h>
#include <math.h>
#include "terminal.h"
#include "data_structures.h"
#include "key_mapping.h"
#include "file_manager.h"
#include "kmp_search.h"

#define True 1
#define False 0

#define APP_VERSION "1.0"
#define WELCOME_MSG "minipad editor -- version %s"
#define DEFAULT_STATUS "HELP: ^S = save | ^X = quit | ^F = find"
#define UNSAVED_QUIT_STATUS "Unsaved changes: ^S = save & quit | ^X = quit | <ESC> = cancel"
#define SEARCH_PROMPT "Search: %s \n^→ = next | ^← = prev | <ENTER> = stop | <ESC> = cancel | ^↓ = %s case-sensitive"
#define INPUT_PROMPT "Save as: %s \n<ENTER> = submit | <ESC> = cancel"
#define STATUS_PERIOD 8 // seconds
#define DEFAULT_CS_SEARCH False // case-sensitive search
#define SHOW_LINE_I True

struct editor_state{
    // -- Cursor
    unsigned long cursor_y; // text's line index
    unsigned long cursor_x; // text's character index (position of char in lines[cursor_y]->raw)
    // NOTE: manipulate cursor_x only (don't mess with render_x)
    unsigned long render_x; // used for rendering tabs
    // -- Cursors previous position
    unsigned long prev_cx; // for smoother scrolling
    unsigned long prev_cy; // for scrolling
    // -- Mouse click position: 0 to 255 (relative to screen view)!
    unsigned int click_x, click_y; 
    // -- Dimentions of window holding text-view
    unsigned short text_view_height, text_view_width;
    // -- Text-view offset: 
    unsigned long row_offset; // refers to how much the current text-view is past the start of the text
    unsigned long col_offset; // refers to how much the current text-view is past the left of the text
    
    // -- File data
    char* filename;
    struct text* txt;
    
    // -- Status bar 
    char status[256];
    unsigned int status_len; // length of status message in status buf
    time_t status_time, curr_time; // to keep track of display period of temporary status 
    unsigned short extra_status_lines_num; // to know whether the status bar takes 1 or 2 lines
    unsigned int default_status_len; // length of DEFAULT_STATUS string 
    
    // -- Miscelaneous
    int max_digits; // number of digits of (txt->lines_num) value: for rendering left-bar with line numbering
    char search_cs; // flags wether pattern search is case-sensitive or not.
};


/*** editor operations ***/

/*
* Initialize editor's state.
*/
void editor_init(void);

/*
* Load file's content to editor's state.
*/
void editor_open_file(const char *filename);

/*
* Set editor's state to the one of a new empty file. 
*/
void editor_new_file(const char *filename);

/*
* Insert character in current editor's cursor position.
*/
void editor_insert_char(char ch);

/*
* Delete character on current editor's cursor position.
*/
void editor_delete_char(void);

/*
* Insert new line in current editor's cursor position.
*/
void editor_insert_newline(void);

/*
* Save state's text to file. 
*/
void editor_save_file(void);

/*
* Find occurences of pattern in state's text:
+   - reveals search prompt and allows navigation between occurences.
*/
void editor_find(void);

/*
* 
*/
void editor_quit(void);


/*** rendering ***/

/*
* Updates and renders current editor's whole view:
*   - cursor position, file's text, info & status bars.
*/
void editor_refresh_screen(void);

/*
* Renders current state's text in editor's view
*/
void editor_render_text(struct dynamic_buffer *buffer);

/*
* Renders bar displaying file information:
*   - filename, modified state, current & total number of lines.
*/
void editor_render_file_info_bar(struct dynamic_buffer *buf);

/*
* Checks wether the status bar to-be rendered has 1 or 2 lines, to adjust whole view:
*   - shall be called before rendering anything to screen.
*/
unsigned int evaluate_status_bar_dimention(void);

/*
* Renders bar displaying status message:
+   - default or temporary status.
*/
void editor_render_status_bar(struct dynamic_buffer *buf, unsigned int newline_i);

/*
* Set temporary status message:
* - arbitrary number of arguments as (str_format, var0, var1, var2): e.g., ("%d %c", count, ch)
*/
void editor_set_status(const char *fmt, ...);

/*
* Updates editor's text-window dimentions according to current terminal's window dimentions.
*/
void editor_update_text_view_dims(void);

/*
* Accounts for special case where cursor goes from end of long line to next smaller line:
*   - Adjusts cursor_x accordingly.
*/
void ensure_cursor_x(void);


/*** keypress handling ***/

/*
* Reads keypress from input.
*   - returns value according to keypress mapping.
*/
int read_keypress(void);

/*
* Reads and processes keypress value based on its mapped value.  
*/
void editor_process_keypress(void);


/*** editor control ***/

/*
* Moves cursor 1 position according to ARROW keypress:
*   - left, right, up, or down.
*/
void editor_move_cursor(int key_val);

/*
* Adjusts text-view according to current cursor position:
* - if current line's text doesn't fit current text-view, view is adjusted so to display such text.
*/
void editor_adjust_text_view(void);

/*
* Scroll through text-view (adjusts view and cursor position):
+   - supports mouse's wheel scroll and page-up & page-down scroll
*/
void editor_scroll(int key_val);

/*
* Handles clicks & presses with mouse:
*   - e.g., right click positions editor's cursor at mouse's cursor position.
*/
void editor_handle_mouse_click(int key_val);

/*
* Positions editor's cursor at mouse's cursor click position.
*/
void editor_position_mouse_cursor(void);


/*
* Enters search/find mode:
*   - search for occurences of pattern in text (reacts to user input);
*   - enables navigation through occurences in text. 
*/
void editor_search_prompt(char* prompt);

/*
* Display prompt to write input:
*  - returns input. 
*/
char* editor_input_prompt(char* prompt, size_t* input_len);

/*
* Navigates through pattern occurences in text according to keypress:
*   - positions editor's cursor at next or previous occurence.
*/
void editor_search_navigation(unsigned long** occs, unsigned long query_len, int key_val);

/*** helper functions ***/
/*
* Returns cursor_x value that corresponds to render_x.
*/
unsigned long rx_to_cx(line *ln, unsigned long rx);

/*
* Returns render_x value that corresponds to cursor_x.
*/
unsigned long cx_to_rx(line *ln, unsigned long cx);



#endif /* EDITOR_H */