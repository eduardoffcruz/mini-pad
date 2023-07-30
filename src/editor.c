#include "editor.h"

struct editor_state editor;


/*** editor operations ***/

void editor_init(void){
    //terminal_clear_all(void);
    editor.cursor_x = 0;
    editor.cursor_y = 0;
    editor.render_x = 0;
    editor.prev_x = 0;
    editor.row_offset = 0; // offset between top(y=0) and current view
    editor.col_offset = 0; // left of the file scroll

    editor.filename = NULL;
    editor.txt.lines_num = 0;
    editor.txt.saved_crc = 0L;
    editor.txt.saved_size = 0;

    editor.info[0] = 0;
    editor.info_len = 0;
    editor.info_time = 0;
    editor.default_info_len = strlen(DEFAULT_INFO);
    //editor.txt = INIT_TEXT;

    if (terminal_get_window_size(&editor.screen_height, &editor.screen_width) == -1) die("terminal_get_window_size");
    editor.screen_height -= 2; // to hold the status+info bar
}

void editor_open_file(const char *filename){
    if (read_file_to_text(filename, &editor.txt) != 0) die("open_file");

    // Record filename of currently opened file, in editor's sata
    free(editor.filename);
    editor.filename = strdup(filename);
}

void editor_insert_char(char ch){
    if (editor.cursor_y == editor.txt.lines_num){
        // add new line
        append_line(&editor.txt, "", 0);
    }
    // insert char at curr cursor location
    if (insert_char(&editor.txt.lines[editor.cursor_y], editor.cursor_x, ch) != 0) die("insert_char");
}

void editor_save_file(void){
    int err;
    // Request filename if NULL
    if (editor.filename == NULL){
        // TODO: request filename
        return;
    }

    // Verify wether text has been modified using crc32
    // - comparison of text's size reduces crc32 collision problem since only the crc32 of same sized texts is compared. 
    unsigned long curr_crc = 0L;
    size_t curr_size = compute_text_size(&editor.txt);
    if (curr_size == editor.txt.saved_size){
        // check crc32 to be confident content was modified
        curr_crc = compute_text_crc32(&editor.txt, curr_size, &err);
        if (err!=0){
            editor_set_info("Failed to save file: unable to verify text changes.");
            return;
        }
        if(curr_crc == editor.txt.saved_crc){
            editor_set_info("Saved.");
            return;
        }
    }

    // Save modified text to file
    err = save_text_to_file(&editor.txt, curr_size, editor.filename);
    switch(err){
        case 0:
            // Update saved_crc and saved_size
            editor.txt.saved_crc = (curr_crc == 0L ? compute_text_crc32(&editor.txt, curr_size, &err) : curr_crc);
            editor.txt.saved_size = curr_size;
            editor_set_info("Your changes have been saved.");
            break;
        case -1:
            editor_set_info("Failed to save file: no filename");
            break;
        case -2:
        case -3:
        case -4:
        case -5:
            editor_set_info("Failed to save file: %s", strerror(errno));
            break;
    }

    return;    
}


/*** rendering ***/

void editor_refresh_screen(void){
    editor_line_scroll();

    struct dynamic_buffer buf = INIT_DYNAMIC_BUFFER;

    append_buffer(&buf, "\x1b[?25l", 6); // hide cursor
    append_buffer(&buf, "\x1b[H", 3); // position cursor top-left

    editor_render_text(&buf);
    editor_render_status_bar(&buf);
    editor_render_info_bar(&buf);

    // position cursor in (row,col)
    char cmd[28];
    snprintf(cmd, sizeof(cmd), "\x1b[%d;%dH", (editor.cursor_y - editor.row_offset) + 1, (editor.render_x - editor.col_offset) + 1);
    append_buffer(&buf, cmd, strlen(cmd));

    append_buffer(&buf, "\x1b[?25h", 6); // show cursor
     
    /*char aux[100];
    sprintf(aux,"[%d,%d,%d]",editor.row_offset,editor.cursor_y,editor.screen_height);
    append_buffer(&buf,aux,strlen(aux));*/

    write(STDOUT_FILENO, buf.bytes, buf.len);
    free_buffer(&buf);
}

void editor_render_text(struct dynamic_buffer *buf) {
    int y;
    for (y = 0; y < editor.screen_height; y++) {
        int file_row = y + editor.row_offset;
        if (file_row >= editor.txt.lines_num){
            if (editor.txt.lines_num == 0 && y == editor.screen_height / 3) {
                char welcome_msg[64];
                int welcome_msg_len = snprintf(welcome_msg, sizeof(welcome_msg), WELCOME_MSG, APP_VERSION);
                if (welcome_msg_len > editor.screen_width){
                    welcome_msg_len = editor.screen_width;
                }
                int padding = (editor.screen_width - welcome_msg_len) / 2;
                if (padding) {
                    append_buffer(buf, "~", 1);
                    padding--;
                }
                while (padding--){
                    append_buffer(buf, " ", 1);
                } 
                append_buffer(buf, welcome_msg, welcome_msg_len);
            } else {
                append_buffer(buf, "~", 1);
            }
            
        } else{
            int line_len = editor.txt.lines[file_row].rendered_len - editor.col_offset;
            if (line_len < 0){
                line_len = 0;
            } else if (line_len > editor.screen_width) {
                line_len = editor.screen_width;
            }
            append_buffer(buf, &(editor.txt.lines[file_row].rendered[editor.col_offset]), line_len);
        }

        append_buffer(buf, "\x1b[K", 3);
        //if (y < editor.screen_height - 1) {
        append_buffer(buf, "\r\n", 2);
        //}
    }
    //append_buffer(buf, "\r\n", 2);
}

void editor_render_status_bar(struct dynamic_buffer *buf){
    char file_info[64], navigation_info[32];
    int file_info_len = snprintf(file_info, sizeof(file_info), "%.20s - %d lines", editor.filename ? editor.filename : "[no name]", editor.txt.lines_num);
    int navigation_info_len = snprintf(navigation_info, sizeof(navigation_info), "%d/%d", editor.cursor_y + 1, editor.txt.lines_num);

    append_buffer(buf, "\x1b[7m", 4); // switch to inverted colors
    append_buffer(buf, file_info, file_info_len > editor.screen_width ? editor.screen_width : file_info_len);
    while(editor.screen_width - file_info_len > navigation_info_len){
        append_buffer(buf, " ", 1);
        file_info_len++;
    }
    if (editor.screen_width - file_info_len == navigation_info_len){
        append_buffer(buf, navigation_info, navigation_info_len); // places navigation_info the most to the right
    } 
    append_buffer(buf,"\x1b[m",3); // switch back to normal formating
    append_buffer(buf, "\r\n", 2);
}

void editor_render_info_bar(struct dynamic_buffer *buf){
    append_buffer(buf, "\x1b[K", 3); // clear info bar
    if (editor.info_len && time(NULL) - editor.info_time < INFO_PERIOD){
        append_buffer(buf, editor.info, editor.info_len > editor.screen_width ? editor.screen_width : editor.info_len);
    } else {
        append_buffer(buf, DEFAULT_INFO, editor.default_info_len > editor.screen_width ? editor.screen_width : editor.default_info_len);
    }
}

void editor_set_info(const char *fmt, ...){
    // Takes a format str and a variable number of arguments, like the printf family of functions
    va_list args;
    va_start(args, fmt);
    editor.info_len = vsnprintf(editor.info, sizeof(editor.info), fmt, args); // safe!
    va_end(args);
    editor.info_time = time(NULL); // timestamp
}


/*** keypress handling ***/

int editor_read_keypress(void){
    int nread;
    char byte_in;
    while ((nread = read(STDIN_FILENO, &byte_in, 1)) != 1) {
        if (nread == -1 && errno != EAGAIN) die("read");
    }

    return map_keypress(byte_in);
}

void editor_process_keypress(void){
    int key_val = editor_read_keypress();
    switch (key_val) {
        case ENTER: //
            break;

        case CTRL_KEY('q'):
            free(editor.filename);
            free_text(&editor.txt);
            terminal_clear();
            exit(0);
            break;

        case CTRL_KEY('s'):
            editor_save_file();
            break;

        case ARROW_UP:
        case ARROW_DOWN:
        case ARROW_LEFT:
        case ARROW_RIGHT:
            editor_move_cursor(key_val);
            break;

        case PAGE_DOWN:
        case PAGE_UP:
            editor_page_scroll(key_val);
            break;
   
        case HOME_KEY: // all-left
            editor.cursor_x = 0;
            break;
        case END_KEY: // all-right
            if(editor.cursor_y < editor.txt.lines_num){
                editor.cursor_x = editor.txt.lines[editor.cursor_y].raw_len;
            }
            break;

        case BACKSPACE:
        case CTRL_KEY('h'):
        case DEL_KEY:
            break;


        case CTRL_KEY('l'): // refresh terminal cmd
        case '\x1b': // ESC
            break; // do nothing

        default:
            editor_insert_char((char)key_val);
            editor.cursor_x++;
            break;
  }
}


/*** editor control ***/

void editor_move_cursor(int key_val){
    switch (key_val) {
        case ARROW_LEFT:
            if(editor.cursor_x != 0){
                editor.cursor_x--;
            } else if(editor.cursor_y > 0){
                editor.cursor_y--;
                editor.cursor_x = editor.txt.lines[editor.cursor_y].raw_len;
            } else{
                return;
            }
            editor.prev_x = editor.cursor_x;
            return;
        case ARROW_RIGHT:
            if(editor.cursor_y < editor.txt.lines_num){
                int line_len = editor.txt.lines[editor.cursor_y].raw_len;
                if(editor.cursor_x < line_len){
                    editor.cursor_x++;
                } else if(editor.cursor_x == line_len){ // check if end of curr line
                    editor.cursor_y++;
                    editor.cursor_x = 0;
                } else{
                    return;
                }
                editor.prev_x = editor.cursor_x;
            }
            return;
        case ARROW_UP:
            if(editor.cursor_y != 0){
                editor.cursor_y--;
                break; // leave switch block
            } 
            return;
        case ARROW_DOWN:
            if(editor.cursor_y < editor.txt.lines_num){
                editor.cursor_y++;
                break; // leave switch block
            }
            return;
        default:
            return;
    }

    // Fix special case 
    handle_cursor_x();
    
    return;
}

void editor_line_scroll(void){
    editor.render_x = 0;
    if (editor.cursor_y < editor.txt.lines_num){
        editor.render_x = compute_render_x(&(editor.txt.lines[editor.cursor_y]), editor.cursor_x);
    }

    // -- Check if cursors are outside of visible screen
    // Vertical scroll
    if(editor.cursor_y < editor.row_offset) // cursor_y is above visible window  
        editor.row_offset = editor.cursor_y; // scroll to where cursor is
    if(editor.cursor_y >= editor.screen_height + editor.row_offset) // cursor_y is past bottom visible window
        editor.row_offset = editor.cursor_y - editor.screen_height + 1;
    
    // Horizontal scroll
    if(editor.render_x < editor.col_offset)   
        editor.col_offset = editor.render_x;
    if(editor.render_x >= editor.screen_width + editor.col_offset) 
        editor.col_offset = editor.render_x - editor.screen_width + 1;
}

void editor_page_scroll(int key_val){
    const int page_scroll_offset = editor.screen_height-2;
    switch(key_val){
        case PAGE_DOWN:
            if(editor.row_offset+editor.screen_height >= editor.txt.lines_num){
                editor.cursor_y = editor.txt.lines_num;
            } else if (editor.cursor_y+page_scroll_offset > editor.txt.lines_num){
                int y_offset = (editor.cursor_y - editor.row_offset);
                editor.row_offset = editor.txt.lines_num - y_offset;
                editor.cursor_y = editor.txt.lines_num;
            } else{
                editor.row_offset += page_scroll_offset;
                editor.cursor_y += page_scroll_offset;
            }
            break;
        case PAGE_UP:
            if (editor.row_offset == 0){
                editor.cursor_y = 0; // go to top of document
            }
            else if (editor.row_offset - page_scroll_offset < 0){
                editor.cursor_y -= editor.row_offset;
                editor.row_offset = 0; // fix view
                //if (editor.cursor_y < 0){
                //    editor.cursor_y += page_scroll_offset;
                //}
            } else{
                editor.row_offset -= page_scroll_offset;
                editor.cursor_y -= page_scroll_offset;
            }
            break;
        default:
            return;
    }

    handle_cursor_x();
}


/*** auxiliar ***/

void handle_cursor_x(void){
    // Account for special case where cursor goes from end of long line to next smaller line (adjust cursor_x accordingly)
    int line_len;
    if (editor.cursor_y < editor.txt.lines_num){
        if (editor.cursor_x > (line_len = editor.txt.lines[editor.cursor_y].rendered_len)){
            editor.cursor_x = line_len;
        } else if (editor.prev_x <= line_len) {
            editor.cursor_x = editor.prev_x;
        }
    }
    return;
}

int compute_render_x(line *ln, int cx){
    int rx = 0;
    for (int j = 0; j < cx; j++){
        if (ln->raw[j] == '\t'){
            rx += ((TAB_SIZE-1) - (rx % TAB_SIZE));
        }
        rx++;
    }
    return rx;
}
