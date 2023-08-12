#include "editor.h"

struct editor_state editor;

/*** editor operations ***/

void editor_init(void){
    //terminal_clear_all(void);
    editor.cursor_x = 0UL;
    editor.cursor_y = 0UL;
    editor.render_x = 0UL;
    editor.click_x = 0U;
    editor.click_y = 0U;
    editor.prev_cx = 0UL, editor.prev_cy = 0UL;
    editor.row_offset = 0UL; // offset between top(y=0) and current view
    editor.col_offset = 0UL; // left of the file scroll

    editor.search_cs = DEFAULT_CS_SEARCH;

    editor.max_digits = 1;

    editor.filename = NULL;
    editor.txt = NULL;

    editor.info[0] = 0;
    editor.info_len = 0U;
    editor.info_time = 0;
    editor.curr_time = 0;
    editor.extra_info_lines_num = 0;
    editor.default_info_len = strlen(DEFAULT_INFO);

    //editor.txt = INIT_TEXT;

    editor_update_text_window_size();
}

void editor_open_file(const char *filename){
    free_text(editor.txt);
    if ((editor.txt = new_text()) == NULL){
        die("new_text");
    }

    if (read_file_to_text(filename, editor.txt) != 0) die("open_file");

    editor.max_digits = (int)log10(editor.txt->lines_num)+1;
    // Record filename of currently opened file, in editor's sata
    free(editor.filename);
    editor.filename = strdup(filename);
}

void editor_empty_file(const char *filename){
    free_text(editor.txt);
    if ((editor.txt = new_text()) == NULL){
        die("new_text");
    }

    if (append_line(editor.txt, editor.txt->lines_num, "", 0) != 0){
        die("append_file");
    }
    editor.max_digits = (int)log10(editor.txt->lines_num)+1;

    if (filename != NULL){
        free(editor.filename);
        editor.filename = strdup(filename);
    }
}

void editor_insert_char(char ch){
    if (editor.cursor_y == editor.txt->lines_num){
        // add new line
        if (append_line(editor.txt, editor.txt->lines_num, "", 0) != 0){
            die("append_file");
        }
        editor.max_digits = (int)log10(editor.txt->lines_num)+1;
    }
    // insert char at curr cursor location
    if (insert_char(&editor.txt->lines[editor.cursor_y], editor.cursor_x, ch) != 0) die("insert_char");
 
    // flag text as modified
    editor.txt->modified = True;
    editor.cursor_x++;
    editor.prev_cx = editor.cursor_x;
}

void editor_delete_char(void){
    if(editor.cursor_y == editor.txt->lines_num || (editor.cursor_x == 0 && editor.cursor_y == 0)){
        // cursor past the EOF or in the beggining of file
        return;
    }

    if(editor.cursor_x != 0){
        // delete char at curr cursor location
        delete_char(&(editor.txt->lines[editor.cursor_y]), --editor.cursor_x);
        editor.prev_cx = editor.cursor_x;
    } else if (editor.cursor_y != 0){
        editor.cursor_x = editor.txt->lines[editor.cursor_y - 1].raw_len;
        if (merge_lines(editor.txt, editor.cursor_y, editor.cursor_y - 1) != 0){
            die("merge_lines");
        }
        editor.cursor_y--;
        editor.prev_cy = editor.cursor_y;
        editor.prev_cx = editor.cursor_x;
    }

    // flag text as modified
    editor.txt->modified = True;
    return;
}

void editor_insert_newline(void){
    if(editor.cursor_x == 0){
        if (append_line(editor.txt, editor.cursor_y, "", 0) != 0){
            die("append_line");
        }
        editor.max_digits = (int)log10(editor.txt->lines_num)+1;
        
    } else{
        line *ln = &editor.txt->lines[editor.cursor_y];
        if (append_line(editor.txt, editor.cursor_y + 1, &ln->raw[editor.cursor_x], ln->raw_len < editor.cursor_x ? 0 : ln->raw_len - editor.cursor_x) != 0){
            die("append_line");
        }
        editor.max_digits = (int)log10(editor.txt->lines_num)+1;
        ln = &(editor.txt->lines[editor.cursor_y]);
        ln->raw_len = editor.cursor_x;
        ln->raw[ln->raw_len] = 0;
        update_rendered(ln);
    }
    editor.cursor_y++;
    editor.cursor_x = 0;
    editor.col_offset = 0;
    editor.prev_cx = editor.cursor_x;
    editor.prev_cy = editor.cursor_y;

    editor.txt->modified = True;
}

void editor_save_file(void){
    int err;
    // Request filename if NULL
    size_t input_len;
    if (editor.filename == NULL && (editor.filename = editor_input_prompt(INPUT_PROMPT, &input_len)) == NULL){
        editor_set_info("Save aborted.");
        return;
    }

    // Check if text has been modified
    if (!editor.txt->modified){
        editor_set_info("Saved.");
        return;
    }

    // Save modified text to file
    err = save_text_to_file(editor.txt, compute_text_size(editor.txt), editor.filename);
    switch(err){
        case 0:
            // Update modified flag
            editor.txt->modified = False; // reset
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


void editor_find(void){
    editor_search_prompt(SEARCH_PROMPT);
    //editor.extra_info_lines_num = 0;
}


/*** rendering ***/

void editor_refresh_screen(){
    unsigned int newline_i = prepare_render_info_bar();
    editor_update_text_window_size(); // keep text window size updated

    editor_line_navigation();
    
    struct dynamic_buffer buf = INIT_DYNAMIC_BUFFER;

    append_buffer(&buf, "\x1b[?25l", 6); // hide cursor
    append_buffer(&buf, "\x1b[H", 3); // position cursor top-left

    editor_render_text(&buf);
    editor_render_status_bar(&buf);
    editor_render_info_bar(&buf, newline_i);

    // position cursor in (row,col)
    char cmd[64];
    snprintf(cmd, sizeof(cmd), "\x1b[%ld;%ldH", (editor.cursor_y - editor.row_offset) + 1, (editor.render_x - editor.col_offset) + 1);
    append_buffer(&buf, cmd, strlen(cmd));

    append_buffer(&buf, "\x1b[?25h", 6); // show cursor
     
    /*char aux[100];
    sprintf(aux,"[%d,%d,%d]",editor.row_offset,editor.cursor_y,editor.screen_height);
    append_buffer(&buf,aux,strlen(aux));*/

    write(STDOUT_FILENO, buf.bytes, buf.len);
    free_buffer(&buf);
}

void editor_render_text(struct dynamic_buffer *buf) {
    unsigned int y;
    for (y = 0; y < editor.screen_height; y++) {
        unsigned long file_row = y + editor.row_offset;
        if (file_row >= editor.txt->lines_num){
            char welcome_msg[64];
            int welcome_msg_len;
            if (editor.txt->lines_num == 1 && editor.txt->lines[0].raw_len == 0 && y == editor.screen_height / 3 && (welcome_msg_len = snprintf(welcome_msg, sizeof(welcome_msg), WELCOME_MSG, APP_VERSION)) >= 0) {
                if (welcome_msg_len > editor.screen_width){
                    welcome_msg_len = editor.screen_width;
                }
                int padding = (editor.screen_width - (unsigned int)welcome_msg_len) / 2;
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
            unsigned long line_len = editor.txt->lines[file_row].rendered_len < editor.col_offset ? 0 : editor.txt->lines[file_row].rendered_len - editor.col_offset;
            if (SHOW_LINE_I){
                if (line_len + editor.max_digits > editor.screen_width) {
                    line_len = editor.screen_width - editor.max_digits;
                }
                char line_num[32];
                if (editor.cursor_y != file_row) append_buffer(buf, "\x1b[2m",4); //\x1b[22m
                int curr_digits = snprintf(line_num, sizeof(line_num),"%lu" ,file_row+1);
                int tmp_count = curr_digits;
                while(tmp_count++ < editor.max_digits){
                    append_buffer(buf, " ", 1);
                }
                append_buffer(buf, line_num, curr_digits);
                if (editor.cursor_y != file_row) append_buffer(buf, "\x1b[22m",5);
                append_buffer(buf, &(editor.txt->lines[file_row].rendered[editor.col_offset]), line_len);
            } else{ // place text (no line_i)
                if (line_len > editor.screen_width) {
                    line_len = editor.screen_width;
                }
                append_buffer(buf, &(editor.txt->lines[file_row].rendered[editor.col_offset]), line_len);
            }

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
    int file_info_len = snprintf(file_info, sizeof(file_info), editor.txt->modified ? "%.20s *" : "%.20s", editor.filename ? editor.filename : "[unnamed]");
    int navigation_info_len = snprintf(navigation_info, sizeof(navigation_info), "%ld/%ld", editor.cursor_y + 1, editor.txt->lines_num);

    append_buffer(buf, "\x1b[7m", 4); // switch to inverted colors
    append_buffer(buf, file_info, file_info_len > editor.screen_width ? editor.screen_width : file_info_len);
    while(editor.screen_width > navigation_info_len + file_info_len){
        append_buffer(buf, " ", 1);
        file_info_len++;
    }
    
    if (editor.screen_width == navigation_info_len + file_info_len){
        append_buffer(buf, navigation_info, navigation_info_len); // places navigation_info the most to the right
    } 
    append_buffer(buf,"\x1b[m",3); // switch back to normal formating
    append_buffer(buf, "\r\n", 2);
}


unsigned int prepare_render_info_bar(void){
    // checks if there's an extra line to be added to screen
    editor.extra_info_lines_num = 0;
    unsigned int i = 0;
    editor.curr_time = time(NULL);
    if (editor.info_len && editor.curr_time - editor.info_time < INFO_PERIOD){
        while(i < editor.info_len){
            if (editor.info[i++] == '\n'){
                // add one extra line for info
                editor.extra_info_lines_num++;
                return i;
            }
        }
    } 
    return 0;
}

void editor_render_info_bar(struct dynamic_buffer *buf, unsigned int newline_i){
    append_buffer(buf, "\x1b[K", 3); // clear info bar
    if (editor.info_len && editor.curr_time - editor.info_time < INFO_PERIOD){ // render fresh_info
        if(editor.extra_info_lines_num != 0){
            append_buffer(buf, editor.info, newline_i-1 > editor.screen_width ? editor.screen_width : newline_i-1);
            append_buffer(buf, "\r\n", 2);
            append_buffer(buf, "\x1b[K", 3); // clear line
            append_buffer(buf, &editor.info[newline_i], editor.info_len-newline_i > editor.screen_width ? editor.screen_width : editor.info_len-newline_i);
            editor.extra_info_lines_num = 0;
        }else{ // no extra line
            append_buffer(buf, editor.info, editor.info_len > editor.screen_width ? editor.screen_width : editor.info_len);
        }
    } else { // render default info (doesn't support extra lines)
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

void editor_update_text_window_size(void){
    if (terminal_get_window_size(&editor.screen_height, &editor.screen_width) == -1) die("terminal_get_window_size");
    editor.screen_height -= 2 + editor.extra_info_lines_num; // to hold the status+info bar
}


/*** keypress handling ***/

int editor_read_keypress(void){
    ssize_t nread;
    char byte_in;
    while ((nread = read(STDIN_FILENO, &byte_in, 1)) != 1) {
        if (nread == -1 && errno != EAGAIN) die("read");
    }

    return map_keypress(byte_in, &editor.click_x, &editor.click_y);
}

void editor_process_keypress(void){
    static char insist_quit = False;
    int key_val = editor_read_keypress();
    switch (key_val) {
        case ENTER:
            editor_insert_newline();
            break;

        case CTRL_KEY('q'):
            if (editor.txt->modified && !insist_quit){
                insist_quit = True;
                editor_set_info(UNSAVED_QUIT_INFO);
                return;
            }
            editor_quit();
            break;

        case CTRL_KEY('s'):
            editor_save_file();
            if (insist_quit){
                editor_quit();
            }
            break;

        case CTRL_KEY('f'):
            editor_find();
            break;

        case ARROW_UP:
        case ARROW_DOWN:
        case ARROW_LEFT:
        case ARROW_RIGHT:
            editor_move_cursor(key_val);
            break;

        case LEFT_CLICK:
        case MIDDLE_CLICK:
        case RIGHT_CLICK:
            editor_handle_click(key_val);
            break;
        case CLICK_RELEASE: // ignore
            break;

        case SCROLL_DOWN:
        case SCROLL_UP:
        case PAGE_DOWN:
        case PAGE_UP:
            editor_scroll(key_val);
            break;
   
        case HOME_KEY: // all-left
            editor.cursor_x = 0;
            editor.prev_cx = editor.cursor_x;
            break;
        case END_KEY: // all-right
            if(editor.cursor_y < editor.txt->lines_num){
                editor.cursor_x = editor.txt->lines[editor.cursor_y].raw_len;
                editor.prev_cx = editor.cursor_x;
            }
            break;

        case BACKSPACE:
        case CTRL_KEY('h'):
        case DEL_KEY:
            if (key_val == DEL_KEY){
                editor_move_cursor(ARROW_RIGHT);
            }
            editor_delete_char();
            break;

        case ESC:
            if (insist_quit){
                // cancel
                editor_set_info(DEFAULT_INFO);
            }

            break;

        case CTRL_KEY('l'): // refresh terminal cmd
            break; // do nothing

        default:
            editor_insert_char((char)key_val);
            break;
    }

    insist_quit = False;
}


/*** editor control ***/

void editor_move_cursor(int key_val){
    switch (key_val) {
        case ARROW_LEFT:
            if(editor.cursor_x != 0){
                editor.cursor_x--;
            } else if(editor.cursor_y != 0){
                editor.cursor_y--;
                editor.cursor_x = editor.txt->lines[editor.cursor_y].raw_len;
                editor.prev_cy = editor.cursor_y;
            } else{
                return;
            }
            editor.prev_cx = editor.cursor_x;
            return;

        case ARROW_RIGHT:
            if(editor.cursor_y < editor.txt->lines_num){
                unsigned long line_len = editor.txt->lines[editor.cursor_y].raw_len;
                if(editor.cursor_x < line_len){
                    editor.cursor_x++;
                } else if(editor.cursor_x == line_len && editor.cursor_y + 1 < editor.txt->lines_num){ // check if end of curr line
                    editor.cursor_y++;
                    editor.cursor_x = 0;
                    //editor.col_offset = 0;
                    editor.prev_cy = editor.cursor_y;
                } else{
                    return;
                }
                editor.prev_cx = editor.cursor_x;
            }
            return;

        case ARROW_UP:
            if(editor.cursor_y != 0){
                editor.cursor_y--;
                editor.prev_cy = editor.cursor_y;
                break; // leave switch block (to handle_cursor_x)
            } 
            return;

        case ARROW_DOWN:
            if(editor.cursor_y + 1 < editor.txt->lines_num){
                editor.cursor_y++;
                editor.prev_cy = editor.cursor_y;
                break; // leave switch block (to handle_cursor_x)
            }
            return;

        default:
            return;
    }

    // Fix special case 
    handle_cursor_x();
    
    return;
}

void editor_line_navigation(void){
    editor.render_x = 0;
    if (editor.cursor_y < editor.txt->lines_num){
        editor.render_x = cx_to_rx(&(editor.txt->lines[editor.cursor_y]), editor.cursor_x);
    }

    // -- Check if cursors are outside of visible screen
    // Vertical scroll
    if(editor.cursor_y < editor.row_offset) // cursor_y is above visible window  
        editor.row_offset = editor.cursor_y; // scroll to where cursor is
    if(editor.cursor_y >= editor.screen_height + editor.row_offset) // cursor_y is past bottom visible window
        // editor.row_offset = editor.cursor_y - editor.screen_height + 1;
        editor.row_offset = editor.cursor_y + 1 < editor.screen_height ? 0 : editor.cursor_y + 1 - editor.screen_height;
    
    // Horizontal scroll
    if(editor.render_x - (SHOW_LINE_I ? editor.max_digits : 0) < editor.col_offset){
        //editor.col_offset = editor.render_x - editor.max_digits; 
        editor.col_offset = editor.render_x > editor.screen_width/2 ? editor.render_x - editor.screen_width/2 : 0;
    }  
    if(editor.render_x >= editor.screen_width + editor.col_offset) 
        // editor.col_offset = editor.render_x - editor.screen_width + 1;
        editor.col_offset = editor.render_x + 1 < editor.screen_width ? 0 : editor.render_x + 1 - editor.screen_width;
}

void editor_scroll(int key_val){
    const unsigned int mouse_scroll_offset = 3; // mouse scroll offset
    const unsigned int page_scroll_offset = editor.screen_height - 2;//editor.screen_height*3/4;
    const unsigned int eof_marks = editor.screen_height*2/3;
    switch(key_val){
        case SCROLL_DOWN:
            if (editor.row_offset + editor.screen_height + mouse_scroll_offset <= editor.txt->lines_num - 1 + eof_marks){
                editor.row_offset += mouse_scroll_offset;
                if(editor.prev_cy >= editor.row_offset && editor.prev_cy <= editor.row_offset+editor.screen_height-1){
                    editor.cursor_y = editor.prev_cy;
                } else {
                    if (editor.prev_cy >= editor.row_offset + editor.screen_height - 1){
                    editor.cursor_y = editor.row_offset + editor.screen_height - 1;
                    } else{
                        editor.cursor_y = editor.row_offset;
                    }
                }
            }
            break;

        case SCROLL_UP:
            if (editor.row_offset > mouse_scroll_offset){
                editor.row_offset -= mouse_scroll_offset;
            } else{
                editor.row_offset = 0;
            }
            if(editor.prev_cy >= editor.row_offset && editor.prev_cy <= editor.row_offset+editor.screen_height - 1){
                editor.cursor_y = editor.prev_cy;
            } else {
                if (editor.prev_cy <= editor.row_offset){
                    editor.cursor_y = editor.row_offset;
                } else{
                    editor.cursor_y = editor.row_offset + editor.screen_height > editor.txt->lines_num - 1 ? editor.txt->lines_num - 1: editor.row_offset + editor.screen_height -1;
                }
            }
            break;

        case PAGE_DOWN:
            if (editor.row_offset + editor.screen_height >= editor.txt->lines_num - 1 + eof_marks){ // place cursor at bottom of file
                editor.cursor_y = editor.txt->lines_num-1;
            } else{
                editor.cursor_y += page_scroll_offset;
                if(editor.cursor_y > editor.txt->lines_num - 1){
                    editor.cursor_y = editor.txt->lines_num - 1;
                }
                editor.row_offset += page_scroll_offset;
                if(editor.row_offset+editor.screen_height > editor.txt->lines_num - 1 + eof_marks){
                    unsigned int y_offset = (editor.row_offset+editor.screen_height) - (editor.txt->lines_num - 1 + eof_marks);
                    editor.row_offset -= y_offset; 
                }
            }
            editor.prev_cy = editor.cursor_y;
            break;

        case PAGE_UP:
            if (editor.row_offset == 0){
                editor.cursor_y = 0; // go to top of document
            } else{
                editor.row_offset = editor.row_offset < page_scroll_offset ? 0 : editor.row_offset - page_scroll_offset;  
                editor.cursor_y = editor.cursor_y < page_scroll_offset ? 0 : editor.cursor_y - page_scroll_offset;  
            }
            editor.prev_cy = editor.cursor_y;
            break;

        
        default:
            return;
    }

    handle_cursor_x();
}

void editor_handle_click(int key_val){
    char event_type;
    switch (key_val){
    case LEFT_CLICK:
        event_type = '"';
        break;
    case MIDDLE_CLICK:
        event_type = '!';
        break;
    case RIGHT_CLICK:
        event_type = ' ';
        break;
    default:
        return;
    }

    // wait for click_release
    unsigned int prev_click_x = editor.click_x, prev_click_y = editor.click_y; // to keep track for press+release coordinates
    if(editor_read_keypress() != CLICK_RELEASE){
        if(event_type){
            ;
        } // TODO: remove
        return; // must be click_release!
    }

    if(key_val == RIGHT_CLICK && prev_click_x == editor.click_x && prev_click_y == editor.click_y){
        editor_position_mouse_cursor(); // handle it myself (cursor postioning)
        return;
    }

    // didn't work, find workaround TODO:
    /*
    // - I want some click behavior to be handled by the terminal emulator
    char seq[6];
    disable_mouse_reporting();
    terminal_disable_raw_mode();
    seq[0] = ESC, seq[1] = '[', seq[2] = 'M';
    // redirect click press to stdout
    seq[3] = event_type; // left click
    seq[4] = prev_click_x+33, seq[5] = prev_click_y+33;
    write(STDOUT_FILENO, seq, 6*sizeof(char));
    // redirect click release to stdout
    seq[3] = '#'; // release click
    seq[4] = editor.click_x+33, seq[5] = editor.click_y+33;
    write(STDOUT_FILENO, seq, 6*sizeof(char));
    terminal_enable_raw_mode();
    enable_mouse_reporting(); // re-enable mouse reporting    
    */
   return;
}

void editor_position_mouse_cursor(){
    unsigned long req_y = editor.row_offset + editor.click_y;
    editor.click_x = (SHOW_LINE_I && editor.click_x < (unsigned)editor.max_digits ? (unsigned)editor.max_digits : editor.click_x);
    unsigned long req_x = editor.col_offset + editor.click_x - (SHOW_LINE_I ? editor.max_digits : 0);
    
    if (req_y >= editor.txt->lines_num){
        req_y = editor.txt->lines_num-1;
    }
    if (req_x >= editor.txt->lines[req_y].rendered_len){
        req_x = editor.txt->lines[req_y].rendered_len;
    }

    editor.cursor_y = req_y;
    /*if (SHOW_LINE_I) {
        req_x = (req_x <= (long unsigned)editor.max_digits) ? 0 : req_x - editor.max_digits;
    }*/
    editor.cursor_x = rx_to_cx(&(editor.txt->lines[editor.cursor_y]), req_x);
    editor.prev_cy = editor.cursor_y;
    editor.prev_cx = editor.cursor_x;
    
    return;
}

void editor_quit(void){
    free(editor.filename);
    free_text(editor.txt);
    terminal_clear();
    exit(0);
}

void editor_search_navigation(unsigned long** occs, unsigned long query_len, int key_val){
    if (occs == NULL){
        return; // no occurrences
    }

    // TODO: finish by highlighting all matches inside current window?
    line *ln;
    unsigned long line_i;
    unsigned long max_occ, last_occ_i;
    unsigned long res;

    unsigned long curr_cy = editor.cursor_y, curr_cx = editor.cursor_x;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wimplicit-fallthrough"
    switch (key_val){
        case PREV:
            // place cursor on match before cursor coordinates
            if(curr_cx == 0){
                // reached beggining of curr line: try to go to prev line
                curr_cy = curr_cy == 0 ? editor.txt->lines_num-1 : curr_cy - 1;
                curr_cx = editor.txt->lines[curr_cy].raw_len == 0 ? 0 : editor.txt->lines[curr_cy].raw_len-1;
            } else{
                curr_cx--;
            } 
            ln = &(editor.txt->lines[curr_cy]);
            max_occ = ln->raw_len/query_len;
            last_occ_i = occs[curr_cy][max_occ];
            res = ULONG_MAX;
            if (last_occ_i != ULONG_MAX){
                unsigned long l = 0, r = last_occ_i;
                while (l <= r) {
                    unsigned long mid = l + (r - l) / 2;
                    if (occs[curr_cy][mid] <= curr_cx) {
                        res = mid;
                        l = mid + 1;  // Search in the right half
                    } else { 
                        if (mid == 0) {
                            break; // Prevent underflow when decrementing `r`
                        }
                        r = mid - 1;  // Search in the left half
                    }
                }
                if (res != ULONG_MAX){
                    editor.cursor_y = curr_cy;
                    editor.cursor_x = occs[curr_cy][res];
                    editor.prev_cy = editor.cursor_y;
                    editor.prev_cx = editor.cursor_x;
                    return;
                }
            }
            // not found in the current cursor line, check prev lines
            line_i = (editor.txt->lines_num + curr_cy - 1)%editor.txt->lines_num; // avoid underflow
            while(line_i != curr_cy){
                ln = &(editor.txt->lines[line_i]);
                max_occ = ln->raw_len/query_len;
                last_occ_i = occs[line_i][max_occ];
                if (last_occ_i != ULONG_MAX){
                    editor.cursor_y = line_i;
                    editor.cursor_x = occs[line_i][last_occ_i];
                    editor.prev_cy = editor.cursor_y;
                    editor.prev_cx = editor.cursor_x;
                    return;
                }
                line_i = (editor.txt->lines_num+line_i-1)%editor.txt->lines_num; // avoid underflow
            }
            ln = &(editor.txt->lines[line_i]);
            max_occ = ln->raw_len/query_len;
            last_occ_i = occs[line_i][max_occ];
            if (last_occ_i != ULONG_MAX){
                editor.cursor_y = line_i;
                editor.cursor_x = occs[line_i][last_occ_i];
                editor.prev_cy = editor.cursor_y;
                editor.prev_cx = editor.cursor_x;
                return;
            }
            break;

        case NEXT:
            // place cursor on match after cursor coordinates
            if(++curr_cx >= editor.txt->lines[curr_cy].raw_len){
                curr_cx = 0;
                if (++curr_cy >= editor.txt->lines_num){
                    curr_cy = 0;
                }
            }
        default:
            // place cursor on match closer to cursor coordinates
            ln = &(editor.txt->lines[curr_cy]);
            max_occ = ln->raw_len/query_len;
            last_occ_i = occs[curr_cy][max_occ];
            res = -1;
            if (last_occ_i != ULONG_MAX){
                unsigned long l = 0, r = last_occ_i;
                while (l <= r) {
                    unsigned long mid = l + (r - l) / 2;
                    if (occs[curr_cy][mid] < curr_cx) {
                        l = mid + 1;  // Search in the right half
                    } else {
                        res = mid;
                        if (mid == 0){
                            break; // avoid underflow
                        }  
                        r = mid - 1;  // Search in the left half
                    }
                }
                if (res != ULONG_MAX){
                    editor.cursor_y = curr_cy;
                    editor.cursor_x = occs[curr_cy][res];
                    editor.prev_cy = editor.cursor_y;
                    editor.prev_cx = editor.cursor_x;
                    return;
                }
            }
            
            // not found in the current cursor line, check next lines
            line_i = (curr_cy+1)%editor.txt->lines_num;
            while(line_i != curr_cy){
                ln = &(editor.txt->lines[line_i]);
                max_occ = ln->raw_len/query_len;
                last_occ_i = occs[line_i][max_occ];
                if (last_occ_i != ULONG_MAX){
                    editor.cursor_y = line_i;
                    editor.cursor_x = occs[line_i][0];
                    editor.prev_cy = editor.cursor_y;
                    editor.prev_cx = editor.cursor_x;
                    return;
                }
                line_i = (line_i+1)%editor.txt->lines_num;
            }
            ln = &(editor.txt->lines[line_i]);
            max_occ = ln->raw_len/query_len;
            last_occ_i = occs[line_i][max_occ];
            if (last_occ_i != ULONG_MAX){
                editor.cursor_y = line_i;
                editor.cursor_x = occs[line_i][0];
                editor.prev_cy = editor.cursor_y;
                editor.prev_cx = editor.cursor_x;
                return;
            }
            break;
    }
#pragma GCC diagnostic pop

    return; // no change to cursor positioning    
}

void editor_search_prompt(char* prompt){
    // ENTER SEARCH MODE
    unsigned long** occs = NULL; // only for find functionality

    size_t buf_size = 128;
    char* buf = (char*)malloc(sizeof(char)*buf_size);
    if (buf == NULL){
        return;
    }
    
    // record cursor coordinates before search
    unsigned long prev_cursor_y = editor.cursor_y, prev_cursor_x = editor.cursor_x;

    unsigned long input_len = 0;
    buf[0] = 0;
    while(input_len*sizeof(char) < buf_size){
        editor_set_info(prompt, buf, !editor.search_cs ? "ON": "OFF");
        editor_refresh_screen();

        int key_val = editor_read_keypress();
        if ((key_val == DEL_KEY || key_val == CTRL_KEY('h') || key_val == BACKSPACE) && input_len != 0){
            buf[--input_len] = 0;
            if (input_len != 0){
                free_occurrences(occs, editor.txt->lines_num);
                occs = get_occurrences(editor.txt, buf, input_len, editor.search_cs);
                editor_search_navigation(occs, input_len, key_val);
            }
            // present all occs, highlighting nearest occurence
            // TODO: highlight visible text

        } else if (key_val == ESC || key_val == CTRL_KEY('q')){
            // cancel search and get back to previous cursor position
            // TODO: stop highlighing if toggled on
            editor.cursor_x = prev_cursor_x, editor.cursor_y = prev_cursor_y;
            editor.prev_cx = editor.cursor_x;
            editor.prev_cy = editor.cursor_y;
            editor_set_info("");
            free_occurrences(occs, editor.txt->lines_num);
            free(buf);
            return;

        } else if (key_val == ENTER || key_val == CTRL_KEY('f')){
            // stop search and continue editing at currently highlighed (selected) position and get back to normal editing
            // TODO: stop highlighing if toggled on
            editor_set_info("");
            free_occurrences(occs, editor.txt->lines_num);
            free(buf);
            return;
            
        } else if (!iscntrl(key_val) && key_val < 128){
            // control chars are non-printable characters
            if(input_len == buf_size - 1 && buf_size < 4096*100){
                buf_size *= 2;
                if ((buf = (char*)realloc(buf, sizeof(char)*buf_size))==NULL){
                    free_occurrences(occs, editor.txt->lines_num);
                    free(buf);
                    return;
                }
            }
            buf[input_len++] = key_val;
            buf[input_len] = 0;

            free_occurrences(occs, editor.txt->lines_num);
            occs = get_occurrences(editor.txt, buf, input_len, editor.search_cs);
            editor_search_navigation(occs, input_len, key_val);
            // present all occs, highlighting nearest occurence
            // TODO: highlight visible text

        } else if (key_val == NEXT || key_val == PREV){
            // present all occs highlighed, navigating to next or prev occurence
            editor_search_navigation(occs, input_len, key_val);
            // TODO: highlight visible text

        } else if (key_val == ARROW_DOWN || key_val == ARROW_UP || key_val == ARROW_RIGHT || key_val == ARROW_LEFT){
            editor_move_cursor(key_val);

        } else if (key_val == PAGE_UP || key_val == PAGE_DOWN || key_val == SCROLL_DOWN || key_val == SCROLL_UP){
            editor_scroll(key_val);

        } else if(key_val == LEFT_CLICK || key_val == RIGHT_CLICK || key_val == MIDDLE_CLICK){
            editor_handle_click(key_val);
            
        } else if(key_val == CS_TOGGLE){
            editor.search_cs = !editor.search_cs;
            // update occs
            if (input_len != 0){
                free_occurrences(occs, editor.txt->lines_num);
                occs = get_occurrences(editor.txt, buf, input_len, editor.search_cs);
                editor_search_navigation(occs, input_len, key_val);
            }
        }
    }

    editor.cursor_x = prev_cursor_x, editor.cursor_y = prev_cursor_y;
    editor.prev_cx = editor.cursor_x;
    editor.prev_cy = editor.cursor_y;
    free_occurrences(occs, editor.txt->lines_num);
    free(buf);
    return;
}

char* editor_input_prompt(char* prompt, size_t* input_len){
    size_t buf_size = 128;
    char* buf = (char*)malloc(sizeof(char)*buf_size);
    if (buf == NULL){
        return NULL;
    }

    *input_len = 0;
    buf[0] = 0;
    while((*input_len)*sizeof(char) < buf_size){
        editor_set_info(prompt, buf);
        editor_refresh_screen();
        int ch = editor_read_keypress();
        if ((ch == DEL_KEY || ch == CTRL_KEY('h') || ch == BACKSPACE) && (*input_len) != 0){
            buf[--(*input_len)] = 0;
        } else if (ch == ESC || ch == CTRL_KEY('q')){
            editor_set_info("");
            free(buf);
            return NULL;
        } else if (ch == ENTER && (*input_len) != 0){
            // for reading input
            editor_set_info("");
            return buf;
        } else if (!iscntrl(ch) && ch < 128){
            // control chars are non-printable characters
            if((*input_len) == buf_size - 1 && buf_size < 4096){
                buf_size *= 2;
                if ((buf = (char*)realloc(buf, sizeof(char)*buf_size))==NULL){
                    return NULL;
                }
            }
            buf[(*input_len)++] = ch;
            buf[*input_len] = 0;
        }
    }

    return buf;
}


/*** auxiliar ***/

void handle_cursor_x(void){
    // Account for special case where cursor goes from end of long line to next smaller line (adjust cursor_x accordingly)
    unsigned long line_len;
    if (editor.cursor_y < editor.txt->lines_num){
        if (editor.cursor_x > (line_len = editor.txt->lines[editor.cursor_y].rendered_len)){
            editor.cursor_x = line_len;
        } else if (editor.prev_cx <= line_len) {
            editor.cursor_x = editor.prev_cx;
        }
    }
    return;
}

unsigned long cx_to_rx(line *ln, unsigned long cx){
    unsigned long rx = 0;
    for (unsigned long j = 0; j < cx; j++){
        if (ln->raw[j] == '\t'){
            rx += ((TAB_SIZE-1) - (rx % TAB_SIZE));
        }
        rx++;
    }
    if (SHOW_LINE_I){
        rx+=editor.max_digits;
    }
    return rx;
}

unsigned long rx_to_cx(line *ln, unsigned long rx){
    unsigned long curr_rx = 0;
    unsigned long cx;
    for (cx = 0; cx < ln->raw_len; cx++){
        if(ln->raw[cx] == '\t'){
            curr_rx += (TAB_SIZE - 1) - (curr_rx % TAB_SIZE);
        }
        curr_rx++;
        if(curr_rx > rx){
            return cx;  
        } 
    }
    return cx;
}
